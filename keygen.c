#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



int main(int argc, char *argv[]) {

    srandom(time(NULL));
    int randNum = random() % 100000;

    return EXIT_SUCCESS;
}