[CmdletBinding()]
param(
    [Parameter()]
    [string]$RootPath = (Get-Location).Path
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

try {
    $resolvedRoot = (Resolve-Path -LiteralPath $RootPath -ErrorAction Stop).ProviderPath
    if (-not (Test-Path -LiteralPath $resolvedRoot -PathType Container)) {
        throw "The scan root is not a directory: $RootPath"
    }

    $skipDirectoryNames = @(
        '.git',
        '.venv',
        'venv',
        'node_modules',
        '__pycache__',
        '.tox',
        '.mypy_cache',
        '.pytest_cache',
        'site-packages',
        'dist',
        'build',
        'target',
        '.next',
        'coverage'
    )
    $repositories = [System.Collections.Generic.List[System.IO.DirectoryInfo]]::new()
    $pendingDirectories = [System.Collections.Generic.Stack[System.IO.DirectoryInfo]]::new()
    $visitedDirectories = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    $skippedCount = 0

    foreach ($directory in (Get-ChildItem -LiteralPath $resolvedRoot -Directory -Force -ErrorAction Stop)) {
        if (($directory.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -eq 0) {
            $pendingDirectories.Push($directory)
        }
    }

    while ($pendingDirectories.Count -gt 0) {
        $directory = $pendingDirectories.Pop()
        if (-not $visitedDirectories.Add($directory.FullName)) {
            continue
        }

        $gitMarker = Join-Path -Path $directory.FullName -ChildPath '.git'
        if (Test-Path -LiteralPath $gitMarker) {
            $repositories.Add($directory)
        }
        else {
            $skippedCount++
        }

        foreach ($childDirectory in (Get-ChildItem -LiteralPath $directory.FullName -Directory -Force -ErrorAction Stop)) {
            if (($skipDirectoryNames -contains $childDirectory.Name) -or
                (($childDirectory.Attributes -band [System.IO.FileAttributes]::ReparsePoint) -ne 0)) {
                continue
            }

            $pendingDirectories.Push($childDirectory)
        }
    }

    $repositories = @($repositories | Sort-Object -Property FullName)
}
catch {
    Write-Host "Unable to scan directories below '$RootPath': $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

function Invoke-GitCommand {
    param(
        [Parameter(Mandatory)]
        [string[]]$Arguments
    )

    $previousErrorActionPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $safeDirectory = (Get-Location).ProviderPath.Replace('\', '/')
        $gitArguments = @('-c', "safe.directory=$safeDirectory") + $Arguments
        $commandOutput = @(& git @gitArguments 2>&1)
        $exitCode = $LASTEXITCODE
    }
    catch {
        return [pscustomobject]@{
            ExitCode = 127
            Output   = @($_.Exception.Message)
        }
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }

    return [pscustomobject]@{
        ExitCode = $exitCode
        Output   = $commandOutput
    }
}

function Write-GitFailure {
    param(
        [Parameter(Mandatory)]
        [string]$FolderName,
        [Parameter(Mandatory)]
        [string]$Operation,
        [Parameter(Mandatory)]
        [object[]]$Output
    )

    Write-Host "[$FolderName] FAILED during $Operation." -ForegroundColor Red
    foreach ($line in $Output) {
        Write-Host "  $line" -ForegroundColor Red
    }
}

$repositoryCount = 0
$skippedCount = 0
$committedCount = 0
$unchangedCount = 0
$pushedCount = 0
$noOriginCount = 0
$sensitiveSkippedCount = 0
$failedCount = 0

Write-Host "Scanning all descendant directories of '$resolvedRoot' (not the root itself)..." -ForegroundColor Cyan

foreach ($directory in $repositories) {
    $folderName = $directory.FullName.Substring($resolvedRoot.Length).TrimStart([char]92)

    $repositoryCount++
    Write-Host "[$folderName] Processing repository..." -ForegroundColor Cyan
    $locationPushed = $false

    try {
        Push-Location -LiteralPath $directory.FullName -ErrorAction Stop
        $locationPushed = $true

        $statusResult = Invoke-GitCommand -Arguments @('status', '--short', '--untracked-files=all')
        if ($statusResult.ExitCode -ne 0) {
            Write-GitFailure -FolderName $folderName -Operation 'git status' -Output $statusResult.Output
            $failedCount++
            continue
        }

        $statusLines = @($statusResult.Output |
            ForEach-Object { $_.ToString() } |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) })

        $sensitivePaths = @($statusLines |
            Where-Object { $_ -match '(?i)(?:^.{0,3}|[/\\])"?\.env(?:$|[./\\"])(?!example|sample|template|dist)' })
        if ($sensitivePaths.Count -gt 0) {
            $sensitiveSkippedCount++
            Write-Host "[$folderName] WARNING: possible environment/credential file changes detected; skipping commit and push." -ForegroundColor Yellow
            foreach ($line in $sensitivePaths) {
                Write-Host "  $line" -ForegroundColor Yellow
            }
            continue
        }

        if ($statusLines.Count -gt 0) {
            Write-Host "[$folderName] Changes detected ($($statusLines.Count) entries):" -ForegroundColor Yellow
            foreach ($line in ($statusLines | Select-Object -First 20)) {
                Write-Host "  $line" -ForegroundColor DarkGray
            }
            if ($statusLines.Count -gt 20) {
                Write-Host "  ... and $($statusLines.Count - 20) more entries." -ForegroundColor DarkGray
            }

            $addResult = Invoke-GitCommand -Arguments @('add', '-A')
            if ($addResult.ExitCode -ne 0) {
                Write-GitFailure -FolderName $folderName -Operation 'git add -A' -Output $addResult.Output
                $failedCount++
                continue
            }

            $commitResult = Invoke-GitCommand -Arguments @('commit', '-m', 'backup before linux setup')
            if ($commitResult.ExitCode -ne 0) {
                Write-GitFailure -FolderName $folderName -Operation 'git commit' -Output $commitResult.Output
                $failedCount++
                continue
            }

            $committedCount++
            Write-Host "[$folderName] Committed changes." -ForegroundColor Green
        }
        else {
            $unchangedCount++
            Write-Host "[$folderName] Nothing to commit." -ForegroundColor Cyan
        }

        $remoteResult = Invoke-GitCommand -Arguments @('remote')
        if ($remoteResult.ExitCode -ne 0) {
            Write-GitFailure -FolderName $folderName -Operation 'git remote' -Output $remoteResult.Output
            $failedCount++
            continue
        }

        $remoteNames = @($remoteResult.Output |
            ForEach-Object { $_.ToString().Trim() } |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) })

        if ($remoteNames -notcontains 'origin') {
            $noOriginCount++
            Write-Host "[$folderName] WARNING: no remote named 'origin'; skipping push." -ForegroundColor Yellow
            continue
        }

        $branchResult = Invoke-GitCommand -Arguments @('branch', '--show-current')
        if ($branchResult.ExitCode -ne 0) {
            Write-GitFailure -FolderName $folderName -Operation 'git branch --show-current' -Output $branchResult.Output
            $failedCount++
            continue
        }

        $branchName = ($branchResult.Output | ForEach-Object { $_.ToString() }) -join "`n"
        $branchName = $branchName.Trim()
        if ([string]::IsNullOrWhiteSpace($branchName)) {
            Write-Host "[$folderName] FAILED: could not determine a current branch (the repository may be in detached HEAD state)." -ForegroundColor Red
            $failedCount++
            continue
        }

        Write-Host "[$folderName] Pushing branch '$branchName' to origin..." -ForegroundColor Cyan
        $pushResult = Invoke-GitCommand -Arguments @('push', 'origin', $branchName)
        if ($pushResult.ExitCode -ne 0) {
            Write-GitFailure -FolderName $folderName -Operation 'git push' -Output $pushResult.Output
            $failedCount++
            continue
        }

        $pushedCount++
        Write-Host "[$folderName] Pushed successfully." -ForegroundColor Green
    }
    catch {
        $failedCount++
        Write-Host "[$folderName] FAILED: $($_.Exception.Message)" -ForegroundColor Red
    }
    finally {
        if ($locationPushed) {
            Pop-Location
        }
    }
}

Write-Host ''
Write-Host "Finished scanning '$resolvedRoot'." -ForegroundColor Cyan
Write-Host "Repositories: $repositoryCount | Committed: $committedCount | Nothing to commit: $unchangedCount | Pushed: $pushedCount | No origin: $noOriginCount | Skipped (possible credentials): $sensitiveSkippedCount | Failed: $failedCount | Skipped (not repositories): $skippedCount"

if ($failedCount -gt 0) {
    exit 1
}
