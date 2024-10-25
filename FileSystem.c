#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

//--PERSONAL INCLUDES--//
#include "FileSystem.h"
#include "functions.h"

//DISK FUNCTIONS:
void Disk_INIT(Disk_Struct* disk, char* name){
    if(disk->IsValid){
        Error_HANDLER("ERROR: Disk Already Existing");
    }
    if(strlen(name) > MAX_STR_LEN){
        Error_HANDLER("ERROR: Invalid Disk Name");
    }
    if(name == NULL || strcmp(name, "") == 0){
        Error_HANDLER("ERROR: Empty Name");
    }

    disk->IsValid = 1;
    strcpy(disk->SessionName, name);

    disk->Buffer = (char*) mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if(disk->Buffer == MAP_FAILED){
        Error_HANDLER("ERROR: Invalid Disk Mapping");
    }

    memset(disk->Buffer, 0, SIZE);
}

void Disk_DESTR(Disk_Struct* disk){
    if(!(disk->IsValid)){
        Error_HANDLER("ERROR: Disk NOT Existing");
    }
    if(munmap(disk->Buffer, SIZE) == -1){
        Error_HANDLER("ERROR: Failed to Unmap the Disk");
    }
}


//FAT FUNCTIONS:
void Fat_INIT(Disk_Struct* disk, Fat_Struct* fat){
    if(!(disk->IsValid)){
        Error_HANDLER("ERROR: Disk NOT Existing");
    }
    if((fat->IsValid)){
        Error_HANDLER("ERROR: FAT ALREADY Existing");
    }

    fat->IsValid = 1;
    memset(fat->BMap, 0, FAT_SIZE);
    memset(fat->BlockList, -1, FAT_SIZE*sizeof(int));
    fat->FreeBlocks = FAT_SIZE;

    for(int i = 0; i < FAT_STRUCT_SIZE/BLOCKS_SIZE; i++){
        ALLOC_BLOCK(disk, fat, i);

        if(i+1 != FAT_STRUCT_SIZE/BLOCKS_SIZE){
            Fat_SET_NEXT(fat, i, i+1);
        }
    }

}

void Fat_DESTR(Disk_Struct* disk, Fat_Struct* fat){
    if (!(fat->IsValid)){
        perror("Error: Invalid FAT");
        exit(-1);
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
        Error_HANDLER("ERROR: Disk NOT Existing");
    }
    memset(disk->Buffer + index*BLOCKS_SIZE, 0, BLOCKS_SIZE);
    if(Fat_BLOCK_IS_FREE(fat, index)){
        fat->BMap[index] = 1;
        fat->BlockList[index] = -1;
        fat->FreeBlocks--;
        return;
    }
    Error_HANDLER("ERROR: Block Allocation Failed");
} 

void DEALLOC_BLOCK(Disk_Struct* disk, Fat_Struct* fat, int index){
    if(!(disk->IsValid)){
        Error_HANDLER("ERROR: Disk NOT Existing");
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
        Error_HANDLER("ERROR: Disk NOT Existing");
    }
    if(!(fat->IsValid)){
        Error_HANDLER("ERROR: FAT NOT Existing");
    }
    if(index*BLOCKS_SIZE >= SIZE){
        Error_HANDLER("ERROR: Index out of bound");
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
        Error_HANDLER("ERROR: Disk NOT Existing");
    }
    if(!(fat->IsValid)){
        Error_HANDLER("ERROR: FAT NOT Existing");
    }
    if(index*BLOCKS_SIZE >= SIZE){
        Error_HANDLER("ERROR: Index out of bound");
    }
    memcpy(disk->Buffer + (index * BLOCKS_SIZE), buffer, BLOCKS_SIZE);

}

char* READ(Disk_Struct* disk, Fat_Struct* fat, int index, int size){
    if(!(disk->IsValid)){
        Error_HANDLER("ERROR: Disk NOT Existing");
    }
    if(!(fat->IsValid)){
        Error_HANDLER("ERROR: FAT NOT Existing");
    }
    if(index*BLOCKS_SIZE >= SIZE){
        Error_HANDLER("ERROR: Index out of bound");
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