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
FILE * g_stream = NULL;
/* Function prototypes */
int redirect_stdout_to_valid_file(char ** line);
int execute_processes_in_parallel(char ** line);
void print_error_msg(void);
int is_builtin_cmd(char * line);
/**
 * exit is a built-in command to exit the shell.
 *
 * it does not accept argument.
 *
 * @param line
 *
 * @return 0 if line starts with 'exit'. Otherwise -1.
 */
int exit_shell(char * line){
  if (strlen(line) == 4 && strcmp(line, "exit") == 0) exit(0);
  if (strncmp(line, "exit", 4) == 0){
    print_error_msg(); /*  it is an error to pass argument to exit */
    return 0;
  }
  else{
    return -1;
  }
}

/**
 * get_path_for_basename
 * Search for basename in g_paths
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
 * Convert str to array of string separated by " ". The last entry is set to NULL.
 *
 * If the size of array exceeds n, the content is truncated to n.
 * Last entry in array is NULL.
 *
 * @param args : args is allocated memory in this function.
 * @param str
 * @param n : max size of array args
 *
 * @return size of args
 */
int convert_whitespc_delimited_string_to_array(char ***args, char * str, int n){
  char * line = strdup(str);
  int i = 0;
  *args =  malloc(sizeof(char *) * n);
  while(line!=NULL && i < n-1){
    (*args)[i] = strsep(&line, " ");
    i++;
  }   /*  parse line into myargs array. */
  (*args)[i] = NULL; /* execv expects myargs array to end with NULL. */
  return i;
}
/**
 * Execute script or system commands in a new process
 *
 * This function is called after a new process is forked.
 * @param line
 */
void execute_command_after_fork(char * line){
  char **myargs;
  char * found_path = malloc(MAX_PATH_LENGTH);  /* it will not be freed. */
  (void)convert_whitespc_delimited_string_to_array(&myargs, line, MAX_CMD_ARG_LEN);
  (void)get_path_for_basename(&found_path, myargs[0]);
  (void)execv(found_path, myargs); /*  line is program arguments */
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
 * @param line : it is modified as side effect, if line is prefixed by 'cd'
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
 * all subsequent system commands (except for bash script) will return error.
 *
 * @param line : line is modified as a side effect.
 *
 *  @return 0 if line starts with 'path'. Otherwise -1.
 */
int set_path(char * line){
  /*  does it start with 'path'? */
  if (strncmp(line, "path", 4) != 0) return -1;
  (void)strsep(&line, " "); /*  move cursor to the first argument. */
  /*  empty g_paths before assignment */
  for(int i=0; i<SEARCH_PATH_SIZE; i++) {
    if(g_paths[i]!=NULL) {
      free(g_paths[i]); /*  free path */
      g_paths[i]=NULL;
    }
    if(line!=NULL){
      char *p = strsep(&line, " ");
      g_paths[i] = strdup(p);
    }
  }
  return 0;
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
  /*  let's determine if it is valid parallel execution command. */
  if (strlen(*line) == 1) {  /* just '&' */
    *line[0] = 0; /*  consume line */
    return 0;  /*  '&' is valid. */
  }
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
  for (int i=0; i<children_count; i++) wait(NULL);
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
  int status = 0;

  if ((*line)[0] == sign[0]) return -1;   /*  starting with '>' is invalid. */
  int redirection_exists = get_output_path(&path, *line, sign);
  (void)trim_string(&path);
  for(int i=0; i< strlen(path); i++){ /* path with whitespace is invalid */
    if(path[i] == ' ') return -1;
  }
  if (redirection_exists == 0) { /*  there is redirection sign? */
    status=redirect_stdout_to_file(path);
    *line = strsep(line,sign); /* keep the segment before '>' */
  }
  return status;
}
/**
 * print out prompt text in interactive mode
 *
 */
void prompt(void){
  if (g_stream == stdin) printf("wish> ");
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
  char * line = NULL;
  size_t len = 0; /*  len is unused. */
  int stdout_copy = dup(1);
  g_paths[0] = strdup("/bin");
  switch(argc){
  case 2: g_stream = fopen(argv[1], "r"); break;
  case 1: g_stream = stdin; prompt(); break;
  default: print_error_msg(); exit(EXIT_FAILURE);
  }
  if (g_stream == NULL) {
	print_error_msg();
	exit(EXIT_FAILURE);
  }
  int program_pid = getpid();
  /*  parse program arguments */
  while(-1 != getline(&line, &len, g_stream)){ /*  while not EOF */
    line[strlen(line)-1] = 0; /*  remove newline at the end. */
    if (line[0] == '#') continue ; /*  ignore comment */
    if (execute_processes_in_parallel(&line) != 0 ||
        redirect_stdout_to_valid_file(&line) != 0){
      print_error_msg();
      prompt();
      continue;
    }/*  invalid parallel execution or invalid redirection ? */
    trim_string(&line);
    if ( is_builtin_cmd(line) == 0 ||     /*  built-in commands */
         strncmp(line, "", 1) == 0) { /* enter */
      prompt();
      continue;
    }
    pid_t pid = fork();     /*  fork and execute model */
    if (pid == 0){
      execute_command_after_fork(line);
    } /*  child process */
    else{
      wait(NULL);
      dup2(stdout_copy, 1); /*  restore standard output   */
      if (program_pid != getpid()) exit(0);
      prompt();
    } /*  wait for the child process to end */
  }
  exit(0);
}
