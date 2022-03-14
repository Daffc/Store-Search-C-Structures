#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "stringsLib.h"

#ifndef VM_NAME_SIZE
    #define VM_NAME_SIZE 20
#endif

typedef struct t_VmData{
    char * name;
    int value;
}TVmData;

typedef struct t_hash{
    int used, size;
    TVmData **items;
}THash;


static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

uint32_t murmur3_32( uint8_t* key)
{
	uint32_t h = 0;
    size_t len = VM_NAME_SIZE;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}


uint32_t djb2(uint8_t *str)
{
    uint32_t hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

uint32_t sdbm(uint8_t *str){
    uint32_t hash = 0;
    int c;

    while (c = *str++)
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

uint32_t loselose(uint8_t *str)
{
    uint32_t hash = 0;
    int c;

    while (c = *str++)
        hash += c;

    return hash;
}


// --------- HASH FUNCTIONS -----------
THash * initHash(int table_size){
    THash * h;

    h = (THash *) malloc(sizeof(THash));

    h->used = 0;
    h->size = table_size;
    h->items = (TVmData **) malloc(table_size * sizeof(TVmData));
    if(h->items  == NULL){
        fprintf(stderr, "ERROR:'hash' allocation. \n");
        exit(1);
    }
    memset(h->items, table_size * sizeof(TVmData), 0);

    return h;
}
void freeHash(THash * hash){
    free(hash->items);
    free(hash);
}

void insertHash(THash *h, TVmData* p_vm, uint32_t (*h_func)(uint8_t *)){

    uint32_t    h_value;

    if(h->size == h->used){
        fprintf(stderr, "ERROR: Hash Table is full.\n");
        exit(1);
    }

    h_value = (*h_func)((uint8_t *) p_vm->name);
    h_value = h_value % h->size; 

    while(h->items[h_value] != 0x0){
        h_value++;
        h_value = h_value % h->size; 
    }

    h->items[h_value] = p_vm;
    h->used ++;
}

TVmData* searchHashItem(THash *h, char* key, uint32_t (*h_func)(uint8_t *)){

    uint32_t    h_value;

    h_value = (*h_func)((uint8_t *) key);
    h_value = h_value % h->size; 

    while(strcmp(h->items[h_value]->name, key) != 0){
        h_value++;
        h_value = h_value % h->size; 
    }

    return h->items[h_value];
}

void printHashTable(THash *h){

    int i;

    printf("\tHASHTABLE:\n");

    for (i = 0; i < h->size; i++)
    {
        if(h->items[i] !=NULL)
            printf("\t\t[%d] %s - %d\n", i, h->items[i]->name, h->items[i]->value);
        else
            printf("\t\t[%d] (nil) - (nil)\n", i);

    }
}

int main(int argc, char *argv[]){

    int i,
        n_vm,
        iterations,
        * acesses,
        counter;

    uint32_t h_value;

    TVmData **vms;
    THash  * hash;
    TVmData *s_vm_name;
    TVmData *f_vm;

    clock_t time;
    double time_taken;

    if(argc != 3){
        fprintf(stderr, "./prog [N_VM] [ITERATIONS].\n");
        exit(1);
    }

    n_vm = strtol(argv[1], NULL, 10);
    iterations = strtol(argv[2], NULL, 10);

    hash = initHash(n_vm);


    vms = (TVmData **) malloc(n_vm * sizeof(TVmData));
    if(vms == NULL){
        fprintf(stderr, "ERROR:'vms' allocation. \n");
        exit(1);
    }

    acesses = malloc(n_vm * sizeof(int));


    // GENERATING VM DATA
    srand(123); 

    for (i = 0; i < n_vm; i++){
        vms[i] = (TVmData *) malloc(sizeof(TVmData));
        vms[i]->name = generateString(VM_NAME_SIZE);
        vms[i]->value = i;
    }

    #ifdef MURMUR3 
        // POPULATING HASH
        memset(acesses, 0, n_vm * sizeof(int));
        counter = 0;
        printf("HASHES murmur3_32:\n");
        for (i = 0; i < n_vm; i++){
            h_value = murmur3_32((uint8_t *) vms[i]->name);
            #ifdef DEBUG
                printf("\t[%s] - [%u] -[%u]\n", vms[i]->name, h_value, h_value%hash->size);
            #endif
            insertHash(hash, vms[i], murmur3_32);
            
            if( acesses[h_value % n_vm] > 0){
                counter ++;
            }
            acesses[h_value % n_vm] ++;
        }

        #ifdef DEBUG
            // PRINTG HASH TABLE
            printHashTable(hash);
        #endif

        // SEARCHING
        time = clock();
        for (i = 0; i < iterations; i++)
        {
            f_vm = searchHashItem(hash, vms[i % n_vm]->name, murmur3_32);
            #ifdef DEBUG
                printf("[%d] %s - %d\n", i,f_vm->name, f_vm->value);
            #endif
        }
        time = clock() - time;
        time_taken = ((double) time)/CLOCKS_PER_SEC;
        printf("\tn_vm: %d    VM_NAME_SIZE: %d     iterations: %d\n", n_vm, VM_NAME_SIZE, iterations);
        printf("\t%f sec (%f per iteration)\n", time_taken, time_taken/iterations);
        printf("\tTOTAL COLISIONS: %d\n", counter);
    #endif

    #ifdef DJB2 
        // POPULATING HASH
        memset(acesses, 0, n_vm * sizeof(int));
        counter = 0;
        printf("HASHES djb2:\n");
        for (i = 0; i < n_vm; i++){
            h_value = djb2((uint8_t *) vms[i]->name);
            #ifdef DEBUG
                printf("\t[%s] - [%u] -[%u]\n", vms[i]->name, h_value, h_value%hash->size);
            #endif
            insertHash(hash, vms[i], djb2);
            
            if( acesses[h_value % n_vm] > 0){
                counter ++;
            }
            acesses[h_value % n_vm] ++;
        }

        #ifdef DEBUG
            // PRINTG HASH TABLE
            printHashTable(hash);
        #endif

        // SEARCHING
        time = clock();
        for (i = 0; i < iterations; i++)
        {
            f_vm = searchHashItem(hash, vms[i % n_vm]->name, djb2);
            #ifdef DEBUG
                printf("[%d] %s - %d\n", i,f_vm->name, f_vm->value);
            #endif
        }
        time = clock() - time;
        time_taken = ((double) time)/CLOCKS_PER_SEC;
        printf("\tn_vm: %d    VM_NAME_SIZE: %d     iterations: %d\n", n_vm, VM_NAME_SIZE, iterations);
        printf("\t%f sec (%f per iteration)\n", time_taken, time_taken/iterations);
        printf("\tTOTAL COLISIONS: %d\n", counter);
    #endif

    #ifdef SDBM 
        // POPULATING HASH
        memset(acesses, 0, n_vm * sizeof(int));
        counter = 0;
        printf("HASHES sdbm:\n");
        for (i = 0; i < n_vm; i++){
            h_value = sdbm((uint8_t *) vms[i]->name);
            #ifdef DEBUG
                printf("\t[%s] - [%u] -[%u]\n", vms[i]->name, h_value, h_value%hash->size);
            #endif
            insertHash(hash, vms[i], sdbm);
            
            if( acesses[h_value % n_vm] > 0){
                counter ++;
            }
            acesses[h_value % n_vm] ++;
        }

        #ifdef DEBUG
            // PRINTG HASH TABLE
            printHashTable(hash);
        #endif

        // SEARCHING
        time = clock();
        for (i = 0; i < iterations; i++)
        {
            f_vm = searchHashItem(hash, vms[i % n_vm]->name, sdbm);
            #ifdef DEBUG
                printf("[%d] %s - %d\n", i,f_vm->name, f_vm->value);
            #endif
        }
        time = clock() - time;
        time_taken = ((double) time)/CLOCKS_PER_SEC;
        printf("\tn_vm: %d    VM_NAME_SIZE: %d     iterations: %d\n", n_vm, VM_NAME_SIZE, iterations);
        printf("\t%f sec (%f per iteration)\n", time_taken, time_taken/iterations);
        printf("\tTOTAL COLISIONS: %d\n", counter);
    #endif

    #ifdef LOSELOSE 
        // POPULATING HASH
        memset(acesses, 0, n_vm * sizeof(int));
        counter = 0;
        printf("HASHES loselose:\n");
        for (i = 0; i < n_vm; i++){
            h_value = loselose((uint8_t *) vms[i]->name);
            #ifdef DEBUG
                printf("\t[%s] - [%u] -[%u]\n", vms[i]->name, h_value, h_value%hash->size);
            #endif
            insertHash(hash, vms[i], loselose);
            
            if( acesses[h_value % n_vm] > 0){
                counter ++;
            }
            acesses[h_value % n_vm] ++;
        }

        #ifdef DEBUG
            // PRINTG HASH TABLE
            printHashTable(hash);
        #endif

        // SEARCHING
        time = clock();
        for (i = 0; i < iterations; i++)
        {
            f_vm = searchHashItem(hash, vms[i % n_vm]->name, loselose);
            #ifdef DEBUG
                printf("[%d] %s - %d\n", i,f_vm->name, f_vm->value);
            #endif
        }
        time = clock() - time;
        time_taken = ((double) time)/CLOCKS_PER_SEC;
        printf("\tn_vm: %d    VM_NAME_SIZE: %d     iterations: %d\n", n_vm, VM_NAME_SIZE, iterations);
        printf("\t%f sec (%f per iteration)\n", time_taken, time_taken/iterations);
        printf("\tTOTAL COLISIONS: %d\n", counter);
    #endif

    for (i = 0; i < n_vm; i++)
    {
        free(vms[i]);
    }

    free(vms);
    freeHash(hash);
}

