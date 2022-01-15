#include <stdio.h>

#include "skard.h"

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    printf("Hello, Skard!\n");
    printf("Version: %s\n", SKARD_VERSION);

    return 0;
}
