#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"
#include "networkr.h"

//main method for server
//WTFserver
int main(int argc, char * argv[]) {
    signal(SIGINT, serverSignalHandler);
    if (pthread_mutex_init(&mutex_clientsocket, NULL) != 0) {
        printf("[X]mutex failed(0)...\n");
        return 1;
    }
    if (pthread_mutex_init(&mutex0, NULL) != 0) {
        printf("[X]mutex failed(1)...\n");
        return 1;
    }
    if (pthread_mutex_init(&mutex_rollback_1, NULL) != 0) {
        printf("[X]mutex failed(2)...\n");
        return 1;
    }
    if (pthread_mutex_init(&mutex_destroy_2, NULL) != 0) {
        printf("[X]mutex failed(3)...\n");
        return 1;
    }
    if (pthread_mutex_init(&mutex_mani_2, NULL) != 0) {
        printf("[X]mutex failed(mani2)...\n");
        return 1;
    }
    if (pthread_mutex_init(&mutex_his_1, NULL) != 0) {
        printf("[X]mutex failed(his1)...\n");
        return 1;
    }

    serverSocket = (int *) malloc(sizeof(int));
    initalServer();
    return 0;
}




