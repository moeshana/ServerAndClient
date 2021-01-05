#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "networkr.h"
#include "readfiler.h"


int initialRollBackClient(char *project, char *version) {
    char *args[2];
    args[0] = project;
    args[1] = version;
    char *msg = composMsgClientToServer(8, args, 2); 
    //printf("msg is %s\n", msg);
    
    clientSocket = (int *) malloc(sizeof(int));
    char *ip = readIporHost(1);
    char *port = readIporHost(0);
    if (ip == NULL || port == NULL) {
        printf("[x]Read configure file failed again..\n");
        return -1; 
    }   
    if(initClient(ip, atoi(port))) {
        if (clientRollBackBehavior(msg, project) > 0)  {
        //do soething else here might 
        }    
    }   
    free(msg);
    free(ip);
    free(port);
    return 1;
}

int clientRollBackBehavior(char *msg, char *projectNameFromInput) {
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
    //printf("buf : %s\n", buf);
    if (startwith(buf,"#ERR_2#")) {
        printf("[X]No thie project on the server...\n");
    } else {
        if (startwith(buf,"#ERR_8#")) {
            printf("[X]There are no this version on the server...\n");
        } else {
            if (startwith(buf, "#CKR#")) {
                res = 1;
                //char *command=mystrcat("rm -rf ./",projectNameFromInput);
                //system(command);
                removeDir(projectNameFromInput);
                whichMsg(buf,projectNameFromInput);
                //free(command);
                printf("[!]success Rollback\n");
            } 
        } 
    }
    free(buf);
    close(*clientSocket);
    free(clientSocket);
    return res;


}



