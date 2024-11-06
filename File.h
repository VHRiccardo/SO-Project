#pragma once

#define MAX_FILE_INPUT (BLOCKS_SIZE - sizeof(File_Header))
#define MAX_ELEM_FB 216
#define MAX_ELEM_NB 255

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

typedef struct Folder_Struct{

    struct File_Header FileHeader;
    int NumFile;
    int PrevFolder;
    int IndexList [MAX_ELEM_FB];
    
} Folder_Struct;

typedef struct Folder_Struct_Next{
    
    int NumBlock;
    int IndexList[MAX_ELEM_NB];

}Folder_Struct_Next;


//FUNCTIONS:

//TO INIT OR CREATE:
Folder_Struct* Root_INIT(Disk_Struct* disk, Fat_Struct* fat);

void Folder_CREATE(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct* fs, char* name);

void File_CREATE(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct* fs, char* name);

void Folder_Struct_Next_INIT(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct_Next* fsn, int num_block, int index);


//TO DESTROY:
void Folder_DESTROY(Folder_Struct* fs);

void File_DESTROY(File_Struct* fs);

void Index_List_DESTROY(Folder_Struct *fs, int index);


//GETTER:
void File_GET_FILE(File_Struct* fs, int index);

void Folder_GET_FOLDER(Folder_Struct* fs, int index);

void File_Header_GET_FILE_HEADER(File_Header* fh, int index);

int* Index_GET_LIST(Folder_Struct* fs);


//TO MANAGE:
int CHANGE_CWD(Folder_Struct* newCWD, int index);

int Index_List_FIND(File_Header* fh, Folder_Struct* fs, char* name);

void Index_List_ADD(Folder_Struct* fs, int index);

void File_Header_RENAME(File_Header* fh, char* name);

int File_CHECK_IS_FOLDER(File_Header* fh);

int File_EXTEND_FILE(char* buffer, int prev);

int File_EDIT(File_Struct* fs);

void File_CAT(File_Struct* fs);

int File_SEEK(File_Struct* fs, int offset);

void File_PRINT(File_Struct* fs);