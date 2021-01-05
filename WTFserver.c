#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#include "networkr.h"
#include "readfiler.h"

#define BUFF_SIZE 10
#define SERVER_PORT 22797
#define SERVER_IP_ADDRESS "localhost"


threadpool *tp;

/*
server function for sub-threading
*/
void *serverFunction(void *args) {
    threadnode *mytnode = (threadnode *) args;
    int myclient = mytnode->cs;
    //int myclient = (int)*((int *)(args));
    pthread_mutex_unlock(&mutex_clientsocket);

    printf("Threading : %d\n",myclient);
    if ((send(myclient, "Hello\0", 6, 0)) < 0) {
        printf("[!]Send msg error\n");
    }
    //make sure get all msg.
    int size = 10;
    int count = 0;
    int length = 0;
    char *buf = (char *)malloc(sizeof(char) * BUFF_SIZE);
    memset(buf, '\0', BUFF_SIZE);
    while (1) {
        length = recv(myclient, &buf[count], 10, 0); 
        count += length;
        size += 10; 
        char *new = (char *)malloc(sizeof(char) * size);
        memset(new, '\0', size);
        memcpy(new, buf, size - 10);
        free(buf);
        buf = new;    
        if (strstr(buf, "#DONE#") != NULL) {
            buf[strlen(buf) - 6] = '\0';
            break;
        }
    }
    printf("Get msg : %s\n", buf);
    char *response = serverSideWhichMsg(buf);
    // may get a null response
    if ((send(myclient,response,strlen(response), 0)) < 0) {
        printf("[!]Send msg error\n");
    }
    if ((send(myclient, "#DONE#", 6, 0)) < 0) {
        printf("[!]Send msg error\n");
    }
    free(response);
    close(myclient);
    mytnode -> isDone = 1;
}


/*
set up server
and let server waitting for client
once a new client connect to the server,
add client info into threadpool
*/
int initalServer() {
    /*
    char *ip = readIporHost(1);
    char *port = readIporHost(0);
    if (ip == NULL || port == NULL) {
        printf("[x]Read configure file failed again..\n");
        return -1; 
    }   
    */
    int clienttemp;
    int clientSocket;
    
    struct sockaddr_in myAddr, otherAddr;
    int sin_size = sizeof(struct sockaddr_in);
    //struct hostent *result = gethostbyname(ip);
    struct hostent *result = gethostbyname(SERVER_IP_ADDRESS);
    memset(&myAddr, 0, sizeof(myAddr));
    bcopy((char *)result->h_name, 
          ((char *)&myAddr.sin_addr.s_addr),
          result->h_length);
    
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(SERVER_PORT);
    //myAddr.sin_port = htons(atoi(port));

    if ((*serverSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[x]Socket create failed\n");
        return -1;
    } 
    if ((bind(*serverSocket, (struct sockaddr*)&myAddr,
            sizeof(struct sockaddr))) < 0) {
        printf("[x]Bind address failed\n");
        return -1;
    } 
    printf("[*]Server is running on %s : %d\n", 
            result->h_name, SERVER_PORT);
    listen(*serverSocket, 8);
    printf("[*]Server is listening...\n");

    if (access("./.BackupLib", F_OK) == -1) {
        printf("[*]Setup Backup Lib...\n");
        system("mkdir ./.BackupLib");
    }

    //free(ip);
    //free(port);

    tp = (threadpool *)malloc(sizeof(threadpool));  
    tp->size = 0;
    tp->head = NULL;

    while(1) {
        if ((clientSocket = accept(*serverSocket, 
                (struct sockaddr*)&otherAddr, &sin_size)) < 0) {
            printf("[x]Accept client failed\n");
            return -1;
        } 
    
        printf("[+]One client connected to the server.\n");
        
        pthread_mutex_lock(&mutex_clientsocket);
        clienttemp = clientSocket;
        pthread_t mythread;
        //pthread_create(&mythread, NULL,
        //            serverFunction, (void *)&clienttemp);
        threadnode *tn = (threadnode *)malloc(sizeof(threadnode));
        tn->cs = clienttemp;
        tn->th = &mythread;
        tn->isDone = 0;
        tn->next = NULL;
        addThread(tp, tn);
        pthread_create(&mythread, NULL,
                    serverFunction, (void *)tn);
    }
    close(clientSocket);
    close(*serverSocket);
    free(serverSocket);
    return 1;    
}

/*
close everything when we close the server(ctrl + c)
*/
void serverSignalHandler(int signum) {
    printf("[!]Stop program!\n");
    pthread_mutex_destroy(&mutex_clientsocket);
    pthread_mutex_destroy(&mutex0);
    pthread_mutex_destroy(&mutex_rollback_1);
    pthread_mutex_destroy(&mutex_destroy_2);
    pthread_mutex_destroy(&mutex_his_1);
    pthread_mutex_destroy(&mutex_mani_2);
    
    threadnode *cur = tp->head;
    int o;
    for (o = 0; o < tp->size; o++) {
        if (cur ->isDone == 0) {
            pthread_cancel(*cur->th);
            shutdown(cur->cs, SHUT_RDWR);
            close(cur->cs);
            printf("[-] Closed one client-thread(%d)\n", cur->cs);
        }
        cur = cur->next;
    }
    
    shutdown(*serverSocket, SHUT_RDWR);
    printf("close serverSocket : %d\n", close(*serverSocket));
    free(serverSocket);
    if (fd1 != NULL) {
        close(*fd1);
    }
    printf("[!]Good Bye.\n");
    exit(0);
}

/*
int main(int argc, char * argv[]) {
    signal(SIGINT, serverSignalHandler);
    serverSocket = (int *) malloc(sizeof(int));
    clientSocket = (int *) malloc(sizeof(int));
    initalServer();
    return 0;
}


*/


