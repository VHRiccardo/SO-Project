#include <linux/limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//--PERSONAL INCLUDES--//
#include "FileSystem.h"
#include "File.h"

extern Disk_Struct DISK;
extern Fat_Struct FAT;
extern Folder_Struct* CWD;

//FUNCTIONS:

//TO INIT OR CREATE:
Folder_Struct* Root_INIT(Disk_Struct* disk, Fat_Struct* fat){
    Folder_Struct* root_srtuct = calloc(1, sizeof(Folder_Struct));

    if(fat->IsPerma){
        memcpy(root_srtuct, disk->Buffer + (5*BLOCKS_SIZE), BLOCKS_SIZE);
        return root_srtuct;
    }

    File_Header* root = calloc(1, sizeof(File_Header));

    root->IsFolder = 1;
    strcpy(root->Name, "root");
    root->BSize = 1;
    root->Offset = 0;

    int first_block = Fat_FIND_NEXT_FREE(fat);
    
    root->FirstBlock = first_block;
    root->CurrentBlock = first_block;
    root->NextBlock = -1;

    root_srtuct->NumFile = 0;
    root_srtuct->FileHeader = *root;
    root_srtuct->PrevFolder = -1;

    for(int i = 0; i < MAX_ELEM_FB; i ++){
        root_srtuct->IndexList[i] = -1;
    }

    char* buffer = calloc(1, sizeof(Folder_Struct));
    memcpy(buffer, root_srtuct, sizeof(Folder_Struct));

    WRITE(disk, fat, first_block, buffer);

    free(buffer);
    free(root);
    return root_srtuct;
}

void Folder_CREATE(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct* fs, char* name){
    File_Header* new_folder = calloc(1, sizeof(File_Header));
    
    new_folder->IsFolder = 1;
    strcpy(new_folder->Name, name);
    new_folder->BSize = 1;
    new_folder->Offset = 0;

    int first_block = Fat_FIND_NEXT_FREE(fat);
    
    new_folder->FirstBlock = first_block;
    new_folder->CurrentBlock = first_block;
    new_folder->NextBlock = -1;

    Folder_Struct* new_folder_srtuct = calloc(1, sizeof(Folder_Struct));

    new_folder_srtuct->NumFile = 0;
    new_folder_srtuct->FileHeader = *new_folder;

    new_folder_srtuct->PrevFolder = fs->FileHeader.FirstBlock;
    for(int i = 0; i < MAX_ELEM_FB; i ++){
        new_folder_srtuct->IndexList[i] = -1;
    }

    char* buffer = calloc(1, sizeof(Folder_Struct));
    memcpy(buffer, new_folder_srtuct, sizeof(Folder_Struct));

    WRITE(disk, fat, first_block, buffer);

    Index_List_ADD(fs, first_block);

    free(new_folder);
    free(new_folder_srtuct);
    free(buffer);
}

void File_CREATE(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct* fs, char* name){
    File_Header* new_file = calloc(1, sizeof(File_Header));
    
    new_file->IsFolder = 0;
    strcpy(new_file->Name, name);
    new_file->BSize = 1;
    new_file->Offset = 0;

    int first_block = Fat_FIND_NEXT_FREE(fat);
    
    new_file->FirstBlock = first_block;
    new_file->CurrentBlock = first_block;
    new_file->NextBlock = -1;

    File_Struct* new_file_srtuct = calloc(1, sizeof(File_Struct));
    new_file_srtuct->FileHeader = *new_file;
    for(int i = 0; i < strlen(new_file_srtuct->Data); i++){
        new_file_srtuct->Data[i] = 0;
    }

    char* buffer = calloc(1, sizeof(File_Struct));
    memcpy(buffer, (char*)(new_file_srtuct), sizeof(File_Struct));

    WRITE(disk, fat, first_block, buffer);

    Index_List_ADD(fs, first_block);

    free(new_file);
    free(new_file_srtuct);
    free(buffer);
}

void Folder_Struct_Next_INIT(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct_Next* fsn, int num_block, int index){
    fsn->NumBlock = num_block;
    for(int i = 0; i < MAX_ELEM_FB; i ++){
        fsn->IndexList[i] = -1;
    }
    int block = Fat_FIND_NEXT_FREE(fat);
    Fat_SET_NEXT(&FAT, index, block);

    char* buffer = calloc(1, sizeof(Folder_Struct_Next));
    memcpy(buffer, (char*)(fsn), sizeof(Folder_Struct_Next));

    WRITE(disk, fat, block, (char*)fsn);

    free(buffer);
}


