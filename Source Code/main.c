#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

// Number of unique genes (GHYWX).
#define GENE_UNIQ 5

// Length of genes in a breed.
#define GENE_LEN 6

// Max numebr of interations.
#define MAX_ITER 10

// Maximum breeding multiplicity.
#define MAX_MULTIPLICITY 3

typedef struct breed_t {
    int num_parents;
    struct breed_t *p0;
    struct breed_t *p1;
    struct breed_t *p2;
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


bool crossbreed(breed_t* ordered_table[], breed_t* running_table[], int *n,
                int I[], int m) {

    for (int k = 0; k < m; k++) {
        printf("%d: %s\t", I[k], running_table[I[k]]->genes);
    }
    printf("\n");

    int W[GENE_LEN][GENE_UNIQ] = {0};

    // for each breed
    for (int i = 0; i < m; i++) {
        // for each gene
        for (int g = 0; g < GENE_LEN; g++) {

            switch (running_table[I[i]]->genes[g]) {
                case 'G':
                    W[g][0] += 6;
                    break;
                case 'H':
                    W[g][1] += 6;
                    break;
                case 'Y':
                    W[g][2] += 6;
                    break;
                case 'W':
                    W[g][3] += 10;
                    break;
                case 'X':
                    W[g][4] += 10;
                    break;
                default:
                    fprintf(stderr, "invalid gene: %c", running_table[I[i]]->genes[g]);
                    exit(1);
            }
        }

    }


    bool ambiguous = false;


    // for each gene slot
    for (int g = 0; g < GENE_LEN; g++) {

        int argmax = 0;
        int max = -1;
        bool uniq = true;

        // for each weight
        for (int w = 0; w < GENE_UNIQ; w++) {

//            printf("%d ", W[g][w]);


            if (W[g][w] > max) {
                max = W[g][w];
                argmax = w;
            } else if (W[g][w] == max) {
                uniq = false;
                break;
            }


        }
//        printf("\n");

        if (uniq == false) {
            ambiguous = true;
            break;
        }

    }


    if (ambiguous) {
        printf("Ambiguous!\n");
    } else {
        printf("Deterministic!\n");
    }

}



int main() {

    // CMake only.
    chdir("../Source Code");

    breed_t* ordered_table[(int) pow(GENE_UNIQ, GENE_LEN)];
    breed_t* running_table[(int) pow(GENE_UNIQ, GENE_LEN)];
    int n = 0;

    FILE *file = fopen("../Data/Set 1.txt", "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    char line[256];


    // read data in
    while (fgets(line, sizeof(line), file)) {

        // Create the breed object.
        breed_t *breed = malloc(sizeof(breed_t));
        breed->num_parents = 0;
        strncpy(breed->genes, line, 6);

        // Calculate index and fill out pointers in the tables.
        int index = get_index(breed->genes);
        ordered_table[index] = breed;
        running_table[n] = breed;
        n += 1;

    }

    printf("number of starter seeds: %d\n", n);

    if (n < MAX_MULTIPLICITY) {
        fprintf(stderr, "Max multiplicity greater than number of starter seeds.\n");
        exit(1);
    }

    for (int iter = 0; iter < MAX_ITER; iter++) {

        for (int m = 2; m < MAX_MULTIPLICITY; m++) {

            m = 2;


            int I[m];
            for (int i = 0; i < m; i++) {
                I[i] = 0;
            }

            while (1) {

                // do stuff;
                crossbreed(ordered_table, running_table, &n, I, m);


                int i = 0;

                int carry = 0;
                int brk = 0;

                while (1) {

                    if (i == m) {
                        brk = 1;
                        break;
                    }

                    if (I[i] < n - 1) {
                        I[i]++;
                        break;
                    } else {
                        carry = 1;
                        i++;
                    }
                }

                if (brk) {
                    break;
                }

                if (carry) {
                    for (int j = 0; j < i; j++) {
                        I[j] = I[i];
                    }
                }



            }



            exit(0);

        }

    }


}