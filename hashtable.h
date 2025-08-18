#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define tablesize 160

typedef enum {
    EMPTY,
    OCCUPIED,
} EntryStatus;


typedef struct entry{
    int cell;
    int *index_arr;
    int count;
    int capacity;
    EntryStatus status;
}entry;


int calchash(int cell);

entry* hashtableint();

void addindex(entry *hashtable ,int cell ,  int index);

int* getindex(entry *hashtable , int cell);
int bucketcount(entry *hashtable, int cell) ;


#endif // HASHTABLE_H_