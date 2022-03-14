#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "stringsLib.h"

#ifndef VM_NAME_SIZE
    #define VM_NAME_SIZE 20
#endif

typedef struct t_VmData{
    char * name;
    int value;
}TVmData;

int cmpVmName(const void* a, const void* b){
    const char* aa = (*((TVmData **) a))->name;
    const char* bb = (*((TVmData **) b))->name;

    return strcmp(aa,bb);
}

int main(int argc, char *argv[]){

    int i,
        n_vm,
        iterations;

    TVmData **vms;
    TVmData **vms_ordered;
    TVmData *s_vm_name;
    TVmData **f_vm;

    clock_t time;
    double time_taken;

    if(argc != 3){
        fprintf(stderr, "./prog [N_VM] [ITERATIONS].\n");
        exit(1);
    }

    n_vm = strtol(argv[1], NULL, 10);
    iterations = strtol(argv[2], NULL, 10);

    vms = (TVmData **) malloc(n_vm * sizeof(TVmData));
    if(vms == NULL){
        fprintf(stderr, "ERROR:'vms' allocation. \n");
        exit(1);
    }
    vms_ordered = (TVmData **) malloc(n_vm * sizeof(TVmData));
    if(vms_ordered == NULL){
        fprintf(stderr, "ERROR:'vms_ordered' allocation. \n");
        exit(1);
    }

    srand(123); 

    for (i = 0; i < n_vm; i++)
    {
        vms[i] = (TVmData *) malloc(sizeof(TVmData));
        vms[i]->name = generateString(VM_NAME_SIZE);
        vms[i]->value = i;
    }

    memcpy(vms_ordered, vms, n_vm * sizeof(TVmData *));

    qsort(vms_ordered, n_vm, sizeof(TVmData *), cmpVmName);
    
    #ifdef DEBUG
        // Defining Original VmData list.
        printf("ORIGINAL LIST:\n");
        for (i = 0; i < n_vm; i++)        {
            printf("\t[%s] - [%d]\n", vms[i]->name, vms[i]->value);
        }
        
        // SortingtimespecElapsedmData list.
        printf("SORTED LIST:\n");
        for (i = 0; i < n_vm; i++){        {
            printf("\t[%s] - [%d]\n", vms_ordered[i]->name, vms_ordered[i]->value);
        }
    #endif

    // SEARCHING
    printf("SEARCHING VM:\n");
    time = clock();
    for (i = 0; i < iterations; i++)
    {
        s_vm_name = vms[i % n_vm];
        f_vm = bsearch(&s_vm_name, vms_ordered, n_vm, sizeof(TVmData *), cmpVmName);
        #ifdef DEBUG
            printf("\t%s - %d\n", (*f_vm)->name, (*f_vm)->value);
        #endif
    }

    
    time = clock() - time;
    time_taken = ((double) time)/CLOCKS_PER_SEC;

    printf("BSEARCH:\n");
    printf("n_vm: %d    VM_NAME_SIZE: %d     iterations: %d\n", n_vm, VM_NAME_SIZE, iterations);
    printf("\t%f sec (%f per iteration)\n", time_taken, time_taken/iterations);

    for (i = 0; i < n_vm; i++)
    {
        free(vms[i]);
    }

    free(vms);
    free(vms_ordered);
}