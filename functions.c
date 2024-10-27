#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--PERSONAL INCLUDES--//
#include "functions.h"

void Error_HANDLER(char* s){
    perror(s);
    exit(-1);
}

int split_str(char *string, char **array){
    int n = 0;
    char *tok = strtok(string, " ");

    while(tok != NULL){
        if(n == 0){strncpy(array[0], tok, 128);}
        if(n == 1){strncpy(array[1], tok, 128);}
        tok = strtok(NULL, " ");
        n++;
    }
    return n-1;
}

int validate_string(char* s){
    if(strlen(s) == 0){
        printf("INPUT can't be empty\n");
        return -1;
    }

    if(strlen(s) > 128){
        printf("INPUT too long\n");
        return -1;
    }
    char forbidden[] = "\\,/&\"()";
    for(int i = 0; i < strlen(s); i++){
        for(int j = 0; j < strlen(forbidden); j++){
            if(s[i] == forbidden[j]) return -1;
        }
    }
    
    return 1;
}