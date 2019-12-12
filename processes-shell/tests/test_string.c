/**
 * @file   test_string.c
 * @author hunglun <hunglun@hunglun-Lenovo-G40-70>
 * @date   Thu Dec  5 22:29:38 2019
 * 
 * @brief  string library test suite
 * 
 * 
 */

#include <string.h> // strsep, strcmp, strcpy
#include <stdio.h> // printf
#include <assert.h> // assert
#include <stdlib.h> // malloc



/** 
 * test strsep
 * 
 */
void test_strsep(void){
  char * str, * orig;

  // Test 1
  str = malloc(100);
  strcpy(str, "echo 1 & echo 2");
  orig = strsep(&str,"&");
  assert(strcmp(orig, "echo 1 ") == 0);
  assert(strcmp(str, " echo 2") == 0);

  // Test 2
  free(orig);
  str = malloc(100);
  strcpy(str, "&");
  orig = strsep(&str, "&");
  assert(strcmp(orig, "") == 0); orig is holding \0 value.
  assert(orig != NULL); // orig is not pointing at address 0.
  assert(strcmp(str, "") == 0);
  assert(str != NULL);
  
}

#ifdef TEST
int main(void){

test_strsep();

}
#endif
