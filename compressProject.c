#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"
#include "networkr.h"

/*
compress whole project into BackupLib
the compressed file is named as version 
*/
int compressPro(char *project, char *version) {
    char head[] = "tar -czf ./.BackupLib/";
    char *i = mystrcat(head, project);
    char *i2 = mystrcat(i,"/");
    char *i3 = mystrcat(i2,version);
    char *i4 = mystrcat(i3, " ");
    char *i5 = mystrcat(i4, project);
    free(i);
    free(i2);
    free(i3);
    free(i4);
    system(i5);
    free(i5); 
    return 1;
}

/*
depress a project from BackupLib by a projectname and a version
*/
int depressPro(char *project, char *version) {
    char head[] = "tar -zxf ./.BackupLib/";
    char *i = mystrcat(head, project);
    char *i2 = mystrcat(i, "/");
    char *i3 = mystrcat(i2, version);
    char *i4 = mystrcat(i3, " -C ./");
    free(i);
    free(i2);
    free(i3);
    system(i4);
    free(i4);
}

/*
int main(int argc, char * argv[])
{
    //compressPro("Eugenie", "1");
    depressPro("Eugenie", "1");
    return 0;
}
*/



