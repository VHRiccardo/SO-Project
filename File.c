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
    File_Header* root = calloc(1, sizeof(File_Header));

    root->IsFolder = 1;
    strcpy(root->Name, "root");
    root->BSize = sizeof(File_Header);
    root->Offset = 0;

    int first_block = Fat_FIND_NEXT_FREE(fat);
    
    root->FirstBlock = first_block;
    root->CurrentBlock = first_block;
    root->NextBlock = -1;

    Folder_Struct* root_srtuct = calloc(1, sizeof(Folder_Struct));
    root_srtuct->NumFile = 0;
    root_srtuct->FileHeader = *root;
    root_srtuct->PrevFolder = -1;


    char* buffer = calloc(1, sizeof(Folder_Struct));
    memcpy(buffer, root_srtuct, sizeof(Folder_Struct));

    WRITE(disk, fat, first_block, buffer);

    printf("SPERMA: %d\n", fat->IsPerma);
    if(fat->IsPerma){
        memcpy(root_srtuct, disk->Buffer + (5*BLOCKS_SIZE), BLOCKS_SIZE);
    }

    free(buffer);
    free(root);
    return root_srtuct;
}

void Folder_CREATE(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct* fh, char* name){
    File_Header* new_folder = calloc(1, sizeof(File_Header));
    
    new_folder->IsFolder = 1;
    strcpy(new_folder->Name, name);
    new_folder->BSize = sizeof(Folder_Struct);
    new_folder->Offset = 0;

    int first_block = Fat_FIND_NEXT_FREE(fat);
    
    new_folder->FirstBlock = first_block;
    new_folder->CurrentBlock = first_block;
    new_folder->NextBlock = -1;

    Folder_Struct* new_folder_srtuct = calloc(1, sizeof(Folder_Struct));

    new_folder_srtuct->NumFile = 0;
    new_folder_srtuct->FileHeader = *new_folder;

    new_folder_srtuct->PrevFolder = fh->FileHeader.FirstBlock;

    char* buffer = calloc(1, sizeof(Folder_Struct));
    memcpy(buffer, new_folder_srtuct, sizeof(Folder_Struct));

    WRITE(disk, fat, first_block, buffer);

    List_Elem* listelem = calloc(1, sizeof(List_Elem));
    strcpy(listelem->Name, name);
    listelem->IsFolder = 1;
    listelem->FirstBlock = first_block;
    listelem->NumBlocks = 1;

    List_Elem_ADD(fh, listelem);

    free(new_folder);
    free(new_folder_srtuct);
    free(listelem);
    free(buffer);

}

void File_CREATE(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct* fh, char* name){
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

    List_Elem* listelem = calloc(1, sizeof(List_Elem));
    strcpy(listelem->Name, name);
    listelem->IsFolder = 0;
    listelem->FirstBlock = first_block;
    listelem->NumBlocks = 0;

    List_Elem_ADD(fh, listelem);

    free(new_file);
    free(new_file_srtuct);
    free(listelem);
    free(buffer);
}

