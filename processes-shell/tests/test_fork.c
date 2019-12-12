/**
 * @file   test_fork.c
 * @author hunglun <hunglun@hunglun-Lenovo-G40-70>
 * @date   Sun Dec  8 11:45:37 2019
 * 
 * @brief  test fork function
 * 
 * 
 */

#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/** 
 * Create n processes using fork
 * 
 * @param n : number of processes
 */
void create_process(int n){
  if(n == 0) return;
  if(fork() == 0){
    printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());
    create_process(n-1);
  }  
}



#ifdef TEST
int main(int argc, char *argv[]){
/* int a = 99; */
/* int pid = fork(); */
/* if (pid == 0){ // child process; */
/* a = 98; */
/* }else{ */
/* wait(0); */
/* assert(a == 99); // assert that parent and child have their own copy of a. */
/* } */
int n;
if (argc > 1){
  sscanf(argv[1], "%d", &n); 
  create_process(n);
}else{
  create_process(2);
}
}
#endif //TEST
