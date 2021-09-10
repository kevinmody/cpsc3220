#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

void * thread_func( void *args){ //function that multiplies the value from command line 
   int *result = (int*) args;
   *(result) *= 10;
}

int main(int argc, char * argv[]){
    //thread id
    pthread_t thread_id1;
    pthread_t thread_id2;
    
    int result1 = atoi(argv[2]);
    int result2 = atoi(argv[3]); 
    
    if (argc < 2 || argc > 4) { //checking the arguments if it is enough
        printf("Error not enough command line arguments. Please try for example: './asg1 17 11 22' \n"); 
        exit(1);
    }
    int pid = fork();
    if(pid == 0){
        execl("./child_function","child_function",argv[1],NULL); //This calls child_function which multiplies the number given via the command line by 2
    }
    else{  
        wait(NULL);
        printf("Parent Process: PID %d and PPID is %d\n", getpid(),getppid()); //getting parent id and parent process id 
    }
    // Create threads that will do function thread_func()
    int err1 = pthread_create(&thread_id1,NULL,thread_func,(void*)&result1); 
    int err2 = pthread_create(&thread_id2,NULL,thread_func,(void*)&result2);
    if(err1){ //checking if thread1 created successfull
        printf("Thread creation failed \n");
        return 1;
    }
    if(err2){ //checking if thread2 created successfull
        printf("Thread creation failed \n");
        return 1;
    }
    err1 = pthread_join(thread_id1, NULL);
    err2 = pthread_join(thread_id2, NULL);
    if (err1){ //checking if thread1 join successfull
        printf( "Failed to join Thread with error %d\n", err1);
        return err1;
    }
    if (err2){ //checking if thread2 join successfull
        printf( "Failed to join Thread with error %d\n", err2);
        return err2;
    }
    printf("Thread %ld, initial value = %d, result1 = %d\n", thread_id1, atoi(argv[2]), result1); 
    printf("Thread %ld, initial value = %d, result2 = %d\n", thread_id2, atoi(argv[3]), result2);
    return 0;
}