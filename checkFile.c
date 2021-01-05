#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "readfiler.h"

/**
check if str start with tar
return 1 if it is, 0 not
*/
int startwith(char *str, char *tar) {
    return strncmp(str, tar, strlen(tar)) == 0? 1 : 0;
}

/*
get a file name and file path from a given string
*/
FilePath *spiltFilePath(char *str) {
    FilePath *fpres = (FilePath *)malloc(sizeof(FilePath));
    int len = strlen(str);
    int loc = 0;
    int i;
    for (i = len -1; i >= 0; i--) {
        if (!strncmp(&str[i], "/", 1)) {
            //printf("%d\n",i);
            loc = i;
            break;
        }
    }
    char *name = (char *)malloc(sizeof(char) * (loc + 2));    
    memset(name, '\0', loc + 2);
    memcpy(name, str, loc + 1);
    char *path = (char *)malloc(sizeof(char) * (strlen(str) - loc + 1));
    memset(path, '\0', strlen(str) - loc + 1);
    memcpy(path, &str[loc + 1], strlen(str) - loc);
    //>_< opposite
    fpres->fp = name;
    fpres->fn = path;
    return fpres;
}

/*
connect two string => "first" + "second" + '\0'.
*/
char *mystrcat(char *first, char *second) {
    int flen = strlen(first);
    int slen = strlen(second);
    int total = flen + slen + 1;
    char *res = (char *)malloc(sizeof(char) * total);
    memset(res, '\0', total);
    memcpy(res, first, flen);
    memcpy(&res[flen],second, slen);
    return res;
}


/*
//test main for spilt path
int main(int a, char*b[]) {
    //FilePath *f = spiltFilePath("./haha/junfeng.txt");
    //printf("fp: %s\n", f->fp);
    //printf("fn: %s\n", f->fn);
    char *f = (char *) malloc (sizeof(char) * 5);
    memset(f,'\0',5);
    memcpy(f,"haha",5);
    char *f2 = (char *) malloc (sizeof(char) * 78);
    //memset(f2,'\0',5);
    memcpy(f2,"Junfeng,Eugenie",17);
    f2[17] = '\0';
    char *news = mystrcat(f,f2);
    printf("%s\n",news);
    free(f);
    free(f2);
    free(news);
    return 0;
}
*/

/**
connect path when reading a directory
result will be path + "/" + filename
*/
char* combinePath(char* path, char* filename) {
    int len = strlen(path) + strlen(filename) + 2;
    char *newPath = (char*)malloc(sizeof(char)*len);
    memset(newPath, '\0', len);
    if (newPath == NULL) {
        printf("Error : Malloc function failed.\n");
        return NULL;
    }
    int i, j;
    for (i = 0; i<strlen(path); i++) {
        newPath[i] = path[i];
    }
    for (i = 0, j = strlen(path) + 1;
            j < len - 1;
            j++, i++) {
        newPath[j] = filename[i];
    }
    newPath[strlen(path)] = '/';
    //newPath[len - 1] = '\0';
    return newPath;
}

/*
check file/project, return: 1 if find the file, otherwise 0
return a value greater than 1 for the files with the same name exist
isPName: 1 for checking if a project exists, 0 for chceking file
when isPName is 1, filename doesn't matter, can pass a empty string 
*/
int checkFile(char *fileName, char *project, char *path, int isPName) {
    struct dirent *de;
    DIR *dir = opendir(path);
    int res = 0;
    while ((de = readdir(dir)) != NULL) {
        if (!(strcmp(de->d_name, ".") && strcmp(de->d_name, ".."))) {
            continue;
        }
        if (isPName && 
            de->d_type == DT_DIR &&
            !strcmp(de->d_name, project)) {
                //printf("get outside from here\n")
                closedir(dir);
                return 1;
        }
        if (!isPName) {
            //printf("%s : %s <=> %s\n", path, fileName, de->d_name);
            if (de->d_type == DT_DIR) {
                char *nd = combinePath(path, de->d_name);
                DIR *d = opendir(nd);
                res += checkFile(fileName, project, nd, isPName);
                closedir(d);
                free(nd);
            } else {
                if (!strcmp(de->d_name, fileName)) { 
                    if (strstr(path, project) != NULL) {
                        closedir(dir);
                        return 1;
                    }
                }            
            }
        }
    }
    closedir(dir);
    return res;
}

/*
write str into the file
isOverWrite set to 1;
maybe implement append write later? 
*/
int writeSomeStuffIntoFile(char *file, char *str, int isOverWrite) {
    int fd = -1;
    if (isOverWrite) {
        fd = open(
            file, 
            O_WRONLY|O_CREAT|O_TRUNC,
            S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);   
    } else {
        fd = open(file, 
            O_WRONLY|O_CREAT|O_APPEND,
            S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);   
    }
    if (fd == -1) {
        printf("[X]Error: create file failed.");
        return -1;
    }
    //printf("write : %s\n", str);
    write(fd, str, strlen(str));
    close(fd);
    return 1;
}

/*
remove a dir by using remove()
first remove every thing in this dir, then remove dir
if there are any sub-dir, recursive call this function
*/
int removeDir(char *path) {
    struct dirent *de;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return 0;
    }
    int res = 0;
    while ((de = readdir(dir)) != NULL) {
        if (!(strcmp(de->d_name, ".") && strcmp(de->d_name, ".."))) {
            continue;
        }
        char *nd = combinePath(path, de->d_name);
        if (de->d_type == DT_DIR) {
            res -= removeDir(nd);
        } else {
            res -= remove(nd);
            if (res < 0) {
                printf("[X]Remove %s failed...\n", de->d_name);
            } else {
                printf("[-]Remove %s successed...\n", de->d_name); 
            }
        }
        free(nd);
    }
    closedir(dir);
    res -= remove(path);
    if (res < 0) {
        printf("[X]Remove %s failed...\n", path);
    } else {
        printf("[-]Remove %s successed...\n", path); 
    }   
    return res;
}








/*
// test main
int main(int argc, char * argv[]) {
    char str[] = "#mytext#";
    char tar[] = "#mytext#i";
    printf("res is : %d\n", startwith(str, tar));
    //printf("res is %d\n", 
            //checkFile("fintest5.txt","proj1","./",1));
    //writeSomeStuffIntoFile("./Configure", "UJ", 1);
    return 0;
}
*/