//TO DESTROY:
void Folder_DESTROY(Folder_Struct* fs){
    strcpy(fs->FileHeader.Name, "");

    REWRITE_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock, (char*)fs);

    int index = Fat_GET_NEXT(&FAT, fs->FileHeader.FirstBlock);
    int next;
    while(index != -1){
        next = Fat_GET_NEXT(&FAT, index);
        DEALLOC_BLOCK(&DISK, &FAT, index);
        index = next;
    }

    DEALLOC_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock);

    return;
}

void File_DESTROY(File_Struct* fs){
    strcpy(fs->FileHeader.Name, "");

    for(int i = 0; i < sizeof(fs->Data); i++){
        memcpy(&fs->Data[i], "0", 1);
    }

    REWRITE_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock, (char*)fs);

    int index = Fat_GET_NEXT(&FAT, fs->FileHeader.FirstBlock);
    int next;
    while(index != -1){
        next = Fat_GET_NEXT(&FAT, index);
        DEALLOC_BLOCK(&DISK, &FAT, index);
        index = next;
    }

    DEALLOC_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock);

    return;
}

void Index_List_DESTROY(Folder_Struct *fs, int index){
    int i = 0;
    while(i < MAX_ELEM_FB){
        if(fs->IndexList[i] == index){
            fs->IndexList[i] = -1;
            CWD->NumFile --;
            REWRITE_BLOCK(&DISK, &FAT, CWD->FileHeader.FirstBlock, (char*)CWD);
            return;
        }
        i++;
    }

    int id = Fat_GET_NEXT(&FAT, fs->FileHeader.FirstBlock);
    if(id == -1){return;}
        
    int next_i;
    char* buffer_block;

    while(id != -1){
        Folder_Struct_Next* fsn = calloc(1, sizeof(Folder_Struct_Next));

        buffer_block = READ(&DISK, &FAT, id, BLOCKS_SIZE);
        memcpy(fsn, buffer_block, sizeof(Folder_Struct_Next));

        i = 0;
        while(i < MAX_ELEM_NB){

            if(fsn->IndexList[i] == index){
                fsn->IndexList[i] = -1;

                REWRITE_BLOCK(&DISK, &FAT, id, (char*)fsn);

                CWD->NumFile --;
                REWRITE_BLOCK(&DISK, &FAT, CWD->FileHeader.FirstBlock, (char*)CWD);

                free(buffer_block);

                return;
            }
            i++;    
        } 
        next_i = Fat_GET_NEXT(&FAT, id);
        id = next_i;
        free(fsn);
    }
    free(buffer_block); 
}


//GETTER:
void File_GET_FILE(File_Struct* fs, int index){
    char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
    memcpy(fs, buffer_block, sizeof(File_Struct));

    free(buffer_block);
    return;
}

void Folder_GET_FOLDER(Folder_Struct* fs, int index){
    char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
    memcpy(fs, buffer_block, sizeof(Folder_Struct));

    free(buffer_block);
    return;
}

void File_Header_GET_FILE_HEADER(File_Header* fh, int index){
    char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
    memcpy(fh, buffer_block, sizeof(File_Header));

    free(buffer_block);
    return;
}

int* Index_GET_LIST(Folder_Struct* fs){
    int* list = calloc(1, fs->NumFile * sizeof(int));
    int j = 0;
    for(int i = 0; i < MAX_ELEM_FB; i++){
        if(fs->IndexList[i] != -1){
            list[j] = fs->IndexList[i];
            j++;
        }
    }
    int index = Fat_GET_NEXT(&FAT, fs->FileHeader.FirstBlock);
    if(index == -1){return list;}


    while(index != -1){
        Folder_Struct_Next* fsn = calloc(1, sizeof(Folder_Struct_Next));
        char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
        memcpy(fsn, buffer_block, sizeof(Folder_Struct_Next));
        
        
        for(int i = 0; i < MAX_ELEM_NB; i++){
                if(fs->IndexList[i] != -1){
                list[j] = fs->IndexList[i];
                j++;
            }

        }
        free(fsn);
        index = Fat_GET_NEXT(&FAT, index);
    }
    return list;
}


//TO MANAGE:
int CHANGE_CWD(Folder_Struct* newCWD, int index){
    char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
    memcpy(newCWD, buffer_block, sizeof(Folder_Struct));

    CWD->FileHeader = newCWD->FileHeader;
    CWD->NumFile = newCWD->NumFile;
    CWD->PrevFolder = newCWD->PrevFolder;
    memcpy(CWD->IndexList, newCWD->IndexList, MAX_ELEM_FB);    

    free(buffer_block);
    return 0;
}

