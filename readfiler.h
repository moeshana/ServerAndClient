#ifndef _READFILER_H_
#define _READFILER_H_


#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


// path/fileName
typedef struct FilePath {
    char *fn; //file name
    char *fp; //file path
} FilePath;

typedef struct Node {
    char *f;  //filePath / fileName
    char *ver; 
    //version of this file, also in other file(not .Manifest)
    //we may still use this struct, we will store OpCode instead of ver.
    //seems we used a bad name =_=|||
    char *hash;
    struct Node *next;
} Node;

typedef struct Manifest {
    struct Node *head;
    int size;
    int mver;   //version of this current version
} Manifest;

//in getHash.c
unsigned int BKDRHash(char *str);
unsigned int readFileThenHash(int fd);
char *readFile(int fd, int len);
int removeFile(char *project, char *file);

//in checkFile.c
char* combinePath(char* path, char* filename);
int checkFile(char *fileName, char *project, char *path, int isPName);
int writeSomeStuffIntoFile(char *file, char *str, int isOverWrite);
int startwith(char *str, char *tar);
FilePath *spiltFilePath(char *str);
char *mystrcat(char *first, char *second);
int removeDir(char *path);

//in compareMani.c
Manifest *convertFileToList(int fd);
Manifest *makeManiListFromMsg(char *msg);
int addItem(Manifest *lists, Node *newNode);
int countFile(char *fileName);
char *myitoa(int a); 
char *setMsgFromFile(char *fileName);
Node *findFile(Manifest *list, char *file);
int freeManifest(Manifest *list);
Node *copyNode(Node *anode);

//in compressProject.c
int compressPro(char *project, char *version);
int depressPro(char *project, char *version);


#endif
