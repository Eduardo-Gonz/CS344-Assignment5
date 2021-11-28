#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


void generateKey(int keyLength) {
    char key[keyLength + 1];

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
        key[i] = randChar;
    }

    key[keyLength] = '\n'; //append new line char to end of key

    printf("%s", key);
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