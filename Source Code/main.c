#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Number of unique genes (GHYWX).
#define GENE_UNIQ 5

// Length of genes in a breed.
#define GENE_LEN 6

typedef struct breed_t {
    int num_parents;
    struct breed_t *p0;
    struct breed_t *p1;
    struct breed_t *p2;
    struct breed_t *p3;
    // this will end up not being a C string :(
    char genes[7];
} breed_t;


// GHYWX
int get_index(const char *genes) {
    int index = 0;
    for (int i = 0; i < GENE_LEN; i++) {
        switch (genes[i]) {
            case 'G':
                index += 0 * (int) pow(GENE_UNIQ, GENE_LEN - i - 1);
                break;
            case 'H':
                index += 1 * (int) pow(GENE_UNIQ, GENE_LEN - i - 1);
                break;
            case 'Y':
                index += 2 * (int) pow(GENE_UNIQ, GENE_LEN - i - 1);
                break;
            case 'W':
                index += 3 * (int) pow(GENE_UNIQ, GENE_LEN - i - 1);
                break;
            case 'X':
                index += 4 * (int) pow(GENE_UNIQ, GENE_LEN - i - 1);
                break;
            default:
                fprintf(stderr, "invalid gene: %c", genes[i]);
                exit(1);
        }
    }
    return index;
}

int main() {

    // CMake only.
    chdir("../Source Code");

    breed_t* ordered_table[(int) pow(GENE_UNIQ, GENE_LEN)];
    breed_t* running_table[(int) pow(GENE_UNIQ, GENE_LEN)];
    int running_table_curr_idx = 0;

    FILE *file = fopen("../Data/Set 1.txt", "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    char line[256];

    while (fgets(line, sizeof(line), file)) {

        // Create the breed object.
        breed_t *breed = malloc(sizeof(breed_t));
        breed->num_parents = 0;
        strncpy(breed->genes, line, 6);

        // Calculate index and fill out pointers in the tables.
        int index = get_index(breed->genes);
        ordered_table[index] = breed;
        running_table[running_table_curr_idx] = breed;
        running_table_curr_idx += 1;

    }

}