int Index_List_FIND(File_Header* fh, Folder_Struct* fs, char* name){

    int* list = Index_GET_LIST(fs);
    for(int i = 0; i < fs->NumFile; i++){
        File_Header_GET_FILE_HEADER(fh, list[i]);
        if(strcmp(fh->Name, name) == 0){
            return 1;
        }
    }
    return 0;
}

void Index_List_ADD(Folder_Struct* fs, int index){
        for(int i = 0; i < MAX_ELEM_FB; i ++){
        if(fs->IndexList[i] == -1){
            fs->IndexList[i] = index;

            fs->NumFile++;
        
            REWRITE_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock, (char*)fs);
            return; 
        }
    }

    Folder_Struct_Next* fsn = calloc(1, sizeof(Folder_Struct_Next));

    int i = 1;
    int id = Fat_GET_NEXT(&FAT, fs->FileHeader.FirstBlock);
    int prev = 0;

    while(index != -1){
        char* buffer = READ(&DISK, &FAT, index, BLOCKS_SIZE);
        memcpy(fsn, buffer, BLOCKS_SIZE);

        for(int i = 0; i < MAX_ELEM_NB; i++){

            if(fsn->IndexList[i]  == -1){
            
                fsn->IndexList[i] = index;

                fs->NumFile++;
            
                REWRITE_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock, (char*)fs);
                REWRITE_BLOCK(&DISK, &FAT, index, (char*)(fsn));
            
                free(fsn);
                free(buffer);
                return;
            }
        }

        i++;
        prev = index;
        index = Fat_GET_NEXT(&FAT, index);
        free(buffer);
    }

    if(id == -1){

        if(prev == 0){
            prev = fs->FileHeader.FirstBlock; 
        }

        Folder_Struct_Next_INIT(&DISK, &FAT, fsn, i, prev);
        fsn->IndexList[0] = index;

        id = Fat_GET_NEXT(&FAT, prev);
        REWRITE_BLOCK(&DISK, &FAT, id, (char*)fsn);

        fs->NumFile++;
        fs->FileHeader.BSize++;

        REWRITE_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock, (char*)fs);

        free(fsn);
    }
}

void File_Header_RENAME(File_Header* fh, char* name){

    strcpy(fh->Name, name);

    REWRITE_HEADER(&DISK, &FAT, fh->FirstBlock, (char*)fh);
    printf("SUCCESS RENAME: %s", name);

}

int File_CHECK_IS_FOLDER(File_Header* fh){
    if(fh->IsFolder){
        return 1;
    }
    return 0;
}

int File_EXTEND_FILE(char* buffer, int prev){
    int index = Fat_GET_NEXT(&FAT, prev);
    
    if(index == -1){
        index = Fat_FIND_NEXT_FREE(&FAT);
        Fat_SET_NEXT(&FAT, prev, index);
        WRITE(&DISK, &FAT, index, buffer);

    }else{
        REWRITE_BLOCK(&DISK, &FAT, index, buffer);
    }
    return index;

}

int File_EDIT(File_Struct* fs){
    printf("Write inside the file (ENTER to terminate):\n\n");
    char* buffer_block = READ(&DISK, &FAT, fs->FileHeader.FirstBlock, BLOCKS_SIZE*fs->FileHeader.BSize);

    char* file = NULL;

    if(fs->FileHeader.Offset > 0){
        file = calloc(1, fs->FileHeader.Offset);
        memcpy(file, buffer_block + sizeof(File_Header), fs->FileHeader.Offset);

        printf("FILE CONTENT:\n%s", file);

    }else{
        printf("FILE CONTENT:\n");

    }

    char* string = NULL;
    size_t size = 0;
    int byte_read;

    byte_read = getline(&string, &size, stdin);
    
    if(byte_read == -1){
        printf("Error while reading");
        free(buffer_block);
        free(file);
        return 0;
    }

    if(size > 0 && string[byte_read-1] == '\n'){
        string[byte_read-1] = '\0';
    }

    if(strlen(string) == 0){
        free(buffer_block);
        free(file);
        return 0;
    }

    if(file == NULL){
        file = calloc(1, byte_read);
        strcpy(file, string);
    }else{
        int old_dim = fs->FileHeader.Offset;
        int new_dim = old_dim + byte_read;

        file = realloc(file, new_dim +1);
        file[new_dim] = '\0';
        strcat(file, string);
    }

    int bsize = 1;
    if(strlen(file) < MAX_FILE_INPUT){
        memcpy(fs->Data, file, strlen(file));

    }else{
        memcpy(fs->Data, file, MAX_FILE_INPUT);
        int rest = strlen(file) - MAX_FILE_INPUT;

        int offset = MAX_FILE_INPUT;

        int index = fs->FileHeader.FirstBlock;
        char* buffer = calloc(1, BLOCKS_SIZE);

        while(rest > 0){      

            int dim = (rest >= BLOCKS_SIZE) ? BLOCKS_SIZE : rest;
            memcpy(buffer, file + offset, dim);

            index = File_EXTEND_FILE(buffer, index);

            offset += dim;
            rest -= dim;
            bsize++;

        }
        free(buffer);
    }

    fs->FileHeader.Offset = strlen(file) +1;
    fs->FileHeader.BSize = bsize;

    REWRITE_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock, (char*)fs);

    free(buffer_block);
    free(string);
    free(file);
    return 0;

}

