#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"

/*
this is a test helper class
this class for generating file to compare with our program
ignore this class
*/

// realFileName fakeFileName isFirstTime
void commit(char *argv[], char *tag) {
    int fd = open(argv[2], O_RDONLY);
    unsigned int hash = readFileThenHash(fd);
    char *h = myitoa(hash);
    close(fd);
    int fd2;
    if (!strcmp(argv[4], "true")) {
        fd2 = open("./shana2/.Commit",
                O_WRONLY|O_CREAT|O_TRUNC,
                S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
        write(fd2, "0\n", 2);
        write(fd2, tag, strlen(tag));
    } else {
        fd2 = open("./shana2/.Commit",
                O_WRONLY|O_CREAT|O_APPEND,
                S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
        write(fd2, tag, strlen(tag));
    }
    write(fd2, argv[3], strlen(argv[3]));
    write(fd2, ":", 1);
    write(fd2, h, strlen(h));
    write(fd2, "\n", 1);
    free(h);
    close(fd2);
}

//     2            3             4        5     6
// realFileName fakeFileName isFirstTime mver filever
void push(char *argv[]) {
    int fd = open(argv[2], O_RDONLY);
    unsigned int hash = readFileThenHash(fd);
    char *h = myitoa(hash);
    int fd2; 
    if (!strcmp(argv[4], "true")) {
        fd2 = open("./shana2/.Manifest",
                O_WRONLY|O_CREAT|O_TRUNC,
                S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
        write(fd2, argv[5], strlen(argv[5]));
        write(fd2, "\n",1);
    } else {
        fd2 = open("./shana2/.Manifest",
                O_WRONLY|O_CREAT|O_APPEND,
                S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    }
    write(fd2, argv[6], strlen(argv[6]));
    write(fd2,":",1);
    write(fd2, argv[3], strlen(argv[3]));
    write(fd2, ":", 1);
    write(fd2, h, strlen(h));
    write(fd2, "\n", 1);
    remove("./shana2/.Commit");
    close(fd2);
    free(h);
}
void update(char *argv[], char *tag) {
    int fd = open(argv[2], O_RDONLY);
    unsigned int hash = readFileThenHash(fd);
    char *h = myitoa(hash);
    close(fd);
    int fd2;
    if (!strcmp(argv[4], "true")) {
        fd2 = open("./shana2/.Update",
                O_WRONLY|O_CREAT|O_TRUNC,
                S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
        write(fd2, "0\n", 2);
        write(fd2, tag, strlen(tag));
    } else {
        fd2 = open("./shana2/.Update",
                O_WRONLY|O_CREAT|O_APPEND,
                S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
        write(fd2, tag, strlen(tag));
    }
    write(fd2, argv[3], strlen(argv[3]));
    write(fd2, ":", 1);
    write(fd2, h, strlen(h));
    write(fd2, "\n", 1);
    free(h);
    close(fd2);
    
}



void createCon(char *argv[]) {
    int fd = open(argv[3], O_RDONLY);
    unsigned int hash = readFileThenHash(fd);
    char *h = myitoa(hash);
    close(fd);
    int fd2;
    if (!strcmp(argv[4], "true")) {
        fd2 = open("./shana2/.Conflict",
                O_WRONLY|O_CREAT|O_TRUNC,
                S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
        write(fd2, "0\n", 2);
    } else {
        fd2 = open("./shana2/.Conflict",
                O_WRONLY|O_CREAT|O_APPEND,
                S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    }
    write(fd2, "[C]:", 4);
    write(fd2, argv[3], strlen(argv[3]));
    write(fd2, ":", 1);
    write(fd2, h, strlen(h));
    write(fd2, "\n", 1);
    free(h);
    close(fd2);
}


int main(int argc, char * argv[])
{
    //printf("This file will be used in Makefile...\n");
    int a = atoi(argv[1]);
    switch(a) {
    case 0:
        commit(argv,"[A]:");
        break;
    case 1:
        push(argv);
        break;
    case 2:
        commit(argv,"[D]:");
        break;
    case 3:
        commit(argv,"[M]:");
        break;    
    case 4:
        update(argv,"[A]:");
        break;
    case 5:
        update(argv,"[M]:");
        break;
    case 6:
        update(argv,"[D]:");
        break;
    case 7:
        createCon(argv);
        break;
    }
    return 0;
}
