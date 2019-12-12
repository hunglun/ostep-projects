/**
 * @file   parallel_execution.c
 * @author hunglun <hunglun@hunglun-Lenovo-G40-70>
 * @date   Sun Dec  8 10:51:38 2019
 * 
 * @brief  create and execute n processes in parallel
 * 
 * 
 */

#include <stdlib.h> // exit
#include <stdio.h> // printf
#include <sys/wait.h> // wait
#include <unistd.h> // fork

/** 
 * Spawn a process for each function and execute all processes in parallel.
 * 
 * @param n : the size of functor array
 * @param functions : array of function pointers
 * 
 */
void parallel_exec(unsigned int n, void (*functions[n])(void) ){
  for(int i=0;i<n;i++){
    int pid = fork();
    if (pid == 0){
      functions[i]();
      exit(0);
    }
  }
  wait(NULL); // wait for all child processes to terminate.
}



/** 
 * print hello
 * 
 */
void hello(void){
  printf("hello\n");
}


#ifdef TEST
int main(void){
  void (*functions[2])(void) = {};
  functions[0] = hello;
  functions[1] = hello;  
  parallel_exec(2, functions);

}
#endif //TEST
