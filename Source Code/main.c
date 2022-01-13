#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct breed_t {
    int num_parents;
    struct breed_t *p0;
    struct breed_t *p1;
    struct breed_t *p2;
    struct breed_t *p3;
    char genes[4];
} breed_t;


int main() {

    // CMake only.
    chdir("../Source Code");

    breed_t* breed_table[4096];

    FILE *fp = fopen("../Data/Set 1.txt", "r");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

}