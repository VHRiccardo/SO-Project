#pragma once

#define TOTAL_COMMANDS 12
#define MAX_INPUT 128

#include "FileSystem.h"
#include "File.h"

extern Disk_Struct DISK;
extern Fat_Struct FAT;
extern Folder_Struct* CWD;

typedef int (*fun_ptr)(void*);

void shell_INIT();

void shell_CLEAR(char* s);

void shell_READ(char* input, char* msg);

int shell_CHECK_INPUT(char* s1, const char* s2);

//------------------------INSIDE SHELL FUNCTIONS-------------------------//

int _help(void* arg);

int _quit(void* arg);

int _clear(void* arg);

int _mk(void* arg);

int _rm(void* arg);

int _cat(void* arg);

int _mkdir(void* arg);

int _rmdir(void* arg);

int _cd(void* arg);

int _ls(void* arg);

int _edit(void* arg);

int _seek(void* arg);

//------------------------ARRAYS OF CMD AND FUN_PTR-------------------------//

const static fun_ptr FN_ARRAY[TOTAL_COMMANDS] = {
    _quit,      
    _clear,     
    _mk,        
    _rm,        
    _cat,       
    _mkdir,     
    _rmdir,     
    _cd,        
    _ls,        
    _edit,     
    _help,     
    _seek      
};