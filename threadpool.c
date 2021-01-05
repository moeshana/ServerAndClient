#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "readfiler.h"
#include "networkr.h"


//add a threadnode into threadpool
int addThread(threadpool *t, threadnode *tn) {
    if (t->size == 0) {
        t->head = tn;
    } else {
        tn->next = t->head;
        t->head = tn;
    }
    t->size++;
    return t->size;
}

//remove a threadnode from a threadpool
int removeThread(threadpool *t, threadnode *tn) {
    int i;
    threadnode *current = t->head;
    for (i = 0; i < t->size; i++) {
        if (current->next == tn) {
            current->next = tn->next;
            t->size--;
            return 1;
        }
        current = current->next;
    }
    return -1;
}

//when thread is done, free this spot
//threadnode->isDone must be set 1
int freeEmpty(threadpool *t) {
    int i;
    int len = t->size;
    threadnode *current = t->head;
    for (i = 0; i < len; i++) {
        if (current->isDone == 1) {
            threadnode *tem = current;
            removeThread(t,tem);
            current = current->next;
            free(tem);
            continue;
        }
        current = current->next;
    }
    return 1;
}




