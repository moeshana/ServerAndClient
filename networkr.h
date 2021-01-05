#ifndef _NETWORKR_H_
#define _NETWORKR_H_

#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

int *serverSocket;
int *clientSocket;
int *fd1;
pthread_mutex_t mutex_clientsocket;
pthread_mutex_t mutex0;
pthread_mutex_t mutex_rollback_1;
pthread_mutex_t mutex_destroy_2;
pthread_mutex_t mutex_his_1;
pthread_mutex_t mutex_mani_2;


//<=================remove 
typedef struct threadnode {
    int cs;
    pthread_t *th;
    struct threadnode *next;
    int isDone;
} threadnode;

typedef struct threadpool {
    int size;
    struct threadnode *head;
} threadpool;

int addThread(threadpool *t, threadnode *tn);
int removeThread(threadpool *t, threadnode *tn);
int freeEmpty(threadpool *t);

//<=================remove


//in clientmain.c
int configureFunction(char *ipStr, char *portStr);
int getCommand(int argc, char *argv[]);

//in WTFclient
int initClient(char* serverIp, int port);
void signalHandler(int signum);
char* readIporHost(int isGetIp);
char *composMsgClientToServer(int command, char* args[], int len);

//in WTFserver
int initalServer();
void serverSignalHandler(int signum);

//in createBeh.c
int clientCreateBehavior(char *projectName);
int initialCreateClient(char *projectName);

//in whatsMessageFromServer.c
typedef struct fileName {
    int nc; // new current position
    char *fn; //file name
} fileName;
int whichMsg(char *msg, char *path);
fileName *getFileName(char *msg, int current);
int saveToFile(char *fileName, char *content);
int fromMsgUpdateFile(char *msg, char *path);
int saveBackupManifest(char *msg, char *path);

//in serverSideMsg.c
char *serverSideWhichMsg(char *msg);

//in checkoutBeh.c
int initialCheckoutClient(char *project);
int clientCheckoutBehavior(char *msg);

//in updateBeh.c
int initialUpdateClient(char *project);
int clientUpdateBehavior(char *msg, char *projectNameFromInput);
char *getversion(char *project);
int createUpdateFile(char *project);

//in upgradeBeh.c
int initialUpgradeClient(char *project,char *updatefile);
int clientUpgradeBehavior(char *msg);
int removeDT(char *project);

//in cvBeh.c
int initialCurrentVersionClient(char *projectName); 
int clientCurrentVersionBehavior(char *msg);

//in commitBeh.c
int initialCommitClient(char *project);
int clientCommitBehavior(char *msg, char *projectNameFromInput);

//in pushBeh.c
int initialPushClient(char *project);
int clientPushBehavior(char *msg, char *projectFromInput);

//in hBeh.c
int initialHistoryClient(char *projectName);
int clientHistoryBehavior(char *msg);

//in destroyBeh.c
int initialDestroyClient(char *projectName);
int clientDestroyBehavior(char *msg);

//in rollbackBeh.c
int clientRollBackBehavior(char *msg, char *projectNameFromInput);
int initialRollBackClient(char *project, char *version);

//in serverUpdate.c
int serversideUpdateAction(char *msg, char *projectName);
int destroyProject(char *project, int includeLib);
int startRollBack(char *projectName, char *ver);


#endif
