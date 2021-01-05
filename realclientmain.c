#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"
#include "networkr.h"

//main method for client
//for WTF
int main(int argc, char *argv[]) {
    signal(SIGINT, signalHandler);
    getCommand(argc,argv);
    return 0;
}




