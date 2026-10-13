#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

void client(int, int), server (int,int);

int main (int argc , char **argv)
{
  int readfd, writefd;
  pid_t childpid;

  /* create two FIFOs; OK if they already exist */
  if ((mkfifo (FIFO1, FILE_MODE) < 0) && (errno != EEXIST)) {
       fprintf(stderr, "can't create %s\n",FIFO1);
       exit(1);
  }

  if ((mkfifo (FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) {
    unlink (FIFO1);
    fprintf(stderr, "Can't create %s \n",FIFO2);
    exit(2);
  }

  if ((childpid = fork()) == 0) { /* child */
     printf("In child\n");
     if ((readfd = open(FIFO1,O_RDONLY, 0)) < 0) {
       fprintf(stderr, "Error opening FIFO %s(%s)\n", FIFO1, strerror(errno));
       exit(3);
     }
     writefd = open(FIFO2,O_WRONLY, 0);
     server(readfd,writefd);
     exit (0);
  }

  /* PARENT */
  printf("In parent\n");
  writefd = open(FIFO1,O_WRONLY, 0);
  readfd = open(FIFO2,O_RDONLY, 0);
  client(readfd,writefd);

  waitpid (childpid, NULL, 0);
  close(readfd);
  close(writefd);
    
  unlink(FIFO1);
  unlink(FIFO2);
  exit(0);
}

void client(int readfd, int writefd)
{
  printf("%s: readfd = %d, writefd = %d\n", __FUNCTION__, readfd, writefd);
  return;
}

void server(int readfd, int writefd)
{
  printf("%s: readfd = %d, writefd = %d\n", __FUNCTION__, readfd, writefd);
  return;
}

