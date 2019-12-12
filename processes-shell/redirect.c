/**
 * @file   redirect.c
 * @author O Hung Lun <hunglun.o@gmail.com>
 * @date   Thu Dec 12 22:11:32 2019
 * 
 * @brief  Support Redirection of Standard Output to File
 * 
 * 
 */
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> // O_WRONLY
#include <stdlib.h>
#include <assert.h>
#include "redirect.h"
/** 
 * convert a string into an array using whitespace as delimiter
 *
 * Note: create a function that replace complex whitespace with single whitespace.
 *       free dest after use. TODO provide a convenient method to free dest.
 * @param pdest 
 * @param src 
 * 
 * @return array size
 */
int convert_string_to_array(char *** pdest, char * src){
  // can i dynamically allocate memory to dest? Yes
  int token_count;
  int srclen;
  char * line;
  char * orig; // remember start of allocated memory before using strsep
  char ** dest;
  
  // INITIALISE
  srclen = strlen(src);
  orig = line = malloc(srclen+1);
  // FIRST PASS OVER SRC TO DETERMINE THE SIZE OF DEST.
  token_count = 0;
  strcpy(line, src);
  while(line!=NULL){
    (void)strsep(&line," ");
    token_count++;
  }
  // PREPARE FOR SECOND PASS
  dest = malloc(token_count * sizeof(char *));
  line = orig;
  
  // SECOND PASS TO POPULATE DEST.
  token_count = 0; // reset token count      
  strcpy(line, src); // overwrite line with src again.
  while(line!=NULL){
    char * token = strsep(&line," ");
    dest[token_count] = malloc(strlen(token) + 1);
    strcpy(dest[token_count], token);
    token_count++;
  }
  // CLEAN UP
  *pdest = dest;
  free(orig);
  return token_count;
}
/** 
 * Store substring in output path after '>' sign.
 * 
 * @param output_path contains pre-allocated memory, enough to hold output path shorter than line.
 * @param line 
 * @param sign is '&'
 * 
 * @return 
 */
int get_output_path(char ** output_path, char * line, char * sign){
  assert(strlen(sign) == 1); // assert that the sign is a single character.
  for(int i=0;i<strlen(line);i++){ // iterate over line
    if(line[i] == *sign) {
      strcpy(*output_path, &line[i+1]); // i+1 will not go out of bound, because there is NULL character after sign.
      return 0;
    }
  }
  return -1;
}
/** 
 * redirect standard output to file
 * 
 * @param path 
 * 
 * @return 0 if success, otherwise return -1
 */
int redirect_stdout_to_file(char * path){
  //remove heading whitespace
  while((*path) == ' ') path++;
  int filefd = open(path, O_WRONLY|O_CREAT, 00600);
  if (filefd == -1) return -1;
  close(1);
  dup(filefd);
  return 0;
}
/** 
 * remove leading and trailing whitespaces in path
 * 
 * @param string 
 */
void trim_string(char ** string){
  // move pointer past leading whitespaces
  int sz = strlen(*string);
  for(int i=0; i<sz; i++){
    if((*string)[0] != ' ') {
#ifdef TEST
      printf("%s, %c, %d\n", *string, (*string)[0], i);
#endif //TEST
      break;      
    }
    (*string)++;
  }
  // replace trailing whitespaces with NULL terminator
  for(int i=strlen(*string)-1; 0 < i; i--){
    if((*string)[i] != ' ') break;
    (*string)[i] = '\0';
  }
}
#ifdef TEST
int main(int argc, char *argv[]){
  // test 1: valid redirection
  
  char * line;
  char * path = malloc(255); // 255 is the maximum filename length of EXT4 filesystem.
  char * sign = "-";
  int result;
  // test 1
  line = "ls  redirect.c - a.txt ";
  result = get_output_path(&path, line, sign);
  assert(result == 0);
  (void)trim_string(&path);
  assert(strcmp(path, "a.txt") == 0);
  // test 2
  line = "ls redirect.c-a.txt ";
  result = get_output_path(&path, line, sign);
  assert(result == 0);
  (void)trim_string(&path);
  assert(strcmp(path, "a.txt") == 0);
  // test 3
  char * str = malloc(100);
  strcpy(str, "          echo test variable whitespace!           ");
  (void)trim_string(&str);
  assert(strcmp(str, "echo test variable whitespace!") == 0);
  
  return 0;
}
#endif 
