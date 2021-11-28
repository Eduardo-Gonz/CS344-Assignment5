#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


void generateKey(int keyLength) {
    char *key = calloc(keyLength + 1, sizeof(char));
    //Generate random chars for the key
    for(int i = 0; i < keyLength; i++) {
        int randNum = random() % 27 + 1;

        if(randNum < 27) {
            randNum += 64; //ascii value of letters A-Z
        }
        else {
            randNum = 32; //ascii value of space
        }

        char randChar = randNum;
        strncat(key, &randChar, 1);
    }

    key[keyLength] = '\n'; //append new line char to end of key
    
    write(1, key, keyLength + 1);
    fflush(stdout);
    free(key);
}


int main(int argc, char *argv[]) {
    srandom(time(NULL));

    if(argc != 2) {
        printf("Incorrect number of arguments");
        return EXIT_SUCCESS;
    }

    int length = atoi(argv[1]);
    generateKey(length);
    return EXIT_SUCCESS;
}