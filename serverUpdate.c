#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "networkr.h"
#include "readfiler.h"

//all the functions in this file need to be 
//locked and unlocked when it done


//includeLib : 1 will remove every thing in backuplib as well
//if get time... use c remove all the files in dir then remove empty dir
//instead of system call...
int destroyProject(char *project, int includeLib) {
    //char *command = mystrcat("rm -rf ./", project);
    //lock
    pthread_mutex_lock(&mutex_destroy_2);
    //system(command);  
    removeDir(project);
    if (includeLib) {
        char *command2 = mystrcat("./.BackupLib/", project);
        //system(command2);
        removeDir(command2);
        free(command2);
    }
    //unlock
    pthread_mutex_unlock(&mutex_destroy_2);
    //free(command);
    return 1;
}

/*
roll back action
remove current project
depress project from BackupLib to ./
wirte rollback info into .History
*/
int startRollBack(char *projectName, char *ver) {
    char *hisfile = mystrcat("./.BackupLib/", projectName);
    //char *command1 = mystrcat("rm -rf ", projectName);
    //lock
    pthread_mutex_lock(&mutex_rollback_1);
    char *his = mystrcat(hisfile, "/.History");
    int hisfd = open(his, 
                     O_WRONLY|O_APPEND|O_CREAT,
                     S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    write(hisfd, "ROLLBACK TO VERSION ", 20);
    write(hisfd, ver, strlen(ver));
    write(hisfd,"\n",1);
    close(hisfd);
    //system(command1);
    removeDir(projectName);
    depressPro(projectName, ver);
    //free(command1);
    free(his);
    free(hisfile);
    
    pthread_mutex_unlock(&mutex_rollback_1);
    //unlock
    return 1;
}


/*
update server side file action
update file 
update .Manifest
write actions into .History
*/
int serversideUpdateAction(char *msg, char *projectName) {
    int i;
    int cur = 0;
    char count[20];
    memset(count,'\0',20);
    for (i = 0; i < strlen(msg); i++) {
        if (msg[i] == '#') {
            cur = i;
            break;
        } else {
            count[cur] = msg[i];
            cur++;
        }    
    } 
    char *filecount = myitoa(atoi(count) + 1);
    char *resetMsg = (char *)malloc(sizeof(char) * 
                (strlen(msg) + 7));
    memset(resetMsg,'\0', (strlen(msg) + 7));
    memcpy(resetMsg, "#CKR#", 5);
    memcpy(&resetMsg[5], filecount, strlen(filecount));
    memcpy(&resetMsg[5+strlen(filecount)], &msg[cur],strlen(msg) - i);

    //lock here
    
    pthread_mutex_lock(&mutex0);
    //compress
    char *version = getversion(projectName);
    compressPro(projectName, version);

    //update file
    fromMsgUpdateFile(resetMsg, ".");
    free(resetMsg);   
    char *comfile = combinePath(projectName, ".Commit");
    int fd2 = open(comfile, O_RDONLY);
    Manifest *com = convertFileToList(fd2);
    close(fd2);
    Node *current = com->head;
    for (i = 0; i < com->size; i++) {
        //printf("com's list : %s\n", current->f);
        if (!strcmp(current->ver,"[D]")) {
            remove(current->f);
        }
        current = current->next;
    }
    


    //update .History
    
    char *hispro0 = combinePath("./.BackupLib", projectName);
    char *hispro = combinePath(hispro0, ".History");
    free(hispro0);

    
    pthread_mutex_lock(&mutex_his_1);
    int hisfd = open(hispro, 
                     O_WRONLY|O_APPEND|O_CREAT,
                     S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    write(hisfd, version, strlen(version));
    write(hisfd, "\n", 1);
    current = com->head;
    for (i = 0; i < com->size; i++) {
        write(hisfd, current->ver, strlen(current->ver));
        write(hisfd, " ", 1);
        write(hisfd, current->f, strlen(current->f));
        write(hisfd, "\n", 1);
        current = current->next;
    }
    free(hispro);
    close(hisfd);
    pthread_mutex_unlock(&mutex_his_1);

    //update .manifest list
    char *manifile = combinePath(projectName, ".Manifest");
    int fd = open(manifile, O_RDONLY);
    Manifest *mani = convertFileToList(fd);
    close(fd);
    Manifest *newmani = (Manifest *)malloc(sizeof(Manifest));
    newmani->size = 0;
    newmani->mver = mani->mver + 1; 
    int ni;
    Node *mycur = mani->head;
    for (ni = 0; ni < mani->size; ni++) {
        //printf("in mani's list %s\n", mycur->f);
        Node *res = findFile(com, mycur->f);
        if (res != NULL) {
            if (!strcmp(res->ver, "[D]")) {
                mycur=mycur->next;
                continue;
            } else {
                Node *nn = copyNode(mycur);
                char *nversion = myitoa(atoi(mycur->ver) + 1);
                nn->ver = nversion;
                nn->hash = res->hash;
                addItem(newmani, nn);
            }
        } else { // file with out any change
            addItem(newmani, copyNode(mycur));
        }
        mycur = mycur->next;
    }

    mycur = com->head;
    for (ni = 0; ni < com->size; ni++) {
        //printf("current tag: %s(%s)\n", mycur->ver,mycur->f);
        if (!strcmp(mycur->ver, "[A]")) {
            Node *nn = copyNode(mycur);
            char *initver = (char *)malloc(sizeof(char) * 2);
            initver[0] = '0';
            initver[1]= '\0';
            nn->ver = initver;
            //printf("nn's name : %s\n", nn->f);
            addItem(newmani, nn);
        }
        mycur = mycur->next;
    }


    //printf("check newmani's size : %d\n", newmani->size);

    
    pthread_mutex_lock(&mutex_mani_2);
    //write to .Manifest file
    fd = open(manifile,
              O_WRONLY|O_CREAT|O_TRUNC,
              S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
    char *newver = myitoa(newmani->mver);
    write(fd, newver, strlen(newver));
    write(fd, "\n", 1);
    mycur = newmani->head;
    for (ni = 0; ni < newmani->size; ni++) {
        write(fd, mycur->ver, strlen(mycur->ver));
        write(fd, ":", 1);    
        write(fd, mycur->f, strlen(mycur->f));
        write(fd, ":", 1);    
        write(fd, mycur->hash, strlen(mycur->hash));
        write(fd, "\n", 1);    
        mycur = mycur->next;
    }
    close(fd);
    pthread_mutex_unlock(&mutex_mani_2);
    //freeManifest(newmani);
    //freeManifest(com);
    //remove .Commit
    remove(comfile);

    free(manifile);
    free(comfile);
    free(version);
    //unlock
    
    pthread_mutex_unlock(&mutex0);
    
    return 1;
}

/*
int main(int argc, char * argv[])
{

    return 0;
}
*/