void Folder_Struct_Next_INIT(Disk_Struct* disk, Fat_Struct* fat, Folder_Struct_Next* fsn, int num_block, int index){
    fsn->NumBlock = num_block;
    for(int i = 0; i < MAX_ELEM_NB; i++){
        strcpy(fsn->FileList[i].Name, "");
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

void List_Elem_DESTROY(Folder_Struct* fs, List_Elem* list){
    int i = 0;
    while(i < MAX_ELEM_FB){
        if(strcmp(fs->FileList[i].Name, list->Name) == 0){
                memcpy(&fs->FileList[i].Name, "", 1);
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

            if(strcmp(fsn->FileList[i].Name, list->Name) == 0){

                memcpy(&fsn->FileList[i].Name, "", 1);

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
void File_GET_FILE(File_Struct* dest, int index){
    char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
    memcpy(dest, buffer_block, sizeof(File_Struct));

    free(buffer_block);
    return;
}

void Folder_GET_FOLDER(Folder_Struct* dest, int index){
    char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
    memcpy(dest, buffer_block, sizeof(Folder_Struct));

    free(buffer_block);
    return;
}

void File_GET_LIST(Folder_Struct* fs, List_Elem* list){
    
    int i = 0;
    int id = 0;
    while(i < MAX_ELEM_FB){

        if(strcmp(fs->FileList[i].Name, "") != 0){
                memcpy(&list[id], &fs->FileList[i], sizeof(List_Elem));
                id++;
            }
        i++;
    }

    int index = Fat_GET_NEXT(&FAT, fs->FileHeader.CurrentBlock);
    if(index == -1){return;}

    while(index != -1){
        Folder_Struct_Next* fsn = calloc(1, sizeof(Folder_Struct_Next));
        char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
        memcpy(fsn, buffer_block, sizeof(Folder_Struct_Next));
        
        int elem = 0;
        while(elem < MAX_ELEM_NB){

            if(strcmp(fsn->FileList[elem].Name, "") != 0){
                memcpy(&list[id], &fsn->FileList[elem], sizeof(List_Elem));
                id++;
            }
            elem++;    
        } 
        index = Fat_GET_NEXT(&FAT, index);
        free(buffer_block);
        free(fsn);
    }
    
}


//TO MANAGE:
int CHANGE_CWD(Folder_Struct* newCWD, int index){
    
    char* buffer_block = READ(&DISK, &FAT, index, BLOCKS_SIZE);
    memcpy(newCWD, buffer_block, sizeof(Folder_Struct));

    CWD->FileHeader = newCWD->FileHeader;
    CWD->NumFile = newCWD->NumFile;
    CWD->PrevFolder = newCWD->PrevFolder;
    memcpy(CWD->FileList, newCWD->FileList, sizeof(List_Elem)*5);    


    free(buffer_block);
    return 0;
}

int List_Elem_FIND(Folder_Struct* fs, List_Elem* dest, char* f_name){
    int n = CWD->NumFile;
    List_Elem list[n];
    File_GET_LIST(CWD, list);

    for(int i = 0; i < n; i++){
        if(strcmp(list[i].Name, f_name) == 0){
            memcpy(dest, &list[i], sizeof(List_Elem));
            return 1;
        }
    }
    dest = NULL;
    return 0;
}

void List_Elem_ADD(Folder_Struct* dest, List_Elem* source){

    for(int i = 0; i < MAX_ELEM_FB; i++){
        if (strcmp(dest->FileList[i].Name, "") == 0){
            memcpy(&dest->FileList[i], source, sizeof(List_Elem));
            dest->FileHeader.BSize += sizeof(List_Elem); 
            dest->NumFile++;
            REWRITE_BLOCK(&DISK, &FAT, dest->FileHeader.FirstBlock, (char*)dest);
            return; 
        }
    }    

    Folder_Struct_Next* fsn = calloc(1, sizeof(Folder_Struct_Next));

    int i = 1;
    int index = Fat_GET_NEXT(&FAT, dest->FileHeader.FirstBlock);
    int prev = 0;

    while(index != -1){
        char* buffer = READ(&DISK, &FAT, index, BLOCKS_SIZE);
        memcpy(fsn, buffer, BLOCKS_SIZE);

        for(int i = 0; i < MAX_ELEM_NB; i++){

            if(strcmp(fsn->FileList[i].Name, "") == 0){
                
                memcpy(&fsn->FileList[i], source, sizeof(List_Elem));

                dest->FileHeader.BSize += sizeof(List_Elem); 
                dest->NumFile++;
                dest->FileHeader.CurrentBlock = dest->FileHeader.FirstBlock;
                REWRITE_BLOCK(&DISK, &FAT, dest->FileHeader.FirstBlock, (char*)dest);
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

    if(index == -1){

        if(prev == 0){
            prev = dest->FileHeader.FirstBlock; 
        }

        Folder_Struct_Next_INIT(&DISK, &FAT, fsn, i, prev);
        memcpy(&fsn->FileList[0], source, sizeof(List_Elem));

        index = Fat_GET_NEXT(&FAT, prev);
        REWRITE_BLOCK(&DISK, &FAT, index, (char*)fsn);

        dest->FileHeader.BSize += sizeof(List_Elem); 
        dest->NumFile++;

        REWRITE_BLOCK(&DISK, &FAT, dest->FileHeader.FirstBlock, (char*)dest);

        free(fsn);
    }
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
    printf("\n----- FILE %s -----\n\n", fs->FileHeader.Name);
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