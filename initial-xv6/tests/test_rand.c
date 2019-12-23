#include <stdio.h>
static unsigned long int next = 1; 
int rand(int max) // return 0 .. max - 1
{
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % max;
} 


void main(void){
  for(int i=0; i<100; i++)
    printf("%d\n", rand(2));

  
}
