// Implementation copied from 
// https://stackoverflow.com/questions/24005459/implementation-of-the-random-number-generator-in-c-c

static unsigned long int next = 1; 
int rand(int max) // return 0 .. max - 1
{
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % max;
} 

void srand(unsigned int seed) 
{ 
  next = seed;
} 
