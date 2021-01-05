#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "networkr.h"
#include "readfiler.h"


Manifest *scanDirCreateManifestList(char *path, Manifest *res) {
    struct dirent *de;
    DIR *dir = opendir(path);
    Node *current = (Node *)malloc(sizeof(Node));
    while ((de = readdir(dir)) != NULL) {
        if (!(strcmp(de->d_name, ".") && strcmp(de->d_name, ".."))) {
            continue;
        }
        if (de->d_type == DT_DIR) {
            char *nd = combinePath(path, de->d_name);
            //DIR *d = opendir(nd);
            scanDirCreateManifestList(nd, res);
            //closedir(d);
            free(nd);
        } else {
            if (!(strcmp(de->d_name, ".Manifest") &&
                  strcmp(de->d_name, ".Conflict") &&
                  strcmp(de->d_name, ".Update") &&
                  strcmp(de->d_name, ".Commit") &&
                  strcmp(de->d_name, ".History"))) {
                continue;
            } 
            char *filename = combinePath(path, de->d_name);
            int fd = open(filename, O_RDONLY);
            current->f = filename;
            unsigned int live = readFileThenHash(fd);
            close(fd);
            current->hash = myitoa(live);
            char *ver = (char *)malloc(sizeof(char) * 2);
            ver[0] = '0';
            ver[1] = '\0';
            current->ver = ver;
            addItem(res,current);
            current = (Node *)malloc(sizeof(Node));
        }
    }
    free(current);
    closedir(dir);
    return res;
}

/**
int main(int argc, char * argv[])
{
    Manifest *res = (Manifest *)malloc(sizeof(Manifest));
    scanDirCreateManifestList("./Eugenie", res);
    Node *c = res->head;
    printf("res size is : %d\n", res->size);
    for (int i = 0; i < res->size; i++) {
        printf("f : %s;  ", c->f);
        printf("hash : %s\n", c->hash);
        printf("ver : %s\n", c->ver);
        c = c->next;
    }
    freeManifest(res);
    return 0;
}
*/


int generateCommit(char *project) {
    //printf("GET HERE %s\n", project);
    char *manipath = combinePath(project, ".Manifest");
    int fdm = open(manipath, O_RDONLY);
    Manifest *mani = convertFileToList(fdm);
    close(fdm);
    free(manipath);
    //printf("GET HERE 2%s\n", project);
    
    char *scanpath = combinePath(".", project);
    Manifest *res = (Manifest *)malloc(sizeof(Manifest));
    memset(res, '\0',sizeof(Manifest));
    res->size = 0;
    scanDirCreateManifestList(scanpath, res);
    free(scanpath);
    
    //<=============================
    /*printf("==============================================\n");
    Node *c = res->head;
    printf("res size is : %d\n", res->size);
    for (int i = 0; i < res->size; i++) {
        printf("f : %s;  ", c->f);
        printf("hash : %s\n", c->hash);
        printf("ver : %s\n", c->ver);
        c = c->next;
    }
    c = mani->head;
    printf("mani size is : %d\n", mani->size);
    for (int i = 0; i < mani->size; i++) {
        printf("f : %s;  ", c->f);
        printf("hash : %s\n", c->hash);
        printf("ver : %s\n", c->ver);
        c = c->next;
    }
    printf("==============================================\n");
    //=============================>
    */
    //printf("GET HERE 3%s\n", project);
    char *file = combinePath(project, ".Commit");
    int fd = open(file,
            O_WRONLY|O_CREAT|O_TRUNC,
            S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    free(file);
    write(fd, "0\n", 2);
    int i;
    Node *current = mani->head;
    //printf("GET HERE 3%s\n", project);

    for (i = 0; i < mani->size; i++) {
        Node *nodeInres = findFile(res, current->f);
        //printf("GET HERE %s\n", current->f);
        if (nodeInres == NULL) {
            printf("[D]:%s\n",current->f);        
            write(fd,"[D]:",4);
            write(fd,current->f,strlen(current->f));
            write(fd,":0\n",3);
        
        } else {
            //printf("%s(%s) : %s(%s)\n", nodeInres->f,nodeInres->hash
            //,current->f,current->hash);
            if (strcmp(nodeInres->hash, current->hash)) {
                printf("[M]:%s\n", current->f);
                write(fd,"[M]:",4);
                write(fd,current->f,strlen(current->f));
                write(fd,":",1);
                write(fd,nodeInres->hash,strlen(nodeInres->hash));
                write(fd,"\n",1);
            }
        }
        current = current->next;
    }
    //printf("GET HERE 444%s\n", project);
    current = res->head;
    for (i = 0; i < res->size; i++) {
        Node *nodeInres = findFile(mani, current->f);
        if (nodeInres == NULL) {
            printf("[A]:%s\n",current->f);        
            write(fd,"[A]:",4);
            write(fd,current->f,strlen(current->f));
            write(fd,":",1);
            write(fd,current->hash,strlen(current->hash));
            write(fd,"\n",1);
        }
        current = current->next;
    }

    //printf("GET HERE 4%s\n", project);
    close(fd);
    freeManifest(res);
    freeManifest(mani);
    return 1;
}

int initialCommitClient(char *project) {
    char *args[1];
    args[0] = project;
    char *msg = composMsgClientToServer(4, args, 1); 
    //printf("msg is %s\n", msg);
    
    clientSocket = (int *) malloc(sizeof(int));
    char *ip = readIporHost(1);
    char *port = readIporHost(0);
    if (ip == NULL || port == NULL) {
        printf("[x]Read configure file failed again..\n");
        return -1; 
    }   
    if(initClient(ip, atoi(port))) {
        if (clientCommitBehavior(msg, project) > 0)  {
        //do soething else here might 
            if (generateCommit(project) < 0) {
                free(msg);
                free(ip);
                free(port);
                return -1;
            } else {
                //printf("[!]Nothing changed...\n");
            }
        }    
    }   
    free(msg);
    free(ip);
    free(port);
    return 1;
}

int clientCommitBehavior(char *msg, char *projectNameFromInput) {
    int res = -1;
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
        printf("[X]No thie project on the server...\n");
    } else {
        if (startwith(buf,"#ERR_4#")) {
            printf("[!]You might need to update your project first...\n");
        } else {
            if (startwith(buf, "#ERR_3#")) {
                res = 1;
            } else {
                printf("[X]Can't find Manifest file on the server...\n"); 
            }
        } 
    }
    free(buf);
    close(*clientSocket);
    free(clientSocket);
    return res;


}



