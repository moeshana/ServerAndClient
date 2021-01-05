#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>

#include "networkr.h"
#include "readfiler.h"

#define BUFF_SIZE 1024


//get a msg to send server
//arg : int command indicate which msg is needed
//      char *args[] file name or other needed info might like version
//      arrlen : how many items in args
//return a string(char*) which is msg for sending to server
//command : 0 => create #CT#PROJECT_NAME
//          1 => checkout #CK#PROJECT_NAME
//          2 => update #UD#PROJECT_NAME#VERSION
//          3 => upgrade 
//      #UG#PROJECT_NAME#NUMBER_FILENAME#PRJECT/.Update#NUM_CON#CONTENT
//          4 =>commit #CM#PROJECT_NAME#VERSION
//          5 =>push 
//              #PS#PROJECT_NAME#NUMBER_FILE
//              #NUMBER_FILENAME#PROJECT/.Commit#NUM_CON#CONTENT
//              #NUMBER_FILENAME#OTHER_FILE#NUM_CON#CONTENT            
//              ....................................
//          6 => destory #DS#PROJECT_NAME
//          7 => currentversion #CV#PROJECT_NAME
//          8 => rollback #RB#PROJECT_NAME#VERSION
//          9 => history #HS#PROJECT_NAME
char *composMsgClientToServer(int command, char* args[], int arrLen) {
    int len;
    char *msg;
    switch(command) {
    case 0:
        len = 5 + strlen(args[0]) + 6;
        msg = (char *)malloc(sizeof(char) * len);
        memset(msg,'\0',len);
        msg[0] = '#';
        msg[1] = 'C';
        msg[2] = 'T';
        msg[3] = '#';
        memcpy(&msg[4], args[0], strlen(args[0]));
        memcpy(&msg[len - 7], "#DONE#", 6);
        break;
    case 1:
        len = 5 + strlen(args[0]) + 6;
        msg = (char *)malloc(sizeof(char) * len);
        memset(msg, '\0', len);
        msg[0] = '#';
        msg[1] = 'C';
        msg[2] = 'K';
        msg[3] = '#';
        memcpy(&msg[4], args[0], strlen(args[0]));
        memcpy(&msg[len - 7], "#DONE#", 6);
        break;
    case 2:; // don't remove ';' here
        char *version = getversion(args[0]);
        len = 5 + strlen(args[0]) + (strlen(version)) + 7;
        msg = (char *)malloc(sizeof(char) * len);
        memset(msg, '\0', len);
        memcpy(msg, "#UD#", 4);
        memcpy(&msg[4],args[0],strlen(args[0]));
        msg[4 + strlen(args[0])] = '#';
        memcpy(&msg[5 + strlen(args[0])], version, strlen(version));
        memcpy(&msg[5+strlen(version)+strlen(args[0])],"#DONE#",6);
        msg[len-1] = '\0';
        free(version);
        break;
    case 3:;//upgrade command, 0=>project, 1=>updatefile
        int len = strlen(args[0]);
        char *lenInArray = myitoa(len);
        char *i1 = mystrcat("#UG#",lenInArray);
        char *i1_5 = mystrcat(i1,"#");
        char *i2 = mystrcat(i1_5, args[0]);
        char *i2_5 = mystrcat(i2, "#");
        int fileCount = countFile(args[1]);
        char *filecInArray = myitoa(fileCount);
        char *i4 = mystrcat(i2_5,filecInArray);
        char *i4_5 = mystrcat(i4,"#");
        free(i1);
        free(i1_5);
        free(i2);
        free(i2_5);
        free(i4);
        free(lenInArray);
        free(filecInArray);
        int fd = open(args[1],O_RDONLY);
        char *content = readFile(fd,fileCount);
        char *i5 = mystrcat(i4_5,content);
        free(i4_5);
        free(content);
        msg = mystrcat(i5,"#DONE#");
        free(i5);
        break;
    case 4:;
        char *version2 = getversion(args[0]);
        len = 5 + strlen(args[0]) + (strlen(version2)) + 7;
        msg = (char *)malloc(sizeof(char) * len);
        memset(msg, '\0', len);
        memcpy(msg, "#CM#", 4);
        memcpy(&msg[4],args[0],strlen(args[0]));
        msg[4 + strlen(args[0])] = '#';
        memcpy(&msg[5 + strlen(args[0])], version2, strlen(version2));
        memcpy(&msg[5+strlen(version2)+strlen(args[0])],"#DONE#",6);
        msg[len-1] = '\0';
        free(version2);
        break;
    case 5:;
        char *head = mystrcat("#PS#", args[0]);
        char *head2 = mystrcat(head, "#");
        free(head);
        char *commitfile = combinePath(args[0],".Commit");
        int commitfd = open(commitfile, O_RDONLY);
        Manifest *commit = convertFileToList(commitfd);
        char *m1_5 = setMsgFromFile(commitfile);
        char *m2 = mystrcat("#",m1_5);//#commit format
        free(commitfile);
        free(m1_5);
        int s;
        Node *cur = commit->head;
        char *cmsg = (char *) malloc(sizeof(char) * (strlen(m2) + 1));
        memset(cmsg, '\0', strlen(m2) + 1);
        memcpy(cmsg, m2, strlen(m2));
        //m2 = #commitnum#content = cmsg
        int firsttime = 0;
        int comoldsize = commit->size;
        for (s = 0; s < comoldsize; s++) {
            if (!strcmp(cur->ver, "[D]")) {
                commit->size--;
                cur = cur->next;
                continue;
            }     
            char *tmp = setMsgFromFile(cur->f);
            if (!firsttime) {
                free(cmsg);
                firsttime = 1;
            }
            cmsg = mystrcat(m2, tmp);
            free(tmp);
            free(m2);
            m2 = cmsg;
            cur = cur->next;
        }
        char *filenumber = myitoa(commit->size);
        char *m1 = mystrcat(head2, filenumber);
        char *fmsg = mystrcat(m1, cmsg); 
        free(filenumber);
        free(cmsg);
        free(m1);
        msg = mystrcat(fmsg, "#DONE#");
        free(fmsg);
        close(commitfd);
        free(head2);
        //printf("Msg : %s\n", msg);
        freeManifest(commit);
        break;
    case 6:
        len = 5 + strlen(args[0]) + 6;
        msg = (char *)malloc(sizeof(char) * len);
        memset(msg,'\0',len);
        memcpy(msg, "#DS#", 4);
        memcpy(&msg[4], args[0], strlen(args[0]));
        memcpy(&msg[len - 7], "#DONE#", 6); 
        break;
    case 7:
        len = 5 + strlen(args[0]) + 6;
        msg = (char *)malloc(sizeof(char) * len);
        memset(msg,'\0',len);
        memcpy(msg, "#CV#", 4);
        memcpy(&msg[4], args[0], strlen(args[0]));
        memcpy(&msg[len - 7], "#DONE#", 6); 
        break;
    case 8:
        len = strlen(args[0]) + strlen(args[1]) + 12;
        msg = (char *)malloc(sizeof(char) * len);
        memset(msg,'\0',len);
        memcpy(msg, "#RB#", 4);
        memcpy(&msg[4], args[0], strlen(args[0]));
        msg[4+strlen(args[0])] = '#';
        memcpy(&msg[5+strlen(args[0])],args[1], strlen(args[1]));
        memcpy(&msg[len - 7], "#DONE#", 6); 
        break;    
    case 9:
        len = 5 + strlen(args[0]) + 6;
        msg = (char *)malloc(sizeof(char) * len);
        memset(msg,'\0',len);
        memcpy(msg, "#HS#", 4);
        memcpy(&msg[4], args[0], strlen(args[0]));
        memcpy(&msg[len - 7], "#DONE#", 6); 
        break;
    } 
    return msg;
}
/*
int main(int argc, char*argv[]) {
    char *arg[1];
    arg[0] = "eugenie";
    printf("%s\n",composMsgClientToServer(0,arg,1));
}
*/

