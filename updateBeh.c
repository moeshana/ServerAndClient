#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "networkr.h"
#include "readfiler.h"

int initialUpdateClient(char *project) {
    char *args[1];
    args[0] = project;
    char *msg = composMsgClientToServer(2, args, 1);
    //printf("msg is %s\n", msg);
    clientSocket = (int *) malloc(sizeof(int));
    char *ip = readIporHost(1);
    char *port = readIporHost(0);
    if (ip == NULL || port == NULL) {
        printf("[x]Read configure file failed again..\n");
        return -1;
    }
    if(initClient(ip, atoi(port))) {
        if (clientUpdateBehavior(msg, project) > 0)  {
            createUpdateFile(project);
        }        
    }
    free(msg);
    free(ip);
    free(port);
    return 1;    
}

int clientUpdateBehavior(char *msg, char *projectNameFromInput) {
    int res = 1;
    int length;
    int size = 10;
    char *buf = (char *) malloc(sizeof(char) * size);
    memset(buf, '\0', size);
    int count = 0;
    length = send(*clientSocket, msg, strlen(msg),0);
    while (1) {
    //read file
        length = recv(*clientSocket, &buf[count], 10, 0);
        count += length;
        size += 10;
        char *new = (char *)malloc(sizeof(char) * size);
        memset(new, '\0', size);
        memcpy(new, buf, size - 10);
        free(buf);
        buf = new;
        if (strstr(buf, "#DONE#") != NULL) {
            buf[strlen(buf) - 6] = '\0';
            printf("[!]Receive info from the server.\n");
            break;
        }
    }
    if (startwith(buf,"#ERR_2#")) {
        printf("[X]No this project on the server...\n");
        res = -1;
    } else {
        if (startwith(buf,"#ERR_3#")) {
            printf("[!]Your project is up-to-date...\n");
            int i;
            for (i = strlen(msg) - 7; i > 0; i--) {
                if (msg[i] == '#') {
                    msg[i] = '\0';
                }
            }
            removeFile(&msg[4],".Update");
            removeFile(&msg[4],".Conflict");
            res = -1;
        } else {
            msg[strlen(msg) - 6] = '\0';
            char *project = combinePath(".", &msg[4]);    
            whichMsg(buf, projectNameFromInput);
            free(project);
        }
    }
    free(buf);
    close(*clientSocket);
    free(clientSocket);
    return res;
}


char *getversion(char *project) {
    char *res;
    char *fileName = combinePath(project, ".Manifest");
    int fd = open(fileName, O_RDONLY);
    free(fileName);
    if (fd == -1) {
        res = (char *) malloc(sizeof(char) * 2);
        res[0] = '0';
        res[1] = '\0';
        return res;
    }
    char buf[1024];
    int readIn = 0;
    int flag = -1;
    do {
        flag = read(fd, buf + readIn, 1);
        readIn++;    
    } while (buf[readIn - 1] != '\n' && flag > 0 );
    buf[readIn - 1] = '\0';
    if (strlen(buf) == 0) {
        buf[0] = '0';
        buf[1] = '\0';
    }
    res = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
    memset(res, '\0', strlen(buf) + 1);
    memcpy(res,buf,strlen(buf));
    return res;
}


int createUpdateFile(char *project) {
    printf("[*]Project name : %s\n", project);
    char *myman = combinePath(project, ".Manifest");
    char *seman = combinePath(project, ".ManifestBackup");
    int myfd = open(myman, O_RDONLY);
    int sefd = open(seman, O_RDONLY);
    free(myman);
    Manifest *mine = convertFileToList(myfd);
    Manifest *sers = convertFileToList(sefd);
    
    close(myfd);
    close(sefd);
    char *filename = combinePath(project, ".Update");
    int fd = open(filename,
                  O_WRONLY|O_CREAT|O_TRUNC,
                  S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    write(fd,"0",1);
    write(fd,"\n",1); 
    char *filenamec = combinePath(project, ".Conflict");
    int fdc = open(filenamec,
                  O_WRONLY|O_CREAT|O_TRUNC,
                  S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    write(fdc,"0",1);
    write(fdc,"\n",1);
    free(filename);
    free(filenamec);

    int i;
    Node* sc = sers->head;
    for (i = 0; i < sers->size; i++) {
        Node *res = findFile(mine, sc->f);
        if (res != NULL) {
            int liveHash = open(sc->f, O_RDONLY);
            unsigned int live = readFileThenHash(liveHash);
            unsigned int clientHash = atoi(res->hash);
            close(liveHash);
            if (strcmp(sc->hash,res->hash) && (live == clientHash)) {
                char info[] = "[M]:";
                char *i2 = mystrcat(info, sc->f);
                char *i2_5 = mystrcat(i2, ":");
                char *i3 = mystrcat(i2_5, sc->hash);
                free(i2);
                free(i2_5);
                printf("%s\n", i3);   
                write(fd, i3, strlen(i3));
                write(fd,"\n",1);
                free(i3);        
            } else {
                if (live != clientHash) {
                    char info[] = "[C]:";
                    char *i2 = mystrcat(info, sc->f);
                    char *i2_5 = mystrcat(i2, ":");
                    char *i3 = mystrcat(i2_5, myitoa(live));
                    free(i2);
                    free(i2_5);
                    printf("%s\n", i3);   
                    write(fdc, i3, strlen(i3));
                    write(fdc,"\n",1);
                    free(i3);        
                }
            }
        } else {
            char info[] = "[A]:";
            char *i2 = mystrcat(info, sc->f);
            char *i2_5 = mystrcat(i2, ":");
            char *i3 = mystrcat(i2_5, sc->hash);
            free(i2);
            free(i2_5);
            printf("%s\n", i3);   
            write(fd, i3, strlen(i3));
            write(fd,"\n",1);
            free(i3); 
        }
        sc = sc->next;
        
    }
    
    sc = mine->head;
    for (i = 0; i < mine->size; i++) {
        Node *res = findFile(sers, sc->f); 
        if (res == NULL) {
            char info[] = "[D]:";
            char *i2 = mystrcat(info, sc->f);
            char *i2_5 = mystrcat(i2, ":");
            char *i3 = mystrcat(i2_5, sc->hash);
            free(i2);
            free(i2_5);
            printf("%s\n", i3);   
            write(fd, i3, strlen(i3));
            write(fd,"\n",1);
            free(i3); 
        }
        sc = sc->next;
    }
    remove(seman);
    free(seman);
    freeManifest(mine);
    freeManifest(sers);
    close(fd);
    close(fdc);
    return 1;
}







/*
int main(int argc, char * argv[])
{
    createUpdateFile("Eugenie");
    char *res = getversion("shana");
    printf("res is %s\n",res);
    free(res);
    return 0;
}
*/


