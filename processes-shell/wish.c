/**
 * @file   wish.c
 * @author O Hung Lun <hunglun.o@gmail.com>
 * @date   Thu Dec 12 22:11:44 2019
 * 
 * @brief  a toy shell implementation that supports system command execution, 
 * script, redirection and parallel execution. Requirement is described in
 * github.com/remzi-arpacidusseau/ostep-projects/tree/master/processes-shell
 * 
 */
/* Standard headers */
#include <sys/wait.h> /* wait */
#include <stdio.h> /* printf */
#include <stdlib.h> /* fopen */
#include <string.h> /* strsep, strcat */
#include <unistd.h> /* execv */
#include <assert.h> /* assert */
#include <errno.h> /*  show last error number */
/* relative headers */
#include "redirect.h" /*  redirection, parse_line_argv */
#define SEARCH_PATH_SIZE 100
#define MAX_PATH_LENGTH 255
#define MAX_PROCESS_CNT 255
#define MAX_CMD_ARG_LEN 10
/* Global variables */
char *g_paths[SEARCH_PATH_SIZE];
/* Function prototypes */
int main(int argc, char *argv[]);
int convert_whitespc_delimited_string_to_array(char ***args, char * str, int n);
int redirect_stdout_to_valid_file(char ** line);
int execute_processes_in_parallel(char ** line);
int get_path_for_basename(char ** path, char * basename);
void execute_command_after_fork(char * line);
void print_error_msg(void);
int is_builtin_cmd(char * line);
int set_path(char * line); /* built_in commands */
int change_directory(char * line); /* built_in commands */
int exit_shell(char * line); /* built_in commands */
/** 
 * get_path_for_basename
 * search for basename in g_paths
 * 
 * @param basename 
 * 
 * @return full path of a match. Return NULL if no match is found.
 */
int get_path_for_basename(char **path, char * basename){
  int i=0;
  while(g_paths[i]!=NULL){
    /*  control copy size.  2 is for "/" and null terminator. */
    strncpy(*path, g_paths[i], MAX_PATH_LENGTH - strlen(basename) - 2); 
    (void)strcat(*path, "/"); 
    (void)strcat(*path, basename); 
    if (access(*path, X_OK) == 0){
      return 0; /*  found! */
    }
    i++;
  }  
  return -1;
}
/** 
 * Convert str to array of string separated by " "
 * 
 * @param args : args is allocated memory in this function.
 * @param str
 * @param n : size of array args
 *
 * @return size of args
 */
int convert_whitespc_delimited_string_to_array(char ***args, char * str, int n){
  char * line = strdup(str);
  int i = 0;  
  *args =  malloc(sizeof(char *) * n); 
  while(line!=NULL && i < n){
    (*args)[i] = strsep(&line, " ");
    i++;
  }   /*  parse line into myargs array. */
  return i;
}
/** 
 * Execute script or system commands in a new process
 *
 * This function is called after a process is created by a fork
 * @param line is modified as a side effect.
 */
void execute_command_after_fork(char * line){
  char **myargs;
  int i =  convert_whitespc_delimited_string_to_array(&myargs, line, MAX_CMD_ARG_LEN);
  myargs[i] = NULL; /* execv expects myargs array to end with NULL. */
  char * found_path = malloc(MAX_PATH_LENGTH);  /* it will not be freed. */
  int found = get_path_for_basename(&found_path, myargs[0]);
  (void) execv(found_path, myargs); /*  line is program arguments */
  print_error_msg();    
  exit(0);  
}
/** 
 * print_error_msg
 */
