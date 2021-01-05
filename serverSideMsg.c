#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "networkr.h"
#include "readfiler.h"

/*
get project name from msg
*/
char *getProjectNameFromMsg(char *msg) {
    char *projectName = (char *)malloc(sizeof(char)*(strlen(msg)-3));
    memset(projectName, '\0', strlen(msg) - 3);
    memcpy(projectName, msg + 4, strlen(msg) - 4);
    int i;
    for (i = 0; i < strlen(projectName); i++) {
        if (projectName[i] == '#') {
            projectName[i] = '\0';
            break;
        }
    }
    return projectName;
}

/*
not exist needed  
1(wanna no this project on the server side)
    => if project existed, return error 1 say project already existed
0(wanna this project on the server side)
    => if project not existed, return error 2 say project not existed
*/
char *checkProjectExist(char *projectName, int notExistNeeded) {
    char *response;
    if (notExistNeeded) {
        if (checkFile("", projectName,"./",1)) {
            char nmsg[] = "#ERR_1#";
            response = (char *)malloc(sizeof(char) * 8);
            memset(response, '\0', 8);
            memcpy(response, nmsg, 7);
            return response;
        } 
    } else {
        if (!checkFile("", projectName,"./",1)) {
            char nmsg[] = "#ERR_2#";
            response = (char *)malloc(sizeof(char) * 8);
            memset(response, '\0', 8);
            memcpy(response, nmsg, 7);
            return response;
        }     
    }
    return NULL;
}


