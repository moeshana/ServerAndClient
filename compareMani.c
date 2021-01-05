#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"
#include "networkr.h"

//convert content to Manifest linked list
//format of this msg : 
//#command#projectName#nn#filename =>(#nn#content)<===this part
Manifest *makeManiListFromMsg(char *msg) {
    //printf("in makefunc : %s\n",msg);
    int i;
    int current = 0;
    for (i = 1; i < strlen(msg); i++) {
        if (msg[i] == '#') {
            current = i;
            break;
        } 
    }
    current += 3;
    Manifest *list = (Manifest *) malloc(sizeof(Manifest));
    memset(list, '\0',sizeof(Manifest));
    list->size = 0;
    Node *cn = (Node *) malloc(sizeof(Node));
    memset(cn, '\0', sizeof(Node));
    int count = 0;
    int type = 0;
    char temp[1024];
    int y;
    for (y = current; y < strlen(msg); y++) {
        if (msg[y] == '\n') {
            char *f = (char *)malloc(sizeof(char) * (count + 1));
            memset(f, '\0', count + 1);
            memcpy(f, temp, count);
            cn->hash = f;                    
            memset(temp, '\0', 1024);
            count = 0;
            type = 0;
            addItem(list, cn);
            cn = (Node *)malloc(sizeof(Node));
            memset(cn, '\0', sizeof(Node));
            continue;
        }
        if (msg[y] == ':'){
            char *f = (char *)malloc(sizeof(char) * (count + 1));
            memset(f, '\0', count + 1);
            memcpy(f, temp, count);
            if (type == 0) {
                cn->ver = f;
                type++;
            } else {
                cn->f = f;
                type++;
            }
            memset(temp, '\0', 1024);
            count = 0;
        } else {
            temp[count] = msg[y];
            count++;
        }
    }
    return list;
}


/*
read a file and convert it to a Linked List
didn't close()
forget the return type... Manifest(>_<)
for other file(.Update/.Conflict/.Commit), 
they must have the same format:
char *\n
<char*>:<filepath/filename>:<hash>\n
<char*>:<filepath/filename>:<hash>\n
..........
*/
Manifest *convertFileToList(int fd) {
    Manifest *list = (Manifest *) malloc(sizeof(Manifest));
    memset(list, '\0',sizeof(Manifest));
    list->size = 0;
    int flag = -1;
    char buffer[1024];
    char temp[1024];
    int count = 0;
    int readIn = 0;
    memset(temp,'\0',1024);
    int firstLine = 0;
    int type = 0;
    Node *current;
    do {
        do {
            flag = read(fd, buffer+readIn, 1024-readIn);
            readIn += flag;
        } while (readIn < 1023 && flag >0);
        int i;
        for (i = 0; i < readIn; i++) {
            if ((buffer[i] == '\n')) {
                if (!firstLine) {
                    list->mver = atoi(temp);
                    firstLine = 1;
                    memset(temp, '\0', 1024);
                    count = 0;
                    continue;
                } 
                char *f = (char *)malloc(sizeof(char) * (count + 1));
                memset(f, '\0', count + 1);
                memcpy(f, temp, count);
                current->hash = f;                    
                memset(temp, '\0', 1024);
                count = 0;
                type = 0;
                //printf("in convert func %s\n",current->f);
                //printf("in convert func %s\n",current->hash);
                //printf("in convert func %s\n",current->ver);
                int oldsize = list->size;
                addItem(list, current);
                if (list->size != oldsize) {
                    current = (Node *)malloc(sizeof(Node));
                    memset(current, '\0', sizeof(Node));
                }
                continue;
            
            }
            if (buffer[i] == ':'){
                char *f = (char *)malloc(sizeof(char) * (count + 1));
                memset(f, '\0', count + 1);
                memcpy(f, temp, count);
                if (type == 0) {

                    current = (Node *)malloc(sizeof(Node)); 
                    memset(current, '\0', sizeof(Node));
                    current->ver = f;
                    type++;
                } else {
                    current->f = f; 
                    type++;
                }
                memset(temp, '\0', 1024);
                count = 0;
            } else {
                temp[count] = buffer[i];
                count++;
            }
        }
        readIn = 0;
    } while (flag > 0);
    return list;
}

/*method for Manifest linked list
add a new Item into a linked list(in the beginning)
*/
int addItem(Manifest *lists, Node *newNode) {
    if (newNode->f == NULL ||
            newNode->ver == NULL ||
            newNode->hash == NULL) {
        //bad node
        return lists->size;
        }
    if (lists->size == 0) {
        lists->head = newNode;
    } else {
        newNode->next = lists->head;
        lists->head = newNode;
    }
    lists->size++;
    //printf("size up %d\n",lists->size);
    return lists->size;
}

/*
count how many char in a file
arg : file path
reutrn : sizeof this file
*/
int countFile(char *fileName) {
    int fd = open(fileName, O_RDONLY); 
    int count = 0;
    char temp[1];
    while (read(fd, temp, 1) > 0) {
        count++;
    }
    close(fd);
    return count;
}

