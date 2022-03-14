#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * generateString(int word_size){

    int i, 
        r;
    char *word;

    word = (char *) malloc(word_size + 1);
    for (i = 0; i < word_size; i++){
        r = rand();
        word[i] = (r % 91) + 33;
    } 
    word[i] = '\0';

    return word;    
}