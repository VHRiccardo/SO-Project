#pragma once 

#define MAX_STR_LEN 128
#define BLOCKS_SIZE 1024
#define SIZE (BLOCKS_SIZE*1000)
#define FAT_SIZE (SIZE/BLOCKS_SIZE)
#define FAT_STRUCT_SIZE sizeof(Fat_Struct)

typedef struct Disk_Struct{

    char IsValid;
    char* Buffer;
    char SessionName[MAX_STR_LEN];

}Disk_Struct;

typedef struct Fat_Struct{

    char IsValid;
    int FreeBlocks;
    int BlockList[FAT_SIZE];
    char BMap[FAT_SIZE];
    char IsPerma;
    char padding[111];

}Fat_Struct;

//DISK FUNCTIONS:

void Disk_INIT(Disk_Struct* disk, char* name);

void Disk_DESTR(Disk_Struct* disk);


//FAT FUNCTIONS:

void Fat_INIT(Disk_Struct* disk, Fat_Struct* fat);

void Fat_DESTR(Disk_Struct* disk, Fat_Struct* fat);

char Fat_BLOCK_IS_FREE(Fat_Struct* fat, int index);

int Fat_GET_NEXT(Fat_Struct* fat, int index);

void Fat_SET_NEXT(Fat_Struct* fat, int index, int next_index);

int Fat_FIND_NEXT_FREE(Fat_Struct* fat);


//FUNCTIONS for BOTH:

void ALLOC_BLOCK(Disk_Struct* disk, Fat_Struct* fat, int index); 

void DEALLOC_BLOCK(Disk_Struct* disk, Fat_Struct* fat, int index);

void WRITE(Disk_Struct* disk, Fat_Struct* fat, int index, char* buffer);

void REWRITE_BLOCK(Disk_Struct* disk, Fat_Struct* fat, int index, char* buffer);

char* READ(Disk_Struct* disk, Fat_Struct* fat, int index, int size);

void SAVE(Disk_Struct* disk, Fat_Struct* fat, char* name);