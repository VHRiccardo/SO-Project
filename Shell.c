#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--PERSONAL INCLUDES--//
#include "./linenoise/linenoise.h"
#include "FileSystem.h"
#include "File.h"
#include "functions.h"
#include "Shell.h"

extern Disk_Struct DISK;
extern Fat_Struct FAT;
extern Folder_Struct* CWD;
extern char* string;

void shell_CLEAR(char* s){
    printf("\033[H\033[J");
    printf("FAT PROJECT: %s\n\n", s);

}

//LINENOISE FUNCTION:
void completion(const char *buf, linenoiseCompletions *lc) {
    switch (buf[0]) {
        case 'q':
            linenoiseAddCompletion(lc,"quit");
            break;
        case 'c':
            linenoiseAddCompletion(lc,"cd");
            linenoiseAddCompletion(lc,"cat");
            linenoiseAddCompletion(lc,"clear");
            break;
        case 'e':
            linenoiseAddCompletion(lc, "edit");
            break;
        case 'm':
            linenoiseAddCompletion(lc, "mk");
            linenoiseAddCompletion(lc, "mkdir");
            break;
        case 'r':
            linenoiseAddCompletion(lc, "rm");
            linenoiseAddCompletion(lc, "rmdir");
            linenoiseAddCompletion(lc, "rename");
            break;
        case 'l':
            linenoiseAddCompletion(lc, "ls");
            break;
        case 'h':
            linenoiseAddCompletion(lc, "help");
            break;
        case 's':
            linenoiseAddCompletion(lc, "seek");
            break;
        default:
            linenoiseAddCompletion(lc, "help");
            break;
    }
}

void shell_INIT(){
    linenoiseSetMultiLine(0);
    linenoiseHistorySetMaxLen(MAX_INPUT);
    linenoiseSetCompletionCallback(completion);
    linenoiseHistoryLoad("linenoise_history.txt");
}

void shell_READ(char *input, char *msg){
    char* line = linenoise(msg);

    strncpy(input, line, MAX_INPUT);
    linenoiseHistoryAdd(line);
    linenoiseHistorySave("linenoise_history.txt");
    free(line);
}

int shell_CHECK_INPUT(char* s1, const char* s2){
    if(strncmp(s1, s2, MAX_INPUT) == 0){ 
        return 1;
    }else{
        return 0;
    }
}

//------------------------INSIDE SHELL FUNCTIONS-------------------------//

int _help(void* arg){
    if (strlen((char*)arg) > 0){
        printf("(help doesn't take any arguments)\n");
        return 0;
    }
    char help_msg[] = (
        "FAT commands:\n"
        "   quit: exits FAT Project\n"
        "   clear: clears the terminal\n"
        "   mk: creates a file\n"
        "   rm: deletes a file\n"
        "   cat: reads the content of a file and prints it to stdout\n"
        "   mkdir: creates a directory\n"
        "   rmdir: deletes a directory (and eventually its content)\n"
        "   cd: changes the current working directory\n"
        "   ls: lists the current directory content\n"
        "   edit: opens the file passed as argument to add content at the end of it\n"
        "   seek: opens the file passed as argument to add content inside of it\n"
        "   rename: change the name of a file or direcoty given in input\n"
    );
    printf("%s",help_msg);
    return 0;
}

int _quit(void* arg){
    printf("Exiting FAT PROJECT...\n");
    printf("THE END!\n");
    exit(0);
    return 0;
}

int _clear(void* arg){
    shell_CLEAR(string);
    return 0;
}

int _mk(void* arg){

    File_Header* fh = calloc(1, sizeof(File_Header));
    
    if(Index_List_FIND(fh, CWD, (char*)arg)){
        printf("Error: File name unavailable\n");
    }
    else if(validate_string(arg) == 1){
        File_CREATE(&DISK, &FAT, CWD, (char*)arg);
        printf("FILE CREATED: %s\n", (char*)arg);
    }else{
        printf("Error: Unable to create the file\n");
    }

    free(fh);
    return 0;
}

int _ls(void* arg){
    if (strlen((char*)arg) > 0){
        printf("(ls doesn't take any arguments)\n");
        return 0;
    }
   
    int n = CWD->NumFile;

    printf("\n");
    printf("NUM FILE: %d\n", n);
    printf("\n");

    if(n <= 0){return 0;}

    printf("NAME\tTYPE\tSIZE in byte\n");

    int* list = Index_GET_LIST(CWD);

    for(int i = 0; i < CWD->NumFile; i++){
        File_Header* fh = calloc(1, sizeof(File_Header));
        File_Header_GET_FILE_HEADER(fh, list[i]);
        printf("%s\t", fh->Name);
        if(File_CHECK_IS_FOLDER(fh)){
            printf("dir\t");
            printf("%d\n", fh->BSize* BLOCKS_SIZE);
        }else{
            printf("file\t");
            int dim = fh->Offset;
            printf("%d\n", dim);
        }
    }
    printf("\n");
    return 0;  
}