/*
copy the data(new memory) of a node, not include the "next" pointer
*/
Node *copyNode(Node* anode) {
    Node *res = (Node *)malloc(sizeof(Node));
    memset(res, '\0', sizeof(Node));
    char *f = (char *)malloc(sizeof(char) * (strlen(anode->f) + 1));
    memset(f, '\0', strlen(anode->f) + 1);
    memcpy(f, anode->f, strlen(anode->f));
    char *ver = (char *)malloc(sizeof(char) * (strlen(anode->ver) + 1));
    memset(ver, '\0', strlen(anode->ver) + 1);
    memcpy(ver, anode->ver, strlen(anode->ver));
    char *hash = (char *)malloc(sizeof(char) * (strlen(anode->hash) + 1));
    memset(hash, '\0', strlen(anode->hash) + 1);
    memcpy(hash, anode->hash, strlen(anode->hash));
    res->f = f;
    res->ver = ver;
    res->hash = hash;
    return res;
}

/*
convert a integer into a char array
alloc memeory for this array
this char array need to be free somewhere else
*/
char *myitoa(int a) {
    char res[100]; 
    int count = 0;
    do {
        res[count] = a % 10 + '0'; 
        a /= 10;
        count++;
    }while(a != 0);
    char *num = (char *)malloc(sizeof(char) * (count + 1));
    memset(num, '\0', count + 1);
    int i;
    int y = count - 1;
    for (i = 0; i < count; i++) {
        num[i] = res[y];
        y--;
    }
    num[count] = '\0';
    //printf("num is %s\n", num);
    return num;
}


// set a msg from a given file name:
// filename_len#filename#content_len#content#
// return formatted msg string
char *setMsgFromFile(char *fileName) {
    int count = countFile(fileName);
    char *fileLen = myitoa(strlen(fileName));
    char *countLen = myitoa(count);
    //int len = (count + strlen(fileLen) + 5 + strlen(countLen));
    int len = (5 + strlen(fileLen) + strlen(fileName) + strlen(countLen) + count);    
    int current = 0;
    char *msg = (char *)malloc(sizeof(char) * (len + 1));
    memset(msg, '\0', len + 1);
    int fd = open(fileName, O_RDONLY);
    char *buf = readFile(fd, count);
    memcpy(msg, fileLen, 
           strlen(fileLen));
    msg[strlen(fileLen)] = '\0';
    
    current += strlen(fileLen);
    msg[current++] = '#';
    
    memcpy(&msg[current], fileName, strlen(fileName));
    msg[current + strlen(fileName)] = '\0';
    
    current += strlen(fileName);
    msg[current++] = '#';
    
    memcpy(&msg[current], countLen, strlen(countLen));
    msg[current + strlen(countLen)] = '\0';
    
    current += strlen(countLen);
    msg[current++] = '#';
    
    //printf("buf is %s\n", buf);
    memcpy(&msg[current],buf,strlen(buf));
    //msg[current + strlen(buf)] = '\0';
    
    current += strlen(buf);
    msg[current++] = '#';
    msg[current] = '\0';
    //printf("in set funciotn : ====> %s(%d)\n",msg,strlen(msg));

     
    //why free here will cause a munmap_chunk() error => fixed
    free(buf);
    
    free(fileLen);
    free(countLen);
    close(fd);
    //printf("msg is : %s\n",msg);
    return msg;
}

/*
looking for a fileName in Manifest list and return the node of this entry
*/
Node *findFile(Manifest *list, char *file) {
    int i;
    Node *current = list->head;
    for (i = 0; i < list->size; i++) {
        if (!strncmp(current->f, file, strlen(file))) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/*
free Manifest list
*/
int freeManifest(Manifest *list) {
    Node *current = list->head;
    int i;
    for (i = 0; i < list->size; i++) {
        //printf("current : %d\n", i);
        //printf("f : %s\n",current->f);
        //printf("ver : %s\n",current->ver);
        //printf("hash : %s\n",current->hash);
        free(current->f);
        free(current->ver);
        free(current->hash);
        /*if (current->next == NULL) {
            break;
        }*/
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
    return 1;
}

/*
// test main
int main(int argc, char * argv[])
{
    int fd = open("./test.txt",O_RDONLY);

    Manifest *list = convertFileToList(fd);
    Node *current = list->head;
    printf("size : %d(%d)\n", list->size,list->mver);
    for (int i = 0; i < list->size; i++) {
        printf("fn : %s\n", current->f);
        printf("ver : %s\n", current->ver);
        printf("hash : %s\n", current->hash);
        current = current->next;
        printf("<==============================>\n");
    }
    printf("--------------\n");
    current = findFile(list,"./Junfeng.txt");
    printf("find node => fn : %s\n", current->f);
    printf("find node => ver : %s\n", current->ver);
    printf("find node => hash : %s\n", current->hash);
    freeManifest(list);
    printf("count : %d\n",countFile("./test.txt"));
    printf("count : %s\n",myitoa(1548));
    printf("<<<====================================>>>\n");
    setMsgFromFile("./WTFserver.c");
    return 0;
}
*/



