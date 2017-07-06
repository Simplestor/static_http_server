#include <string.h>

int strcutfront(char* string,char* substring)
{
    int i = 0;
    int length = strlen(substring);
    if(length == 0){
        return 0;
    }
    while(*string){
        if(strncmp(string, substring, length) == 0){
            return i;
        }
        string++;
        i++;
    }
    return 0;

}

char* strcutbehind(char* string,char* substring){
    int length = strlen(substring);
    if(length == 0){
        return NULL;
    }
    while(*string){
        if(strncmp(string, substring, length) == 0){
            return string + length;
        }
        string++;
    }
    return NULL;
}

