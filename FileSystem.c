#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//--PERSONAL INCLUDES--//
#include "FileSystem.h"
#include "File.h"
#include "functions.h"

//GLOBAL
int FAT_INDEX[FAT_STRUCT_SIZE/BLOCKS_SIZE];


//DISK FUNCTIONS:
void Disk_INIT(Disk_Struct* disk, char* name){
    if(name == NULL || strcmp(name, "") == 0){
        Error_HANDLER("Empty Name");
    }
    if(strlen(name) > MAX_STR_LEN){
        Error_HANDLER("Invalid Disk Name");
    }

    if(validate_string(name)){
        strcpy(disk->SessionName, name);
        disk->IsValid = 1;
        disk->Buffer = (char*) mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        if(disk->Buffer == MAP_FAILED){
            Error_HANDLER("Invalid Disk Mapping");
        }

        if(access(name, R_OK | W_OK) == 0){
            FILE* f = fopen(name, "r+");
            int ret = fread(disk->Buffer, 1, SIZE, f);
            if(ret == 0){
                Error_HANDLER("Impossible to Download DISK data");
            }
            fclose(f);

        }else{
            memset(disk->Buffer, 0, SIZE);
        }

    }else{
        Error_HANDLER("Invalid Disk Name");
    }
    return;
}

void Disk_DESTR(Disk_Struct* disk){
    if(!(disk->IsValid)){
        Error_HANDLER("Disk NOT Existing");
    }
    if(munmap(disk->Buffer, SIZE) == -1){
        Error_HANDLER("Failed to Unmap the Disk");
    }
}


//FAT FUNCTIONS:
void Fat_INIT(Disk_Struct* disk, Fat_Struct* fat){
    if(!(disk->IsValid)){
        Error_HANDLER("Disk NOT Existing");
    }
    
    memcpy(fat, disk->Buffer, FAT_STRUCT_SIZE);

    if((fat->IsValid)){

        fat->IsPerma = 1;
        int index = 0;
        for(int i = 0; i < FAT_STRUCT_SIZE/BLOCKS_SIZE; i++){
            FAT_INDEX[i] = index;
            index = Fat_GET_NEXT(fat, index);
        }
        return;
    }else{

        fat->IsValid = 1;
        fat->IsPerma = 0;
        memset(fat->BMap, 0, FAT_SIZE);
        memset(fat->BlockList, -1, FAT_SIZE*sizeof(int));
        fat->FreeBlocks = FAT_SIZE;

        int iteraction = FAT_STRUCT_SIZE/BLOCKS_SIZE;
        if(FAT_STRUCT_SIZE%BLOCKS_SIZE != 0){
            iteraction++;
        }

        for(int i = 0; i < iteraction; i++){
            ALLOC_BLOCK(disk, fat, i);
            FAT_INDEX[i] = i;

            if(i+1 != iteraction){
                Fat_SET_NEXT(fat, i, i+1);
            }
        }
    }
}

void Fat_DESTR(Disk_Struct* disk, Fat_Struct* fat){
    if (!(fat->IsValid)){
        Error_HANDLER("Invalid FAT");
    }
    else{
        memset(fat->BMap, 0, FAT_SIZE);
        memset(fat->BlockList, -1, FAT_SIZE*sizeof(int));
        fat->FreeBlocks = FAT_SIZE;
        memset(disk->Buffer, 0, SIZE);
    } 
    return;
}

char Fat_BLOCK_IS_FREE(Fat_Struct* fat, int index){
    if(fat->BMap[index])
        return 0;
    else 
        return 1;
}

int Fat_GET_NEXT(Fat_Struct* fat, int index){
    return fat->BlockList[index];
}

void Fat_SET_NEXT(Fat_Struct* fat, int index, int next_index){
    fat->BlockList[index] = next_index;
    return;
}

int Fat_FIND_NEXT_FREE(Fat_Struct* fat){
    int i = 0;
    while (i < FAT_SIZE) {
        if(Fat_BLOCK_IS_FREE(fat, i))
            break;
        else
            i++;
    }
    return i;   
}


//FUNCTIONS for BOTH
void ALLOC_BLOCK(Disk_Struct* disk, Fat_Struct* fat, int index){
    if(!(disk->IsValid)){
        Error_HANDLER("Disk NOT Existing");
    }
    memset(disk->Buffer + index*BLOCKS_SIZE, 0, BLOCKS_SIZE);
    if(Fat_BLOCK_IS_FREE(fat, index)){
        fat->BMap[index] = 1;
        fat->BlockList[index] = -1;
        fat->FreeBlocks--;
        return;
    }
    Error_HANDLER("Block Allocation Failed");
} 

