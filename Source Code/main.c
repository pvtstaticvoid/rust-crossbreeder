#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// Number of different gene types (GHYWX).
#define NUM_UNIQ_GENES 5
int GENES[NUM_UNIQ_GENES] = {'G', 'H', 'Y', 'W', 'X'};

// Number of gene slots in a breed (______).
#define NUM_GENE_SLOTS 6

// Max number of iterations.
#define MAX_ITER 10

// Maximum breeding multiplicity.
#define MAX_MULTIPLICITY 3

// A breed structure.
typedef struct breed_t {

    int num_parents;
    struct breed_t *p0;
    struct breed_t *p1;
    struct breed_t *p2;

    int genes[7];
    char genes_str[7];

} breed_t;

/**
 * Given a breed's genes, calculate its index in the ordered lookup table.
 */
int get_index(const char *genes) {
    int index = 0;
    for (int i = 0; i < NUM_GENE_SLOTS; i++) {
        switch (genes[i]) {
            case 'G':
                index += 0 * (int) pow(NUM_UNIQ_GENES, NUM_GENE_SLOTS - i - 1);
                break;
            case 'H':
                index += 1 * (int) pow(NUM_UNIQ_GENES, NUM_GENE_SLOTS - i - 1);
                break;
            case 'Y':
                index += 2 * (int) pow(NUM_UNIQ_GENES, NUM_GENE_SLOTS - i - 1);
                break;
            case 'W':
                index += 3 * (int) pow(NUM_UNIQ_GENES, NUM_GENE_SLOTS - i - 1);
                break;
            case 'X':
                index += 4 * (int) pow(NUM_UNIQ_GENES, NUM_GENE_SLOTS - i - 1);
                break;
            default:
                fprintf(stderr, "invalid gene: %c", genes[i]);
                exit(1);
        }
    }
    return index;
}

/**
 * Crossbreed a batch of breeds, and update the provided lookup tables when done.
 */
bool crossbreed(breed_t* ordered_table[], breed_t* running_table[], int *running_table_len,
                int batch[], int mult) {

    // Debugging information.
    for (int k = 0; k < mult; k++) {
        printf("%d: %s\t", batch[k], running_table[batch[k]]->genes_str);
    }
    printf("\n");

    // Aggregate gene weight array.
    int W[NUM_GENE_SLOTS][NUM_UNIQ_GENES] = {0};

    // Add up all gene weights.
    for (int batch_idx = 0; batch_idx < mult; batch_idx++) {
        for (int slot = 0; slot < NUM_GENE_SLOTS; slot++) {
            switch (running_table[batch[batch_idx]]->genes_str[slot]) {
                case 'G':
                    W[slot][0] += 6;
                    break;
                case 'H':
                    W[slot][1] += 6;
                    break;
                case 'Y':
                    W[slot][2] += 6;
                    break;
                case 'W':
                    W[slot][3] += 10;
                    break;
                case 'X':
                    W[slot][4] += 10;
                    break;
                default:
                    fprintf(stderr, "Invalid gene.");
                    exit(1);
            }
        }
    }

    bool deterministic = true;
    int argmax[NUM_UNIQ_GENES] = {0};

    // Loop over each slot, and check if there is a maximum gene weight.
    for (int slot = 0; slot < NUM_GENE_SLOTS; slot++) {

        int max = -1;
        bool unique_max = true;

        for (int gene = 0; gene < NUM_UNIQ_GENES; gene++) {
            if (W[slot][gene] > max) {
                max = W[slot][gene];
                argmax[slot] = gene;
                unique_max = true;
            } else if (W[slot][gene] == max) {
                unique_max = false;
            }
        }

        if (!unique_max) {
            deterministic = false;
            break;
        }

    }

    if (deterministic) {
        printf("Deterministic!\t");
        for (int w = 0; w < NUM_GENE_SLOTS; w++) {
            printf("%c", GENES[argmax[w]]);
        }
        printf("\n");
    } else {
        printf("Ambiguous!\n");
    }

    return deterministic;

}

int main() {

    // CMake only.
    chdir("../Source Code");

    // Lookup tables.
    breed_t* ordered_table[(int) pow(NUM_UNIQ_GENES, NUM_GENE_SLOTS)];
    breed_t* running_table[(int) pow(NUM_UNIQ_GENES, NUM_GENE_SLOTS)];

    // Table counters.
    int running_table_len = 0;
    int running_table_curr = 0;

    // Open data file.
    FILE *file = fopen("../Data/Set 1.txt", "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    // Read data file.
    char line[256];
    while (fgets(line, sizeof(line), file)) {

        // Create the breed object.
        breed_t *breed = malloc(sizeof(breed_t));
        breed->num_parents = 0;
        strncpy(breed->genes_str, line, 6);

        // Calculate index and fill out pointers in the tables.
        int index = get_index(breed->genes_str);
        ordered_table[index] = breed;
        running_table[running_table_len] = breed;
        running_table_len += 1;

    }

    // Print data file summary.
    printf("Number of starter seeds: %d.\n", running_table_len);
    assert(running_table_len >= MAX_MULTIPLICITY);

    // Main loop.
    for (int iter = 0; iter < MAX_ITER; iter++) {
        for (int max_mult_curr_iter = 2; max_mult_curr_iter < MAX_MULTIPLICITY; max_mult_curr_iter++) {

            // Temporary.
            max_mult_curr_iter = 2;

            // An index array for the next batch of breeds to be crossbred.
            int batch[max_mult_curr_iter];
            memset(batch, 0, sizeof(batch));

            // Crossbreeding loop.
            while (true) {

                // Crossbreed current batch of genes.
                crossbreed(ordered_table, running_table, &running_table_len, batch, max_mult_curr_iter);

                // Loop variables.
                int batch_idx = 0;
                bool carry = false;
                bool done = false;

                // Increment the batch array.
                while (true) {

                    // Break if we've hit the multiplicity limit.
                    if (batch_idx == max_mult_curr_iter) {
                        done = true;
                        break;
                    }

                    // Otherwise, increment the current digit in the array, carrying over if needed.
                    if (batch[batch_idx] < running_table_len - 1) {
                        batch[batch_idx]++;
                        break;
                    } else {
                        carry = true;
                        batch_idx++;
                    }

                }

                if (done) {
                    break;
                }

                if (carry) {
                    for (int carry_idx = 0; carry_idx < batch_idx; carry_idx++) {
                        batch[carry_idx] = batch[batch_idx];
                    }
                }

            }

            // Temporary.
            exit(0);

        }
    }
}