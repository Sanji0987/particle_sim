#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define tablesize 2048
#define initial_bucket_cap 20

typedef enum {
    EMPTY,
    OCCUPIED,
} EntryStatus;


typedef struct entry{
    int64_t cell;
    int *index_arr;
    int count;
    int capacity;
    EntryStatus status;
}entry;


int calchash(int64_t cell);

entry* hashtableint();

void addindex(entry *hashtable ,int64_t cell ,  int index);

void hashtablefree(entry *hashtable);
int* getindex(entry *hashtable , int64_t cell);
int bucketcount(entry *hashtable, int64_t cell) ;


#endif // HASHTABLE_H_