#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--PERSONAL INCLUDES--//
#include "FileSystem.h"
#include "File.h"
#include "functions.h"
#include "Shell.h"


//GLOBALS:
// DISK struct
Disk_Struct DISK;

// FAT struct
Fat_Struct FAT;

// CWD (Current Working Directory)
Folder_Struct* CWD;

//DISK name
char* string;

const static char* CMD_ARRAY[TOTAL_COMMANDS] = {
    "quit",     
    "clear",    
    "mk",       
    "rm",       
    "cat",      
    "mkdir",    
    "rmdir",    
    "cd",       
    "ls",      
    "edit",    
    "help",    
    "seek",
    "rename"     
};

int main(int argc, char** argv){
    printf("START\n");
    
    Disk_INIT(&DISK, argv[1]);
    printf("DISK_INIT\n");
    
    Fat_INIT(&DISK, &FAT);
    printf("FAT INIT\n");

    CWD = Root_INIT(&DISK, &FAT);
    printf("ROOT INIT\n");

    int n_args;
    int cmd_i;
    char input_msg[256];
    char input[MAX_INPUT];
    char split0[MAX_INPUT];
    char split1[MAX_INPUT];
    char* split_input[2] = {split0, split1};

    string = calloc(1, MAX_STR_LEN);
    strcpy(string, argv[1]);

    shell_INIT();
    shell_CLEAR(string);

    while(1){

        strncpy(input, "", MAX_INPUT);
        strncpy(split_input[0], "", MAX_INPUT);
        strncpy(split_input[1], "", MAX_INPUT);
        cmd_i = -1;

        sprintf(input_msg, "[usr]:%s >> ", CWD->FileHeader.Name);
        printf("\n");
        shell_READ(input, input_msg);

        n_args = split_str(input, split_input);

        if(n_args > 1){
            printf("TOO MANY ARGS, MAX 1\n");
            continue;
        }

        int flag = 0;
        for(int i=0; i<TOTAL_COMMANDS; ++i){
            if(shell_CHECK_INPUT(split_input[0], CMD_ARRAY[i])){
                cmd_i = i;
                flag = 1;
                break;
            }
        }

        if(strcmp(split_input[0], "quit") == 0){
            if (n_args == 0){
                printf("SAVING DISK...\n");
                SAVE(&DISK, &FAT, string);    

                free(string);
                free(CWD);

                printf("FAT DESTR\n");
                Fat_DESTR(&DISK, &FAT);

                printf("DISK DESTR\n");
                Disk_DESTR(&DISK);
            }
                    
            else{
                printf("(quit doesn't take any arguments)\n");
                continue;
            }
        }

        if(!flag){
            printf("INVALID INSTRUCTION\n(help to see the command list)\n");
            continue;    
        }
    
        (*FN_ARRAY[cmd_i])((void*)split_input[1]);
    }
    return 0;
}