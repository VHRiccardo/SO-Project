#pragma once

#define MAX_FILE_INPUT (BLOCKS_SIZE - sizeof(File_Header))
#define MAX_ELEM_FB 5
#define MAX_ELEM_NB 7

//--PERSONAL INCLUDES--//
#include "FileSystem.h"

typedef struct File_Header{
    
    char IsFolder;
    char Name[MAX_STR_LEN];
    int BSize;
    int Offset;
    int FirstBlock;
    int NextBlock;
    int CurrentBlock;

} File_Header;

typedef struct File_Struct{

    struct File_Header FileHeader;
    char Data[MAX_FILE_INPUT];

} File_Struct;

typedef struct List_Elem{
    
    char IsFolder;
    int FirstBlock;
    int NumBlocks;
    char Name[MAX_STR_LEN];

}List_Elem;

typedef struct Folder_Struct{

    struct File_Header FileHeader;
    int NumFile;
    struct List_Elem PrevFolder;
    struct List_Elem FileList[MAX_ELEM_FB];
    char padding[28];

} Folder_Struct;

typedef struct Folder_Struct_Next{
    
    int NumBlock;
    struct List_Elem FileList[MAX_ELEM_NB];
    char padding[40];

}Folder_Struct_Next;


//FUNCTIONS:

//TO INIT OR CREATE:
Folder_Struct* Root_INIT(Disk_Struct* disk, Fat_Struct* fat);

void Folder_CREATE(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct* fh, char* name);

void File_CREATE(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct* fh, char* name);

void Folder_Struct_Next_INIT(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct_Next* fsn, int num_block, int index);


//TO DESTROY:
void Folder_DESTROY(Folder_Struct* fs);

void File_DESTROY(File_Struct* fs);

void List_Elem_DESTROY(Folder_Struct* fs, List_Elem* list);


//GETTER:
void File_GET_FILE(File_Struct* dest, int index);

void Folder_GET_FOLDER(Folder_Struct* dest, int index);

void File_GET_LIST(Folder_Struct* fs, List_Elem* list);


//TO MANAGE:
int CHANGE_CWD(Folder_Struct* newCWD, int index);

int List_Elem_FIND(Folder_Struct* fs, List_Elem* dest, char* f_name);

void List_Elem_ADD(Folder_Struct* dest, List_Elem* source);

int File_CHECK_IS_FOLDER(File_Header* fh);

int File_EXTEND_FILE(char* buffer, int prev);

int File_EDIT(File_Struct* fs);

void File_CAT(File_Struct* fs);

int File_SEEK(File_Struct* fs, int offset);

void File_PRINT(File_Struct* fs);