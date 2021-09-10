#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

void main(int argc, char *argv[]) {

    int input = atoi(argv[1]);
    input = input + 10;
    printf("Child process: PID %d and PPID is: %d, Initial Value: %d , Result = %d\n",  getpid(), getppid(),atoi(argv[1]), input);
   return;
}