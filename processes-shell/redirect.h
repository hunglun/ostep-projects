/**
 * @file   redirect.h
 * @author O Hung Lun <hunglun.o@gmail.com>
 * @date   Thu Dec 12 22:11:02 2019
 * 
 * @brief  Support Redirection of Standard Output to File
 * 
 * 
 */
#ifndef redirect_h
#define redirect_h

int convert_string_to_array(char *** pdest, char * src);
int get_output_path(char ** path, char * line, char * sign);
int redirect_stdout_to_file(char * path);
void trim_string(char ** str);

#endif //redirect_h