int _mkdir(void* arg){
    File_Header* fh = calloc(1, sizeof(File_Header));
    
    if(Index_List_FIND(fh, CWD, (char*)arg)){
        printf("Error: File name unavailable\n");
    }
    else if(validate_string(arg) == 1){
        Folder_CREATE(&DISK, &FAT, CWD, (char*)arg);
        printf("Folder CREATED: %s\n", (char*)arg);
    }else{
        printf("Error: Unable to create the folder\n");
    }

    free(fh);
    return 0;
}

int _cd(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(cd needs an argument)\n");
        return 0;
    }

    Folder_Struct* newCWD = calloc(1, sizeof(Folder_Struct));

    if(strcmp((char*)arg, "..") == 0){
        if(CWD->PrevFolder == -1){
            printf("Error: There is NO Father Folder\n");
            return 0;
        }
        else{
            CHANGE_CWD(newCWD, CWD->PrevFolder);
            free(newCWD);
            return 0;
        }
    }
    File_Header* fh = calloc(1, sizeof(File_Header));

    if(Index_List_FIND(fh, CWD, (char*)arg)== 0 || fh->IsFolder == 0){
        printf("Error: There is NO Folder with this name\n");
        return 0;
    }
    if(fh->IsFolder){
        CHANGE_CWD(newCWD, fh->FirstBlock);
    }else{
        free(fh);
        free(newCWD);
        return 0;
    }
    free(fh);
    free(newCWD);
    return 0;
}

int _rmdir(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(rmdir needs an argument)\n");
        return 0;
    }
    Folder_Struct* fol = calloc(1, sizeof(Folder_Struct));
    File_Header* fh = calloc(1, sizeof(File_Header));

    if(Index_List_FIND(fh, CWD, (char*)arg)== 0 || fh->IsFolder != 0){
        printf("Error: There is NO Folder with this name\n");
        return 0;
    }

    Folder_GET_FOLDER(fol, fh->FirstBlock);
    Folder_DESTROY(fol);

    Index_List_DESTROY(CWD, fh->FirstBlock);
    printf("FOLDER DESTROYED: %s\n", (char*)arg );
    
    free(fh);
    free(fol);
    return 0;
}

int _edit(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(edit needs an argument)\n");
        return 0;
    }

    File_Header* fh = calloc(1, sizeof(File_Header));

    if(Index_List_FIND(fh, CWD, (char*)arg)== 0 || fh->IsFolder != 0){
        printf("Error: There is NO File with this name\n");
        return 0;
    }

    File_Struct* file = calloc(1, sizeof(File_Struct));
    File_GET_FILE(file, fh->FirstBlock);

    File_EDIT(file);
    
    free(fh);
    free(file);
    return 0;
}

int _cat(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(cat needs an argument)\n");
        return 0;
    }

    File_Header* fh = calloc(1, sizeof(File_Header));

    if(Index_List_FIND(fh, CWD, (char*)arg)== 0 || fh->IsFolder != 0){
        printf("Error: There is NO File with this name\n");
        return 0;
    }

    File_Struct* file = calloc(1, sizeof(File_Struct));
    File_GET_FILE(file, fh->FirstBlock);

    File_CAT(file);
    
    free(fh);
    free(file);
    return 0;
}

int _seek(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(seek needs an argument)\n");
        return 0;
    }
    
    File_Header* fh = calloc(1, sizeof(File_Header));

    if(Index_List_FIND(fh, CWD, (char*)arg)== 0 || fh->IsFolder != 0){
        printf("Error: There is NO Filr with this name\n");
        return 0;
    }

    File_Struct* file = calloc(1, sizeof(File_Struct));
    File_GET_FILE(file, fh->FirstBlock);


    int offset;
    printf("Insert the offset: ");
    scanf("%d", &offset);
    getchar();

    File_SEEK(file, offset);

    free(fh);
    free(file);
    return 0;
}

int _rm(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(rm needs an argument)\n");
        return 0;
    }
    
    File_Header* fh = calloc(1, sizeof(File_Header));

    if(Index_List_FIND(fh, CWD, (char*)arg)== 0 || fh->IsFolder == 0){
        printf("Error: There is NO File with this name");
        return 0;
    }


    File_Struct* file = calloc(1, sizeof(File_Struct));
    File_GET_FILE(file, fh->FirstBlock);
    File_DESTROY(file);

    Index_List_DESTROY(CWD, fh->FirstBlock);
    printf("FILE DESTROYED: %s\n", (char*)arg );

    free(fh);
    free(file);
    return 0;
}

int _rename(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(rename needs an argument)\n");
        return 0;
    }

    File_Header* fh = calloc(1, sizeof(File_Header));

    if(Index_List_FIND(fh, CWD, (char*)arg)== 0){
        printf("Error: There is NO File or Folder with this name");
        return 0;
    }
    char new_name[MAX_INPUT];
    printf("Insert the new name: ");
    scanf("%s", new_name);

    if(validate_string(arg) == 1){
        File_Header* fh1 = calloc(1, sizeof(File_Header));
        if(Index_List_FIND(fh1, CWD, new_name) == 0){
            File_Header_RENAME(fh, new_name);
            free(fh1);
        }else{
            printf("Error: This name is already in use");
            free(fh1);
            return 0;
        }
    }else{
        printf("Error: The input string is NOT valid");
    }
    free(fh);
    return 0;
}
