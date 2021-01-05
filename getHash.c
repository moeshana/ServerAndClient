#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"

int FD_FOR_GEN_HASH;

//replace later if find better one
//BKDR ALG FOR HASH 
unsigned int BKDRHash(char *str) {
    unsigned int seed = 131;
    unsigned int hash = 0;
    while (*str) {
        hash  = hash * seed + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}

//read a file then return the hash of the string/content in that file
//pass a file descriptor but didn't close it inside function
unsigned int readFileThenHash(int fd) {
    unsigned int res = 0;
    int tempSize = 1;
    int flag = -1;
    char buffer[1024];
    char *temp = (char *)malloc(sizeof(char) * tempSize);
    int count = 0;
    int readIn = 0;
    memset(temp, '\0', tempSize);
    do {
        do {
            flag = read(fd, buffer + readIn, 1024 - readIn);
            readIn += flag;        
        } while (readIn < 1023 && flag > 0);
        int i;
        for (i = 0; i < readIn; i++) {
            temp[count] = buffer[i];
            count++;
            //might segfault here
            if (count >= tempSize) {
                //printf("Size up!\n");
                tempSize = 2 * tempSize;
                char *new = (char *)malloc(sizeof(char) * tempSize);
                memset(new, '\0', tempSize);
                memcpy(new, temp, tempSize / 2);
                free(temp);
                temp = new;
            }
        }
        readIn = 0;
    } while (flag > 0);
    res = BKDRHash(temp);
    //printf("file is : \n%s",temp);
    free(temp);
    return res;
}

//read file and return content of this file
//return char * point to a heap address
//need to free
//fd : file descriptor
//len : length of the file
char *readFile(int fd, int len) {
    int tempSize = len;
    int flag = -1; 
    char buffer[len];
    char *temp = (char *)malloc(sizeof(char) * (tempSize + 1));
    int count = 0;
    int readIn = 0;
    memset(temp, '\0', tempSize + 1);
    do {
        flag = read(fd, temp + readIn, len - readIn);
        readIn += flag;
    } while (readIn < len - 1 && flag > 0);
    temp[len] = '\0';
    return temp;
}

//remove a file 
//project : is a path of the file
//file : file name only
//return 0 if success, otherwise -1
int removeFile(char *project, char *file) {
    char *filepath = combinePath(project, file);
    if (remove(filepath) == 0) {
        printf("[-]Successful removed file [%s/%s]\n", project, file);
        free(filepath);
        return 0;
    }
    free(filepath);
    return -1;
}

/*
int main(int argc, char * argv[]) {
    char a[] = "Junfeng and Eugenie\nheihei~~~~~\n";
    char *str = a;
    printf("res : %d\n",BKDRHash(str));
    FD_FOR_GEN_HASH = open("../server/Eugenie/us/chat.txt", O_RDONLY);
    printf("res : %u\n", readFileThenHash(FD_FOR_GEN_HASH));
    close(FD_FOR_GEN_HASH);
    return 0;
}
*/




