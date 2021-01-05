#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "networkr.h"
#include "readfiler.h"

#define BUFF_SIZE 5

int initialCreateClient(char *projectName) {
    char *args[1];
    args[0] = projectName;
    char * msg = composMsgClientToServer(0, args, 1);
    //printf("msg is : %s\n", msg);
    clientSocket = (int *) malloc(sizeof(int));
    char *ip = readIporHost(1);
    char *port = readIporHost(0);
    if (ip == NULL || port == NULL) {
        printf("[x]Read configure file failed again..\n");
        return -1; 
    }   
    if(initClient(ip, atoi(port))) {
        clientCreateBehavior(msg);
    }
    free(msg);
    free(ip);
    free(port);
    return 1;
} 

int clientCreateBehavior(char *msg) {
    int length;
    char *buf = (char *) malloc(sizeof(char) * BUFF_SIZE);
    memset(buf, '\0', BUFF_SIZE);
    int size = 10;
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
    if (startwith(buf,"#ERR_1#")) {
        printf("[X]Project already existed.\n");
    } else {
        msg[strlen(msg) - 6] = '\0';
        mkdir(&msg[4],00755);
        //printf("msg is: %s\n", buf);
        char *project = combinePath("./", &msg[4]);
        whichMsg(buf, project);
        free(project);
    }
    free(buf);
    close(*clientSocket);
    free(clientSocket);
    return 1;
}


/*
int main(int argc, char * argv[])
{

    return 0;
}
*/



