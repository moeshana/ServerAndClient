#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"
#include "networkr.h"

/*
write char into a file
fileName : the path of the file(path/filename)
content : what info we wanna write into file
return : 1 successful, otherwise -1
*/
int saveToFile(char *fileName, char *content) {
    printf("[+]Start update file...[");
    FilePath *fp = spiltFilePath(fileName);
    //printf("fp : %s\n", fp->fp);
    printf("%s]\n", fp->fn);

    if (access(fp->fp, F_OK) == -1) {
        char *command = (char *)malloc(sizeof(char)*(strlen(fp->fp) + 10));
        memset(command, '\0', strlen(fp->fp) + 10);
        memcpy(command, "mkdir -p " ,9);
        memcpy(&command[9],fp->fp, strlen(fp->fp));
        printf("[+]create new dirtectory : %s\n", command);
        system(command);
        free(command);
    }    
    int fd = open(fileName,
                  O_WRONLY|O_CREAT|O_TRUNC,
                  S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    if (fd == -1) {
        printf("[X]Error: create file failed in whatmsgfromserver:23.\n");
        return -1;
    }
    //printf("content : %s\n", content);
    write(fd, content, strlen(content));
    close(fd);
    free(fp->fp);
    free(fp->fn);
    free(fp);
    return 1;
}

/*
switch here
when client get a correct msg from server
use this method to go to different functions 
*/
int whichMsg(char *msg, char *path) {
    if (startwith(msg, "#CVR#")) {
        path[strlen(path) - 6] = '\0';
        printf("[*]Project Name : %s\n", path);
        printf("%s",&msg[5]);
        return 1;
    }
    if (startwith(msg, "#CTR#")) {
        fromMsgUpdateFile(msg, path);
        return 1;
    }
    if (startwith(msg, "#CKR#")) {
        fromMsgUpdateFile(msg, path);
        return 1;
    }
    if (startwith(msg, "#UDR#")) {
        if (saveBackupManifest(msg,path) > 0) {
            return 1;
        }
        return -1;
    }
    if (startwith(msg, "#UGR#")) {
        fromMsgUpdateFile(msg, "./");
        int i;
        int start = -1;
        char project[1024];
        for (i = 0; i < strlen(path); i++) {
            if (start >= 0 && path[i] == '#') {
                project[start] = '\0';
                break;
            }
            if (start >= 0) {
                project[start] = path[i];
                start++;
            }          
            if (start < 0 && path[i] == '#') {
                start = 0;
            }
        }
        removeDT(project);
        //printf("project : %s\n",project);
        removeFile(project, ".Update");
        removeFile(project,".Conflict");
        return 1;
    }
}

/*
get a struct fileName form msg
this method use for get file name from a msg which contains many files info
char *msg : msg from server
int current : current pointer in msg
    becasue msg may contains many files, 
    we need current to indicate where we start for the next file  
*/
fileName *getFileName(char *msg, int current) {
    fileName *f = (fileName *)malloc(sizeof(fileName));
    memset(f,'\0',sizeof(fileName));
    char temp[1024];
    memset(temp, '\0', 1024);
    int counter = 0;
    while (msg[current] != '#') {
        temp[counter] = msg[current];
        current++;
        counter++;
    }
    current++;
    counter = atoi(temp);
    char *fileName = (char *)malloc(sizeof(char) * (counter + 1));
    memset(fileName, '\0', counter + 1);
    memcpy(fileName, &msg[current], counter);
    fileName[counter] = '\0';
    //printf("filename : %s(%ld)\n",fileName,strlen(fileName));
    f->fn = fileName;
    f->nc = current + counter + 1;
    return f;
}

/*
save server's .Manifest file to compare it with local .Manifest file
*/
int saveBackupManifest(char *msg, char *path) {
    char *file = combinePath(path,".ManifestBackup");
    //printf("[*]File : %s\n", file);
    int fd = open(file,
                  O_WRONLY|O_CREAT|O_TRUNC,
                  S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    if (fd == -1) {
        printf("[X]Create file failed....here\n");
        return -1;
    }
    write(fd, &msg[5], strlen(msg) - 5);
    //write(fd, &msg[5], strlen(msg) - 6);
    free(file);
    close(fd);
    return 1;
}

/**
int main(int args,char *argv[]) {
    char msg[] = "#UDR#Eugenie so cute~~~#DONE#";
    saveBackupManifest(msg, "./shana");

}
*/

/*
update file from msg
char *msg : msg from server
char *path : where the new updated file is
*/
int fromMsgUpdateFile(char *msg, char *path) {
    int current = 5;
    char temp[1024];
    memset(temp,'\0',1024);
    int fileCurrent = 0;
    while (msg[current] != '#') {
        temp[fileCurrent] = msg[current];
        current++;
        fileCurrent++;
    }
    current++;
    fileCurrent = atoi(temp);
    int y;
    fileName *f;
    for (y = 0; y < fileCurrent; y++) {
        f = getFileName(msg, current);
        char *newFile;
        int ns = 0;

        //printf("fn and path : %s <=> %s\n", f->fn, path);
        if (strstr(f->fn, path) == NULL) {
            newFile = combinePath(path, f->fn);
            ns = 1;
        } else {
            newFile = f->fn;
        }
        //printf("come here=> %d : %s\n", f->nc, f->fn);

        current = f->nc;
        int size = 0;
        char filelen[100];
        memset(filelen, '\0', 100);
        while (msg[current] != '#') {
            filelen[size] = msg[current];
            size++;
            current++;
        }
        current++;
        size = atoi(filelen);
        char *content = (char *) malloc(sizeof(char) * (size + 1));
        memset(content, '\0', size + 1);
        memcpy(content, &msg[current], size);
        content[size] = '\0';
        //printf("content : %s\n", content);
        saveToFile(newFile, content);
        current += size;
        current++;
        if (ns) {
            free(newFile);
        }
        free(content);
        free(f->fn);
        free(f);
    }
}

/**
int main(int argc, char * argv[]) {
    whichMsg("#CTR#3#9#.Manifest#29#Junfeng Zhao and Eugenie Shin#4#TTTT#15#ASDFGHJKLASDFGH#5#JJJJJ#10#1234567897#","./testpro");
    return 0;
}
*/



