#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"
#include "networkr.h"

int initialHistoryClient(char *projectName) {
    char *args[1];
    args[0] = projectName;
    char * msg = composMsgClientToServer(9, args, 1);
    //printf("msg is : %s\n", msg); 
    clientSocket = (int *) malloc(sizeof(int));
    char *ip = readIporHost(1);
    char *port = readIporHost(0);
    if (ip == NULL || port == NULL) {
        printf("[x]Read configure file failed again..\n");
        return -1;
    }
    if(initClient(ip, atoi(port))) {
        clientHistoryBehavior(msg);
    }
    free(msg);
    free(ip);
    free(port);
    return 1;
}

int clientHistoryBehavior(char *msg) {
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
        printf("[X]No thie project on the server...\n");
        res = -1;
    } else {
        whichMsg(buf,&msg[4]);
        res = -1;
    
    }
    free(buf);
    close(*clientSocket);
    free(clientSocket);
    return res;
}

/*
int main(int argc, char * argv[])
{

    return 0;
}
*/