void print_error_msg(void){
  char * error_message =  "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}
/** 
 * cd is a built-in command to change directory.
 * 
 * @param line : it will be modified as side effect, if line is prefixed by 'cd'
 * 
 * @return 0 if line starts with 'cd'. Otherwise -1.
 */
int change_directory(char * line){
  if (strncmp(line, "cd", 2) == 0) {
    char * cmd = strsep(&line, " "); /*  cmd == cd           */
    char * dest = strsep(&line, " ");
    if(line != NULL /* does not expect a second argument. */
       || chdir(dest) == -1){ /* change directory fails */ 
      print_error_msg();
    }    
    return 0;
  }else{
    
    return -1;
  }
}
/** 
 * path is a built-in command to reset search path to given argument.
 *
 * If no argument is given, search path will be empty and
 * all subsequent system commands will return error.
 * 
 * @param line : line will be modified as a side effect.
 * 
 *  @return 0 if line starts with 'path'. Otherwise -1.
 */
int set_path(char * line){
  unsigned char path_cursor = 0;
  /*  does it start with 'path'? */
  if (strncmp(line, "path", 4) != 0) return -1; 
  /*  empty g_paths before assignment */
  for(int i=0; i<SEARCH_PATH_SIZE; i++) {
    if(g_paths[i]!=NULL) {
      free(g_paths[i]); /*  free path */
      g_paths[i]=NULL;	   
    }else{ /*  last search path? */
      break;
    }
  }
  (void)strsep(&line, " "); /*  move cursor to the first argument. */
  
  while(line!=NULL){
    char *p = strsep(&line, " ");
    g_paths[path_cursor] = malloc(strlen(p) + 1); /*  allocate memory. */
    strcpy(g_paths[path_cursor], p);
    
    path_cursor++;
    if(path_cursor>SEARCH_PATH_SIZE-1){ /*  g_paths[99] is the last slot. */
      print_error_msg();
      exit(0); /*  exit program, as malicious user input is detected. */
    }
  }
  return 0;
}
/** 
 * exit is a built-in command to exit the shell.
 *
 * it does not accept argument.
 * 
 * @param line : modified if it is exit command.
 * 
 * @return 0 if line starts with 'exit'. Otherwise -1.
 */
int exit_shell(char * line){
  /*  parse exit command */
  if (strncmp(line, "exit", 4) == 0){
    (void)strsep(&line, " ");
    if(line != NULL){ /*  it is an error to pass argument to exit */
      print_error_msg();
      return 0;
    }
    else{      
      exit(0); /*  no return code needed. */
    }
  }else{
    return -1;
  }
}
/** 
 * parse for built-in command
 * 
 * @param line : modified as a side effect, if it is a built-in command.
 * 
 * @return 0 if it is built-in command, otherwise, -1.
 */
int is_builtin_cmd(char * line){
  /*  parse exit, path and cd commands */
  return  (set_path(line) == 0         ||
	   change_directory(line) == 0 ||
	   exit_shell(line) == 0)? 0 : -1;
}
/** 
 * spawn new processes to support parallel execution
 *
 * @param line : if parallel execution is valid, line become empty.
 * 
 * @return 0 if  valid parallel execution or  no parallel execution.
 *         -1 if  invalid parallel execution.
 */
int execute_processes_in_parallel(char ** line){
  if (strchr(*line, '&') == NULL) return 0; /*  no parallel commands */
  /*  This is a parallel execution command. */
  /*  let's determine if it is valid. */
  if (strlen(*line) == 1) {
    *line[0] = 0; /*  consume line */
    return 0;  /*  '&' is valid. */
  }
  (void)trim_string(line);
  if ((*line)[strlen(*line) - 1] != '&')
    strcat(*line, " &"); /*  append & to make  a&b command truly parallel  */
  
  /*  create new process for each & sign. */
  char * orig = strsep(line, "&");
  char * prev = orig; /*  remember the last token */
  int children_pid[MAX_PROCESS_CNT]; /*  TODO make the size  more flexible */
  int children_count = 0;
  while(*line != NULL){
    int pid = fork();
    children_pid[children_count++]=pid;
    if (pid == 0){
      *line = strdup(prev);
      return 0;
    }
    prev = strsep(line, "&");
  }
  /*  wait for all children here: */
  for (int i=0; i<children_count; i++){    
    wait(NULL);
  }
  *line = prev;
  return 0;
}
/** 
 * redirect stdout to valid output path.
 * 
 * @param line 
 * 
 * @return -1 if output path is invalid, otherwise 0.
 */
int redirect_stdout_to_valid_file(char ** line){
  char * path = malloc(MAX_PATH_LENGTH); 
  char * sign = ">";
  int redirection_exists = get_output_path(&path, *line, sign);
  int status = 0;
  (void)trim_string(&path);
  /*  check if line start with '>', flag it as invalid. */
  if ((*line)[0] == '>') return -1;
  
  /*  check that there is no whitespace in path. */
  for(int i=0; i< strlen(path); i++){
    if(path[i] == ' ') return -1;
  }
  if (redirection_exists == 0) { /*  there is redirection sign? */
    status=redirect_stdout_to_file(path);
    /*  replace redirection sign with NULL terminator */
    for(int i=0; i< strlen(*line); i++){
      if((*line)[i] == *sign) (*line)[i] = '\0';
    }
  }
  return status;
}
/** 
 * main function
 * 
 * @param argc 
 * @param argv 
 * 
 * @return 
 */
int main(int argc, char *argv[]){
  FILE * stream = NULL;
  char * line = NULL;
  size_t len = 0; /*  len is unused. */
  int stdout_copy = dup(1);
  
  g_paths[0] = strdup("/bin");
  /*  determine batch or interactive mode */
  if (argc > 2) {
    print_error_msg();
    exit(EXIT_FAILURE);
  }else if(argc == 2) {
    stream = fopen(argv[1], "r");
    if (stream == 0) { /*  does the script exit? */
      print_error_msg();
      exit(EXIT_FAILURE);
    }
  }else{
    printf("wish> ");
    stream = stdin; /*  default to standard input */
  }
  int program_pid = getpid();
  /*  parse program arguments */
  while(-1 != getline(&line, &len, stream)){ /*  while not EOF */
    
    line[strlen(line)-1] = 0; /*  remove newline at the end. */
    if (line[0] == '#') continue ; /*  ignore comment */
    /*  parse for parallel execution */
    if (execute_processes_in_parallel(&line) != 0){ 
      print_error_msg();
      if (stream == stdin) printf("wish> "); 
      continue;
    
    }/*  is there invalid parallel execution? */
    trim_string(&line);
    
    /*  parse for redirection */
    if (redirect_stdout_to_valid_file(&line) != 0){
      print_error_msg();
      if (stream == stdin) 
	printf("wish> "); 
      continue;
    }
    
    /*  built-in commands */
    if ( is_builtin_cmd(line) == 0 || strncmp(line, "", 1) == 0) {
      if (stream == stdin) 
	printf("wish> "); 
      continue;
    }
    /*  if path is empty, any command would not be able to run. */
    if (g_paths[0]==NULL){
      print_error_msg();
      if (stream == stdin) 
	printf("wish> ");       
      continue;
    }
    /*  fork and execute model */
    pid_t pid = fork();
    if (pid == 0){ /*  child process */
      execute_command_after_fork(line);
    }
    else{
      wait(NULL); /*  wait for all children */
      dup2(stdout_copy, 1); /*  restore standard output   */
      if (program_pid != getpid()) exit(0);
      if (stream == stdin) printf("wish> "); 
    }
  }
  exit(0);
}
