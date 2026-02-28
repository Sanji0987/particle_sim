#include "hashtable.h"

// using the cell no to create hashes

int calchash(const int64_t cell){
    int64_t temp_calc = cell * 32 - 120;
    int hash = (int)(temp_calc % tablesize);
    if (hash < 0) hash = hash*-1;

    return hash;

}

entry* hashtableint(){
    entry *hashtable = malloc(sizeof(entry)*tablesize);
    if (hashtable == NULL) return NULL;

    for (int i = 0 ; i < tablesize; i++){
        hashtable[i].cell = 0;
        hashtable[i].index_arr = (int*)calloc(initial_bucket_cap, sizeof(int));
        hashtable[i].count = 0 ;
        hashtable[i].capacity = initial_bucket_cap;
        hashtable[i].status = EMPTY;
    }
    return hashtable;
}

void addindex(entry *hashtable , int64_t cell ,  int index){

    int hash = calchash(cell);
    hashtable[hash].cell = cell;

    if (hashtable[hash].count >= hashtable[hash].capacity) {
        int new_cap = hashtable[hash].capacity * 2;
        // if realloc fails we skip this insert, particle misses force checks this frame
        int *temp = realloc(hashtable[hash].index_arr, sizeof(int) * new_cap);
        if (temp == NULL) return;
        hashtable[hash].index_arr = temp;
        hashtable[hash].capacity = new_cap;
    }
    hashtable[hash].index_arr[hashtable[hash].count] = index;
    hashtable[hash].count++;
    hashtable[hash].status= OCCUPIED;
}

void hashtablefree(entry *hashtable){
    for (int i = 0; i < tablesize; i++){
        free(hashtable[i].index_arr);
    }
    free(hashtable);
}

int* getindex(entry *hashtable , int64_t cell){
    int hash = calchash(cell);
    return hashtable[hash].index_arr;

}

int bucketcount(entry *hashtable, int64_t cell) {
    int hash = calchash(cell);
    return hashtable[hash].count;
}