/*
server side whichMsg() method
when server get a new msg, 
use this method to switch and deal different actions
return a string which is server's response
*/
char *serverSideWhichMsg(char *msg) {
    //printf("MSG is : %s\n", msg);
    char *response;
    if (startwith(msg, "#RB#")) {
        printf("rollback...\n");
        char *projectName = getProjectNameFromMsg(msg);
        if (!checkFile("", projectName,"./",1)) {
            response = (char *)malloc(sizeof(char) * 8);
            memset(response, '\0', 8);
            memcpy(response, "#ERR_2#", 7);
            free(projectName);
            return response;
        } else {
            if (!checkFile(&msg[5+strlen(projectName)],projectName,"./.BackupLib", 0)) {
                response = (char *)malloc(sizeof(char) * 8);
                memset(response, '\0', 8);
                memcpy(response, "#ERR_8#", 7);
                free(projectName);
                return response;
            }
            startRollBack(projectName, &msg[5+strlen(projectName)]);            
            char *newmsg = mystrcat("#CK#",projectName);
            response = serverSideWhichMsg(newmsg);
            free(newmsg);
            free(projectName);
            return response; 
        } 
    }
    
    if (startwith(msg, "#DS#")) {
        printf("destroy...\n");    
        char *projectName = getProjectNameFromMsg(msg);
        if (!(checkFile("", projectName,"./",1) && 
              strcmp(projectName, ".BackupLib"))) {
            response = (char *)malloc(8);
            memset(response, '\0', 8);
            memcpy(response, "#ERR_2#", 7);
            free(projectName);
            return response;
        } else {
            if (destroyProject(projectName, 1) > 0) {
                response = (char *)malloc(sizeof(char) * 8);
                memset(response, '\0', 8);
                memcpy(response, "#SUC_1#", 7);
            } else {
                response = (char *)malloc(sizeof(char) * 8);
                memset(response, '\0', 8);
                memcpy(response, "#FAI_1#", 7);   
            }
            free(projectName);
            return response;  
        }    
    }
    if (startwith(msg, "#HS#")) {
        printf("history...\n"); 
        char *projectName = getProjectNameFromMsg(msg);
        if (!checkFile("", projectName,"./",1)) {
            response = (char *)malloc(8);
            memset(response, '\0', 8);
            memcpy(response, "#ERR_2#", 7);
            free(projectName);
            return response;
        } else {
            char *proloc = combinePath("./.BackupLib", projectName);
            char *file = combinePath(proloc, ".History");
            int len = countFile(file);
            int hisfd = open(file, O_RDONLY);
            free(proloc);
            free(file);
            char msg[] = "#CVR#[+]----------Start-----------[+]\n";
            char *content = readFile(hisfd, len); 
            char *cmsg = mystrcat(msg, content);
            response = mystrcat(cmsg,"\n[+]----------Done-----------[+]\n");
            free(content);
            free(cmsg);
            close(hisfd);
            return response;
        }    
    }

    if (startwith(msg,"#PS#")) { 
        printf("push...\n");
        char *projectName = getProjectNameFromMsg(msg);
        if (!checkFile("", projectName,"./",1)) {
            response = (char *)malloc(sizeof(char) * 8);
            memset(response, '\0', 8);
            memcpy(response, "#ERR_2#", 7);
            free(projectName);
            return response; 
        } else { 
            serversideUpdateAction(&msg[5+strlen(projectName)],projectName);
            char *manifestFilePath = combinePath(projectName, ".Manifest");
            char *content = setMsgFromFile(manifestFilePath);
            response = mystrcat("#CKR#1#", content);
            free(manifestFilePath);
            free(content);
            free(projectName);
            return response;  
        }
    }
    if (startwith(msg,"#CM#")) {
        printf("commit...\n");    
        char *projectName = getProjectNameFromMsg(msg);
        if (!checkFile("", projectName,"./",1)) {
            response = (char *)malloc(sizeof(char) * 8);
            memset(response, '\0', 8);
            memcpy(response, "#ERR_2#", 7);
            free(projectName);
            return response;
        } else {
            char *ver = getversion(projectName);
            if (ver == NULL) {
                response = (char *)malloc(sizeof(char) * 8);
                memset(response, '\0', 8);
                memcpy(response, "#ERR_5#", 7);
                free(projectName);
                free(ver);
                return response;
            }
            //printf("compare:%s <=> %s\n",msg+5+strlen(projectName),ver);
            if (!strncmp(&msg[5 + strlen(projectName)],ver,strlen(ver))) {
                response = (char *)malloc(sizeof(char) * 8);
                memset(response, '\0', 8);
                memcpy(response, "#ERR_3#", 7);
                free(projectName);
                free(ver);
                return response;
            } else { 
                response = (char *)malloc(sizeof(char) * 8);
                memset(response, '\0', 8);
                memcpy(response, "#ERR_4#", 7);
                free(projectName);
                free(ver);
            }
        }
        return response;
    
    
    }
    if (startwith(msg, "#CV#")) {
        printf("check current version...\n");
        char *projectName = getProjectNameFromMsg(msg);
        //char *projectName = (char *)malloc(sizeof(char)*(strlen(msg)-3));
        //memset(projectName, '\0', strlen(msg) - 3);
        //memcpy(projectName, msg + 4, strlen(msg) - 4);
        //printf("project name : %s\n", projectName);
        if (!checkFile("", projectName,"./",1)) {
            response = (char *)malloc(8);
            memset(response, '\0', 8);
            memcpy(response, "#ERR_2#", 7);
            free(projectName);
            return response;
        } else {
            printf("stat checking... \n");
            char *file = combinePath(projectName, ".Manifest");
            int fd = open(file, O_RDONLY);
            Manifest *list = convertFileToList(fd);
            char msg[] = "#CVR#[+]----------Start-----------[+]\nCurrent Version:";
            int i;
            char *ver = myitoa(list->mver);
            char *temp = mystrcat(msg,ver);
            char *cmsg = temp;
            free(ver);
            if (list->size != 0) {
                cmsg = mystrcat(temp, "\nContains file[version]: \n\t");
                free(temp);
                Node *current = list->head;
                for (i = 0; i < list->size; i++) {
                    char *i1 = mystrcat(current->f,"[");
                    char *i2 = mystrcat(i1,current->ver);
                    char *i3 = mystrcat(i2, "]\n\t");
                    temp = mystrcat(cmsg, i3);
                    free(cmsg);
                    free(i1);
                    free(i2);
                    free(i3);
                    cmsg = temp;
                    current = current->next;
                }
            }
            response = mystrcat(cmsg,"\n[+]----------Done-----------[+]\n");
            free(cmsg);
            close(fd);
            free(projectName);
            return response;
        }
    }
    if (startwith(msg, "#UG#")) {
        printf("upgrade...\n");
        int current = 0;
        int i;
        char count[20];
        for (i = 4; i < strlen(msg); i++) {
            if (msg[i] == '#') {
                current = i;
                break;
            } else {
                count[current] = msg[i];
                current++;
            }
        }
        current++;
        int prolen = atoi(count);
        char *projectName = (char *)malloc(sizeof(char) * (prolen + 1));
        memset(projectName, '\0', prolen+1);
        memcpy(projectName, &msg[current], prolen);   
        if (!checkFile("", projectName,"./",1)) {
            response = (char *)malloc(sizeof(char) * 8);
            memset(response, '\0', 8);
            memcpy(response, "#ERR_2#", 7);
            free(projectName);
            return response;
        } else {
            Manifest *list = makeManiListFromMsg(&msg[current+prolen]);
            Node *c = list->head;
            int filenumber = 0;
            char *cmsg;
            
            char *temp = (char *) malloc(sizeof(char));
            memset(temp,'\0',1);
            int mi;
            for (mi = 0; mi < list->size; mi++) {
                if (strcmp(c->ver, "[D]")) {
                    char *i = setMsgFromFile(c->f);
                    cmsg = mystrcat(i, temp);
                    free(i);
                    free(temp);
                    temp = cmsg;
                    filenumber++;
                }            
                //printf("find node => fn : %s\n", c->f);
                //printf("find node => ver : %s\n", c->ver);
                //printf("find node => hash : %s\n", c->hash);
                c = c ->next;
            }
            char *mani = combinePath(projectName,".Manifest");
            char *i2 = setMsgFromFile(mani);
            cmsg = mystrcat(i2, temp);
            free(mani);
            free(i2);
            filenumber++;
            char *fileNumber = myitoa(filenumber);
            temp = mystrcat("#UGR#",fileNumber);
            char *temp2 = mystrcat(temp, "#");
            free(temp);
            //char *projectNumber = myitoa(strlen(projectName));
            //char *addpn = mystrcat(temp2,projectNumber);
            //char *addpntp = mystrcat(addpn, "#");
            //char *addp = mystrcat(addpntp, projectName); 
            //free(projectNumber);
            //free(addpntp);
            //free(addpn);
            //char *last = mystrcat(addp, "#");
            response = mystrcat(temp2, cmsg);
            //free(addp);
            free(temp2);
            //free(last);
            free(cmsg);
            free(projectName);
            freeManifest(list);
            //printf("res is : %s\n", response);
            return response;
        }    
    }
    if (startwith(msg, "#UD#")) {
        printf("update...\n");    
        char *projectName = getProjectNameFromMsg(msg);
        if (!checkFile("", projectName,"./",1)) {
            response = (char *)malloc(sizeof(char) * 8);
            memset(response, '\0', 8);
            memcpy(response, "#ERR_2#", 7);
            free(projectName);
            return response;
        } else {
            char *ver = getversion(projectName);
            //printf("compare:%s <=> %s\n",msg+5+strlen(projectName),ver);
            if (!strncmp(&msg[5 + strlen(projectName)],ver,strlen(ver))) {
                response = (char *)malloc(sizeof(char) * 8);
                memset(response, '\0', 8);
                memcpy(response, "#ERR_3#", 7);
                free(projectName);
                free(ver);
                return response;
            } else {
                int current = 5;
                //char *newproject = combinePath(".",projectName);
                //free(projectName);
                //char *mani = combinePath(newproject, ".Manifest");
                char *mani = combinePath(projectName, ".Manifest");
                //free(newproject);
                //free(projectName);
                char *manifest = setMsgFromFile(mani);
                
                //printf("mani : %s\n", manifest);
                
                //free(mani);
                int cur = 0;
                int i;
                for (i = 0; i < strlen(manifest); i++) {
                    if (manifest[i] == '#' && cur < 3) {
                        cur++;
                    }
                    if (manifest[i] == '#' && cur == 3) {
                        cur = i;
                        break;
                    }

                }
                char header[] = "#UDR#";
                response = mystrcat(header, &manifest[i+1]);
                //printf("res : %s\n", response);
                //free(manifest);
            }
            //free(ver);
        }
        return response;
    }
     
    
    if (startwith(msg,"#CT#")) {
        printf("create....\n");

        char *projectName = (char *)malloc(sizeof(char)*(strlen(msg)-3));
        memset(projectName, '\0', strlen(msg) - 3);
        memcpy(projectName, msg + 4, strlen(msg) - 4);
        if (checkFile("", projectName,"./",1)) {
            char nmsg[] = "#ERR_1#";
            response = (char *)malloc(8);
            memset(response, '\0', 8);
            memcpy(response, nmsg, 7);
            return response;
        } else {
            mkdir(projectName, 00755);
            char *file = combinePath(projectName, ".Manifest");
            fd1 = (int *)malloc(sizeof(int));
            memset(fd1, '\0', sizeof(int));
            *fd1 = open(file, 
                    O_WRONLY|O_CREAT|O_TRUNC,
                    S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
            if (*fd1 == -1) {
                printf("[X]Create file failed...");
                return NULL;
            }
            write(*fd1, "0", 1);
            write(*fd1, "\n", 1);
            close(*fd1);

            char *fileback = combinePath("./.BackupLib", projectName);
            if (access(fileback, F_OK) == -1) {
                char *command = 
                    (char *)malloc(sizeof(char)*(strlen(fileback) + 10));
                memset(command, '\0', (strlen(fileback) + 10));
                memcpy(command, "mkdir -p " ,9);
                memcpy(&command[9],fileback, strlen(fileback));
                //printf("%s\n", command);
                system(command);
            }
            char *fileHis = combinePath(fileback, ".History");
            int fhis = open(fileHis, 
                            O_WRONLY|O_CREAT|O_TRUNC,
                            S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);

            close(fhis);
            free(fileback);
            free(fileHis);

            char nmsg[] = "#CTR#1#9#.Manifest#2#0\n#";
            response = (char *)malloc(sizeof(char) * (strlen(nmsg) + 1));
            memset(response, '\0', strlen(nmsg) + 1);
            memcpy(response, nmsg, strlen(nmsg));
            free(file);
            free(projectName);
            return response;
        }
    } 

    if (startwith(msg,"#CK#")) {
        printf("checkout....\n");
        //printf("%s : %ld\n", msg, strlen(msg)); 
        char *projectName = (char *)malloc(sizeof(char)*(strlen(msg)-3));
        memset(projectName, '\0', strlen(msg) - 3); 
        memcpy(projectName, msg + 4, strlen(msg) - 4); 
        if (!checkFile("", projectName,"./",1)) {
            char nmsg[] = "#ERR_2#";
            response = (char *)malloc(sizeof(char) * 8);
            memset(response, '\0',8);
            memcpy(response, nmsg, 7);
            return response;
        } else {
            int current = 5;
            char *newproject = combinePath(".",projectName);
            free(projectName);
            char *mani = combinePath(newproject, ".Manifest");
            free(newproject);
            int fd = open(mani, O_RDONLY);
            Manifest* list = convertFileToList(fd);
            close(fd);
            char *filecount = myitoa(list->size + 1);            
            
            char *manifest = setMsgFromFile(mani);
            int initLen = strlen(manifest) + strlen(filecount) + 6;
            response = (char *)malloc(sizeof(char) * initLen);
            memset(response, '\0', 
                    (strlen(manifest) + strlen(filecount) + 6));
            if (response == NULL){ 
                printf("error\n");
            }
            memcpy(response, "#CKR#", 5);
            memcpy(&response[current], filecount, strlen(filecount));
            response[strlen(filecount) + current] = '\0';
            current += strlen(filecount);
            response[current++] = '#';
            memcpy(&response[current], manifest, strlen(manifest));
            response[strlen(manifest) + current] = '\0';
            //printf("hehe 2 %s\n",response);
            current += strlen(manifest);
            response[current] = '\0';
            free(filecount);
            free(mani);
            free(manifest);
            int m;
            Node *cNode = list->head;
            for (m = 0; m < list->size; m++) {
                // printf("in serverMSG filename : %s\n", cNode->f);
                char *newFile = setMsgFromFile(cNode->f);
                cNode = cNode->next;
                int nflen = strlen(newFile);
                int reslen = strlen(response);
                int total;
                total = nflen + reslen;
                char *temp = (char *)malloc(sizeof(char) * (total + 1));
                memset(temp, '\0', total + 1);
                memcpy(temp, response, reslen);
                temp[reslen] = '\0';
                memcpy(&temp[reslen], newFile, nflen);
                //printf("hehe 4 %s\n",temp);
                temp[total] = '\0';
                temp[total - 1] = '#';
                //printf("total pos: %d\n", temp[total]);
                //printf("=====>\n%s\n<========\n",temp);
                free(response);
                response = temp;
                free(newFile);
            }
            //printf("RE : \n%s\n", response);
            return response;
        }
    }
    return NULL;
}


/*
int main(int argc, char * argv[])
{
    serverSideWhichMsg("#CK#Eugenie");
    return 0;
}
*/



