#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"
#include "networkr.h"

int initialUpgradeClient(char *project, char *updatefile) {
    char *args[2];
    args[0] = project;
    args[1] = updatefile;
    char *msg = composMsgClientToServer(3, args, 2);
    clientSocket = (int *) malloc(sizeof(int));
    char *ip = readIporHost(1);
    char *port = readIporHost(0);
    if (ip == NULL || port == NULL) {
        printf("[x]Read configure file failed again..\n");
        return -1; 
    }   
    if(initClient(ip, atoi(port))) {
        clientUpgradeBehavior(msg);
    }
    free(msg);
    free(ip);
    free(port);
    
    return 1;
}

int clientUpgradeBehavior(char *msg) {
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
    } else {
        msg[strlen(msg) - 6] = '\0';
        char *project = combinePath(".", &msg[4]);
        whichMsg(buf, project);
        free(project);
    }
    free(buf);
    close(*clientSocket);
    free(clientSocket);
    return 1;
}

int removeDT(char *project) {
    char *file = combinePath(project, ".Update");
    int fd = open(file, O_RDONLY);
    Manifest *list = convertFileToList(fd);
    int i;
    Node* current = list->head;
    for (i = 0; i < list->size; i++) {
        if (!strcmp(current->ver,"[D]")) {
            //printf("remove here[%s]\n", current->f);
            removeFile("./", current->f);
        }
        current = current->next;
    }
    free(file);
    free(list);
    close(fd);
}
/**
int main(int argc, char * argv[])
{

    return 0;
}
*/