void DEALLOC_BLOCK(Disk_Struct* disk, Fat_Struct* fat, int index){
    if(!(disk->IsValid)){
        Error_HANDLER("Disk NOT Existing");
    }
    
    memset(disk->Buffer + index*BLOCKS_SIZE, 0, BLOCKS_SIZE);
    
    if(Fat_BLOCK_IS_FREE(fat, index)) 
        return;
    fat->BMap[index] = 0;
    fat->BlockList[index] = -1;
    fat->FreeBlocks++;
    return;
}

void WRITE(Disk_Struct* disk, Fat_Struct* fat, int index, char* buffer){
    if(!(disk->IsValid)){
        Error_HANDLER("Disk NOT Existing");
    }
    if(!(fat->IsValid)){
        Error_HANDLER("FAT NOT Existing");
    }
    if(index*BLOCKS_SIZE >= SIZE){
        Error_HANDLER("Index out of bound");
    }
    int n_bytes = sizeof(buffer);
    int n_blocks = n_bytes/BLOCKS_SIZE;

    int next_i = Fat_FIND_NEXT_FREE(fat);

    if(n_bytes%BLOCKS_SIZE != 0){
        n_blocks++;
    }
    
    for(int i = 0; i < n_blocks; i++){ 
        ALLOC_BLOCK(disk, fat, index);
        memcpy(disk->Buffer + (index * BLOCKS_SIZE), buffer + (i*BLOCKS_SIZE), BLOCKS_SIZE);

        if(i < n_blocks -1){
            next_i = Fat_FIND_NEXT_FREE(fat);
            ALLOC_BLOCK(disk, fat, next_i);
            Fat_SET_NEXT(fat, index, next_i);
            index = next_i;
        }
    }
}

void REWRITE_BLOCK(Disk_Struct* disk, Fat_Struct* fat, int index, char* buffer){
    if(!(disk->IsValid)){
        Error_HANDLER("Disk NOT Existing");
    }
    if(!(fat->IsValid)){
        Error_HANDLER("FAT NOT Existing");
    }
    if(index*BLOCKS_SIZE >= SIZE){
        Error_HANDLER("Index out of bound");
    }
    memcpy(disk->Buffer + (index * BLOCKS_SIZE), buffer, BLOCKS_SIZE);

}

void REWRITE_HEADER(Disk_Struct* disk, Fat_Struct* fat, int index, char* buffer){
    if(!(disk->IsValid)){
        Error_HANDLER("Disk NOT Existing");
    }
    if(!(fat->IsValid)){
        Error_HANDLER("FAT NOT Existing");
    }
    if(index*BLOCKS_SIZE >= SIZE){
        Error_HANDLER("Index out of bound");
    }
    memcpy(disk->Buffer + (index * BLOCKS_SIZE), buffer, sizeof(File_Header));

}

char* READ(Disk_Struct* disk, Fat_Struct* fat, int index, int size){
    if(!(disk->IsValid)){
        Error_HANDLER("Disk NOT Existing");
    }
    if(!(fat->IsValid)){
        Error_HANDLER("FAT NOT Existing");
    }
    if(index*BLOCKS_SIZE >= SIZE){
        Error_HANDLER("Index out of bound");
    }

    char* return_buffer = (char*)malloc(size);

    int n_bytes = size;
    int n_blocks = n_bytes/BLOCKS_SIZE;
    int next_i;

    if(n_bytes%BLOCKS_SIZE != 0){
        n_blocks++;
    }

    for(int i = 0; i < n_blocks; i++){

        memcpy(return_buffer + (i*BLOCKS_SIZE), disk->Buffer + (index * BLOCKS_SIZE), BLOCKS_SIZE);

        if(i < n_blocks -1){
            if(fat->BlockList[index] == -1) return return_buffer;
            next_i = Fat_GET_NEXT(fat, index);
            index = next_i;
        }
    }
    return return_buffer;
}

void SAVE(Disk_Struct* disk, Fat_Struct* fat, char* name){

    char* block_buffer = calloc(1, BLOCKS_SIZE);
    for(int i = 0; i < FAT_STRUCT_SIZE/BLOCKS_SIZE; i++){
        memcpy(block_buffer, (char*)fat + (i*BLOCKS_SIZE), BLOCKS_SIZE);
        REWRITE_BLOCK(disk, fat, FAT_INDEX[i], block_buffer);
    }
    free(block_buffer);

    FILE* f = fopen(name, "w+");
    int ret = fwrite(disk->Buffer, 1, SIZE, f);

    if(ret == 0){
        Error_HANDLER("Impossible to save DISK data");
    }
    fclose(f);    

}