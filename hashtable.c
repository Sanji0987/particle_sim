#include "hashtable.h"

// using the cell no to create hashes

int calchash(const int cell){
    int temp_calc = cell * 32 - 120;
    int hash = temp_calc % tablesize;
    if (hash < 0) hash = hash*-1;

    return hash;

}

entry* hashtableint(){
    entry *hashtable = malloc(sizeof(entry)*tablesize);

    for (int i = 0 ; i < tablesize; i++){
        hashtable[i].cell = 0;
        hashtable[i].index_arr = (int*)calloc(20, sizeof(int));
        hashtable[i].count = 0 ;
        hashtable[i].capacity = 20;
        hashtable[i].status = EMPTY;
    }
    return hashtable;
}

void addindex(entry *hashtable , int cell ,  int index){

    int hash = calchash(cell);
    hashtable[hash].cell = cell;

    if (hashtable[hash].count >= hashtable[hash].capacity) {
        hashtable[hash].capacity *= 2;
        hashtable[hash].index_arr = realloc(hashtable[hash].index_arr, sizeof(int)*(hashtable[hash].capacity));
    }
    hashtable[hash].index_arr[hashtable[hash].count] = index;
    hashtable[hash].count++;
    hashtable[hash].status= OCCUPIED;
}

int* getindex(entry *hashtable , int cell){
    int hash = calchash(cell);
    return hashtable[hash].index_arr;

}

int bucketcount(entry *hashtable, int cell) {
    int hash = calchash(cell);
    return hashtable[hash].count;
}