void File_CAT(File_Struct* fs){
    printf("\n----- FILE %s -----\n", fs->FileHeader.Name);
    File_PRINT(fs);
    printf("------------------\n");
}

int File_SEEK(File_Struct* fs, int offset){
    if(offset < 0 || offset > fs->FileHeader.Offset){
        printf("Index out of bound");
        return 0;
    }

    char* buffer_block = READ(&DISK, &FAT, fs->FileHeader.FirstBlock, BLOCKS_SIZE*fs->FileHeader.BSize);

    char* file = NULL;

    if(fs->FileHeader.Offset > 0){
        file = calloc(1, fs->FileHeader.Offset);
        memcpy(file, buffer_block + sizeof(File_Header), fs->FileHeader.Offset);
    }

    char* string = NULL;
    size_t size = 0;
    int byte_read;

    byte_read = getline(&string, &size, stdin);
    
    if(byte_read == -1){
        printf("Error while reading");
        free(buffer_block);
        free(file);
        return 0;
    }

    if(size > 0 && string[byte_read-1] == '\n'){
        string[byte_read-1] = '\0';
    }

    if(strlen(string) == 0){
        free(buffer_block);
        free(file);
        return 0;
    }

    char* s1 = calloc(1, fs->FileHeader.Offset);
    char* s2 = calloc(1, byte_read + fs->FileHeader.Offset);
    char* aux = calloc(1, byte_read + fs->FileHeader.Offset);

    if(offset == 0){

        if(file == NULL){
            file = calloc(1, byte_read);
            strcpy(s2, string);
        }else{
            strcpy(s1, file);
            strcpy(s2, string);
            strcat(s2, s1);
        }

    }else{
        strncpy(s1, file, offset-1);
        strcpy(aux, file + (offset-1));


        printf("%s\n", s1);
        printf("\n\n--%s--\n\n", aux);

        strcat(s2, s1);
        strcat(s2, string);

    }
    int new_dim = strlen(s2);
    file = realloc(file, new_dim +1);
    strcpy(file, s2);

    int bsize = 1;
    if(strlen(file) < MAX_FILE_INPUT){
        memcpy(fs->Data, file, strlen(file));

    }else{
        memcpy(fs->Data, file, MAX_FILE_INPUT);
        int rest = strlen(file) - MAX_FILE_INPUT;

        int offset = MAX_FILE_INPUT;

        int index = fs->FileHeader.FirstBlock;
        char* buffer = calloc(1, BLOCKS_SIZE);

        while(rest > 0){      

            int dim = (rest >= BLOCKS_SIZE) ? BLOCKS_SIZE : rest;
            memcpy(buffer, file + offset, dim);

            index = File_EXTEND_FILE(buffer, index);

            offset += dim;
            rest -= dim;
            bsize++;

        }
        free(buffer);
    }

    fs->FileHeader.Offset = strlen(file) +1;
    fs->FileHeader.BSize = bsize;

    REWRITE_BLOCK(&DISK, &FAT, fs->FileHeader.FirstBlock, (char*)fs);

    File_PRINT(fs);

    free(buffer_block);
    free(file);
    free(string);
    free(s1);
    free(s2);
    free(aux);

    return 0;
}

void File_PRINT(File_Struct* fs){
    char* buffer_block = READ(&DISK, &FAT, fs->FileHeader.FirstBlock, BLOCKS_SIZE*fs->FileHeader.BSize);
    char* file = NULL;

    if(fs->FileHeader.Offset > 0){
        file = calloc(1, fs->FileHeader.Offset);
        memcpy(file, buffer_block + sizeof(File_Header), fs->FileHeader.Offset);
        printf("\nFILE CONTENT:\n%s\n", file);

    }else{
        printf("\nFILE CONTENT:\n");
    }

    free(buffer_block);
    free(file);
}