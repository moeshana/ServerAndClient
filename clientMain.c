#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "networkr.h"
#include "readfiler.h"

//action event for configure command
int configureFunction(char *ipStr, char *portStr) {
    int ipLen = strlen(ipStr);
    int portLen = strlen(portStr);
    int len = ipLen + portLen + 2;
    char myStr[len];
    memcpy(myStr, ipStr, ipLen);
    myStr[ipLen] = ':';
    memcpy(myStr + ipLen + 1, portStr, portLen);
    myStr[len - 1] = '\0';
    if (writeSomeStuffIntoFile("./.Configure", myStr, 1) > 0) {
        printf("[*]Successful write IP and PORT into Configure file.\n");
        return 1;
    }
    return 0;
}

//get command from user, and pass args to different client
//we have 2 more command run forever to test multi-threading
//th => test history  run histroy command forever
//tc => test current version run currentversion command forever
int getCommand(int argc, char *argv[]) {
    if (argc >= 3) {
        if (argv[2][strlen(argv[2]) - 1 ] == '/') {
            argv[2][strlen(argv[2]) - 1] = '\0';
        }
    }
    if (argc <= 1) {
        printf("[X]Missing args\n");
        return -1;
    }
    if (!strcmp(argv[1], "tc")) {
        char *argv[3] = {"./WTF","currentversion", "shana"};
        //save this space for test as well
        while (1) {
            getCommand(3, argv);
            sleep(1);
        }
    }

    if (!strcmp(argv[1], "th")) {
        //save this space for test as well
        char *argv[3] = {"./WTF","history", "shana"};
        while (1) {
            getCommand(3, argv);
            sleep(1);
        }
    }
    if (!strcmp(argv[1], "configure")) {
        printf("[!]Start configure...\n");
        if (argc < 4) {
            printf("[X]Missing args\nUsage: ./wtf configure IP PORT.\n");
            return -1;
        }
        if (!configureFunction(argv[2], argv[3])) { 
            printf("[X]Something wrong when I try to create a new file.\n");
            return -1;
        }
        return 1;
    }
    if (!strcmp(argv[1], "checkout")) {
        printf("[!]Start checkout...\n");
        if (argc < 3) {
            printf("[X]Missing args\nUsage: ./wtf checkout project_name.\n");
            return -1;
        }
        if (checkFile("",argv[2],"./",1) > 0) {
            printf("[X]Project already existed on the client...\n");
            return -1;
        }
        if (initialCheckoutClient(argv[2]) > 0) {
            printf("[!]Finished!\n");
        } else {
            printf("[X]Something wrong...\n");
            return -1;
        }
        return 1;
    }
    if (!strcmp(argv[1], "update")) {
        printf("[!]Start update...\n");
        if (argc < 3) {
            printf("[X]Missing args\nUsage: ./wtf update project_name.\n");
            return -1;
        }
        if (checkFile("",argv[2],"./",1) == 0) {
            printf("[X]Project doesn't existed on the client...\n[X]Use checkout command first...\n");
            return -1;
        }
        if (initialUpdateClient(argv[2]) > 0) {
            printf("[!]Finished!\n");
        } else {
            printf("[X]Something wrong...\n");
            return -1;
        }
        return 1;
    }
    if (!strcmp(argv[1], "upgrade")) {
        printf("[!]Start upgrade...\n");

        if (argc < 3) {
            printf("[X]Missing args\nUsage: ./wtf update project_name.\n");
            return -1;
        }
        if (checkFile("",argv[2],"./",1) == 0) {
            printf("[X]Project doesn't existed on the client...\n");
            printf("[X]Use checkout command first...\n");
            return -1;
        }
        if (checkFile(".Update", argv[2], "./", 0) == 0) {
            printf("[X]Didn't find update file..\n");
            printf("[X]Please run update command first...\n");
            return -1;
        } 
        if (checkFile(".Conflict", argv[2], "./", 0) > 0) {
            char *filename = combinePath(argv[2], ".Conflict");
            int a = countFile(filename);
            free(filename);
            if (a > 5) {
                printf("[X]Please resolve conflict files first...\n");
                return -1;
            } 
        }
        char *updatefile = combinePath(argv[2], ".Update");
        int c = countFile(updatefile);
        if (c < 5) {
            printf("[!]Seems nothing need to be upgrade...\n");
            free(updatefile);
            return -1;        
        }
        printf("[!]Everthing looks good. Ready to upgrade..\n");
        if (initialUpgradeClient(argv[2],updatefile) > 0) {
            printf("[!]Finished!\n"); 
        } else {
            printf("[X]Something wrong...\n");
            free(updatefile);
            return -1;
        }
        free(updatefile);
        return 1;
    }
    if (!strcmp(argv[1], "commit")) {
        printf("[!]Start commit...\n");
        if (argc < 3) {
            printf("[X]Missing args\nUsage: ./wtf commit project_name.\n");
            return -1;
        }
        if (checkFile("",argv[2],"./",1) == 0) {
            printf("[X]Project doesn't existed on the client...\n");
            printf("[X]Use checkout command first...\n");
            return -1;
        }
        if (checkFile(".Conflict", argv[2], "./", 0) > 0) {
            char *filename = combinePath(argv[2], ".Conflict");
            int a = countFile(filename);
            free(filename);
            if (a > 5) {
                printf("[X]Please resolve conflict files first...\n");
                return -1;
            } 
        }
        if (checkFile(".Update", argv[2], "./", 0) > 0) {
            char *filename = combinePath(argv[2], ".Update");
            int a = countFile(filename);
            free(filename);
            if (a > 5) {
                printf("[X]Find items which need to be upgraded...\n");
                printf("[X]Please use upgrade command first...\n");
                return -1;
            } 
        }
        if (initialCommitClient(argv[2]) > 0) {
            printf("[!]Finished!\n"); 
        } else {
            printf("[X]Something wrong...\n");
            return -1;
        }
        return 1;
    }
    if (!strcmp(argv[1], "push")) {
        printf("[!]Start push...\n");
        if (argc < 3) {
            printf("[X]Missing args\nUsage: ./wtf push project_name.\n");
            return -1;
        }
        if (checkFile("",argv[2],"./",1) == 0) {
            printf("[X]Project doesn't existed on the client...\n");
            printf("[X]Use checkout command first...\n");
            return -1;
        }

        if (checkFile(".Commit", argv[2], "./", 0) > 0) {
            char *filename = combinePath(argv[2], ".Commit");
            int a = countFile(filename);
            free(filename);
            if (a <= 5) {
                printf("[X]Please run commit command first...\n");
                return -1;
            } else {
                //printf("Everything looks good.....\n"); 
                if (initialPushClient(argv[2]) > 0) {
                    printf("[!]Finished!\n"); 
                } else {
                    printf("[X]Something wrong...\n");
                    return -1;
                }
                return 1; 
            }
        } else {
            printf("[X]Please run commit command first...\n");
            return -1;
        }
    }
    if (!strcmp(argv[1],"create")) {
        printf("[!]Start create...\n");
        if (argc < 3) {
            printf("[X]Missing args\nUsage: ./wtf create project_name.\n");
            return -1;
        }
        if (initialCreateClient(argv[2]) > 0) {
            printf("[!]Finished!\n");
            return 1;
        } else {
            printf("[X]Something wrong...\n");
            return -1;
        }
    }
    if (!strcmp(argv[1], "destroy")) {
        printf("[!]Start destory...\n");
        if (argc < 3) {
            printf("[X]Missing args\n");
            printf("Usage: ./wtf destroy project_name.\n");
            return -1;
        }
        if (initialDestroyClient(argv[2]) > 0) {
            printf("[!]Finished!\n");
            return 1;
        } else {
            printf("[X]Something wrong...\n");
            return -1;
        } 
    }
    if (!strcmp(argv[1], "currentversion")) {
        printf("[!]Start current version...\n");
        if (argc < 3) {
            printf("[X]Missing args\n");
            printf("Usage: ./wtf currentversion project_name.\n");
            return -1;
        }
        if (initialCurrentVersionClient(argv[2]) > 0) {
            printf("[!]Finished!\n");
            return 1;
        } else {
            printf("[X]Something wrong...\n");
            return -1;
        } 
    }
    if (!strcmp(argv[1],"history")) {
        printf("[!]Start history...\n");
        if (argc < 3) {
            printf("[X]Missing args\n");
            printf("Usage: ./wtf history project_name.\n");
            return -1;
        }
        if (initialHistoryClient(argv[2]) > 0) {
            printf("[!]Finished!\n");
            return 1;
        } else {
            printf("[X]Something wrong...\n");
            return -1;
        } 
        
    }
    if (!strcmp(argv[1], "rollback")) {
        printf("[!]Start rollback...\n");
        if (argc < 4) {
            printf("[X]Missing args\n");
            printf("Usage: ./wtf rollback project_name version.\n");
            return -1;
        }
        if (initialRollBackClient(argv[2], argv[3]) > 0) {
            printf("[!]Finished!\n");
            return 1;
        } else {
            printf("[X]Something wrong...\n");
            return -1;
        } 
    }

    printf("[X]Your command may correct(Lol), but we didn't finish yet!\n");
    printf("[X]Double check your input...\n");
    return -1;
}

/*
int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);
    getCommand(argc,argv);
    return 0;
}
*/


