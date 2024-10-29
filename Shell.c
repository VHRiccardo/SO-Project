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
    // printf("\033[H\033[J");
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
    List_Elem *dest = calloc(1, sizeof(List_Elem));

    if(List_Elem_FIND(CWD, dest, (char*)arg)){
        printf("Error: File name unavailable\n");
    }
    else if(validate_string(arg) == 1){
        File_CREATE(&DISK, &FAT, CWD, (char*)arg);
        printf("FILE CREATED: %s\n", (char*)arg);
    }else{
        printf("Error: Unable to create the file\n");
    }

    free(dest);
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

    List_Elem list[n];
    File_GET_LIST(CWD, list);

    int i = 0;
    
    while(i < n){
        printf("%s\t", list[i].Name);
        if(list[i].IsFolder){
            printf("dir\t");
            printf("%d\n", (list[i].NumBlocks)*BLOCKS_SIZE);
        }else{
            printf("file\t");

            File_Struct* file = calloc(1, sizeof(File_Struct));
            int index = list[i].FirstBlock;
            File_GET_FILE(file, index);
            int dim = file->FileHeader.Offset;

            printf("%d\n", dim);

            free(file);
        }
        i++;
    }
    printf("\n");
    return 0;
  
}

int _mkdir(void* arg){
    List_Elem *dest = calloc(1, sizeof(List_Elem));

    if(List_Elem_FIND(CWD, dest, (char*)arg)){
        printf("Error: File name unavailable\n");
    }
    else if(validate_string(arg) == 1){
        Folder_CREATE(&DISK, &FAT, CWD, (char*)arg);
        printf("FOLDER CREATED: %s\n", (char*)arg);
    }else{
        printf("Error: Unable to create the folder\n");
    }

    free(dest);
    return 0;
}

int _cd(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(cd needs an argument)\n");
        return 0;
    }

    Folder_Struct* newCWD = calloc(1, sizeof(Folder_Struct));

    if(strcmp((char*)arg, "..") == 0){
        if(strcmp(CWD->PrevFolder.Name, "null") == 0){
            printf("Error: There is NO father folder");
            return 0;
        }
        else{
            CHANGE_CWD(newCWD, CWD->PrevFolder.FirstBlock);
            free(newCWD);
            return 0;
        }
    }

    
    List_Elem *listelem = calloc(1, sizeof(List_Elem));

    if(List_Elem_FIND(CWD, listelem, (char*)arg) == 0 || listelem->IsFolder == 0){
        printf("Error: There is NO Folder with this name");
        return 0;
    }
    if(listelem->IsFolder){
        CHANGE_CWD(newCWD, listelem->FirstBlock);

    }else{
        free(listelem);
        free(newCWD);
        return 0;
    }
    free(listelem);
    free(newCWD);
    return 0;
}

int _rmdir(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(rmdir needs an argument)\n");
        return 0;
    }
    Folder_Struct* fol = calloc(1, sizeof(Folder_Struct));

    List_Elem* listelem = calloc(1, sizeof(List_Elem));
    

    if(List_Elem_FIND(CWD, listelem, (char*)arg) == 0|| listelem->IsFolder == 0){
        printf("Error: There is NO Folder with this name\n");
        return 0;
    }

    Folder_GET_FOLDER(fol, listelem->FirstBlock);
    Folder_DESTROY(fol);

    List_Elem_DESTROY(CWD, listelem);
    printf("FOLDER DESTROYED: %s\n", (char*)arg );
    
    free(listelem);
    free(fol);
    return 0;
}

int _edit(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(edit needs an argument)\n");
        return 0;
    }

    List_Elem* listelem = calloc(1, sizeof(List_Elem));
    
    if(List_Elem_FIND(CWD, listelem, (char*)arg) == 0 || listelem->IsFolder != 0){
        printf("Error: There is NO File with this name");
        return 0;
    }

    File_Struct* file = calloc(1, sizeof(File_Struct));
    File_GET_FILE(file, listelem->FirstBlock);

    File_EDIT(file);
    
    free(listelem);
    free(file);
    return 0;
}

int _cat(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(cat needs an argument)\n");
        return 0;
    }

    List_Elem* listelem = calloc(1, sizeof(List_Elem));

    if(List_Elem_FIND(CWD, listelem, (char*)arg) == 0 || listelem->IsFolder != 0){
        printf("Error: There is NO File with this name");
        return 0;
    }

    File_Struct* file = calloc(1, sizeof(File_Struct));
    File_GET_FILE(file, listelem->FirstBlock);

    File_CAT(file);

    free(listelem);
    free(file);
    return 0;
}

int _seek(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(seek needs an argument)\n");
        return 0;
    }
    
    List_Elem* listelem = calloc(1, sizeof(List_Elem));

    if(List_Elem_FIND(CWD, listelem, (char*)arg) == 0 || listelem->IsFolder != 0){
        printf("Error: There is NO File with this name");
        return 0;
    }

    File_Struct* file = calloc(1, sizeof(File_Struct));
    File_GET_FILE(file, listelem->FirstBlock);


    int offset;
    printf("Insert the offset: ");
    scanf("%d", &offset);
    getchar();

    File_SEEK(file, offset);

    free(listelem);
    free(file);
    return 0;
}

int _rm(void* arg){
    if (strlen((char*)arg) == 0){
        printf("(rm needs an argument)\n");
        return 0;
    }
    
    List_Elem* listelem = calloc(1, sizeof(List_Elem));
    
    if(List_Elem_FIND(CWD, listelem, (char*)arg) == 0 || listelem->IsFolder != 0){
        printf("Error: There is NO File with this name\n");
        return 0;
    }  

    File_Struct* file = calloc(1, sizeof(File_Struct));
    File_GET_FILE(file, listelem->FirstBlock);
    File_DESTROY(file);

    List_Elem_DESTROY(CWD, listelem);
    printf("FILE DESTROYED: %s\n", (char*)arg );

    free(listelem);
    free(file);
    return 0;
}
