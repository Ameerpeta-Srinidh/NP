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

    $directories = @(Get-ChildItem -LiteralPath $resolvedRoot -Directory -Force -ErrorAction Stop |
        Sort-Object -Property Name)
}
catch {
    Write-Host "Unable to read scan root '$RootPath': $($_.Exception.Message)" -ForegroundColor Red
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
        $commandOutput = @(& git @Arguments 2>&1)
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

Write-Host "Scanning immediate subdirectories of '$resolvedRoot'..." -ForegroundColor Cyan

foreach ($directory in $directories) {
    $folderName = $directory.Name
    $gitDirectory = Join-Path -Path $directory.FullName -ChildPath '.git'

    if (-not (Test-Path -LiteralPath $gitDirectory -PathType Container)) {
        $skippedCount++
        Write-Host "[$folderName] Skipped: no .git directory." -ForegroundColor DarkGray
        continue
    }

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
            Where-Object { $_ -match '(?i)(^|[/\\])\.env(?:$|[./\\"])(?!example|sample|template|dist)' })
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