/*
get ip or port form .Configure file
int : 1 read for ip; 0 read for port
return : string(char *) ip/port
*/
char* readIporHost(int isGetIp) {
    int fd = open("./.Configure",O_RDONLY);
    if (fd < 0) {
        printf("[x]Read configure file failed...\n");
        return NULL;
    }
    char *res = (char *)malloc(sizeof(char) * 128);
    char buffer[128];
    int count = 0;
    int readIn = 0;
    int flag = -1;
    do {
        flag = read(fd, buffer + readIn, 128 - readIn);
        readIn += flag;
    } while (readIn < 127 && flag > 0);
    int i;
    for (i = 0; i < readIn; i++) {
        if (buffer[i] == ':') {
            if (isGetIp) {
                break;
            } else {
                isGetIp = 1; 
                continue;
            }
        }
        if (isGetIp) {
            res[count] = buffer[i];
            count++;
        }
    }
    res[count] = '\0';
    close(fd);
    return res;
}


/*
set up client side and connect to server
*/
int initClient(char* serverIp, int port) {
    int length;
    struct sockaddr_in serverAddr;
    int sin_size = sizeof(struct sockaddr_in);
    char buf[BUFF_SIZE];
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if ((*clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[x]Socket create failed\n");
        return -1;
    }
    if ((connect(*clientSocket, (struct sockaddr*)&serverAddr,
            sizeof(struct sockaddr))) < 0) {
        printf("[x]Connect server failed\n");
        return -1;
    } 

    printf("[*]Connect to the server.\n");
    length = recv(*clientSocket, buf, BUFF_SIZE, 0);
    buf[length] = 0;
    printf("[!]Get a message from server : %s\n",buf);
    return 1;
}

//close client socket when get a ctrl + c
void signalHandler(int signum) {
    printf("[!]Stop program!\n");
    close(*clientSocket);
    free(clientSocket);
    printf("[!]Good Bye.\n");
    exit(0);
}

/*
int main(int argc, char * argv[]) {
    signal(SIGINT, signalHandler);
    clientSocket = (int *) malloc(sizeof(int));
    char *ip = readIporHost(1);
    char *port = readIporHost(0);
    if (ip == NULL || port == NULL) {
        printf("[x]Read configure file failed again..\n");
        return -1;
    }
    if(initClient(ip, atoi(port))) {
        clientBehavior();
    };
    return 0;
}
*/



