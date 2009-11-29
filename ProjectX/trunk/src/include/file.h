#ifndef FILE_H
#define FILE_H

#include "main.h"
#include <stdio.h>

void touch_file( char* path );
BOOL is_folder( char* path );
int folder_exists( char *pathspec, ... );
long Get_File_Size( char * Filename );
long Read_File( char * Filename, char * File_Buffer, long Read_Size );
long Write_File( char * Filename, char * File_Buffer, long Write_Size );
long Get_File_Size( char * Filename );
long Read_File( char * Filename, char * File_Buffer, long Read_Size );
void AddCommentToLog( char * str );
BOOL File_Exists( char * Filename );
BOOL delete_file( char * path );

char* find_file( char * path );
char* find_next_file( void );
void find_close( void );

#endif // FILE_H
