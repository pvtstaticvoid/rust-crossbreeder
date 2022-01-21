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
char GENES[NUM_UNIQ_GENES] = {'G', 'H', 'Y', 'W', 'X'};

// Number of gene slots in a breed (______).
#define NUM_GENE_SLOTS 6

// Max number of iterations.
#define MAX_ITER 20

// Maximum breeding multiplicity.
#define MAX_MULTIPLICITY 4

// A breed structure.
typedef struct breed_t {

    int num_parents;
    struct breed_t *parents[MAX_MULTIPLICITY];
    int score;
    int genes[7];
    char genes_str[7];

} breed_t;

/**
 * Check
 */
bool check_good(char *genes, char *target) {
    int net = 0;

    for (int i = 0; i < NUM_GENE_SLOTS; i++) {
        switch (genes[i]) {
            case 'G':
                net += 1;
                break;
            case 'H':
                net += 10;
                break;
            case 'Y':
                net += 100;
                break;
            case 'W':
                net += 1000;
                break;
            case 'X':
                net += 10000;
                break;
            default:
                fprintf(stderr, "invalid gene: %c", genes[i]);
                exit(1);
        }
    }

    for (int i = 0; i < NUM_GENE_SLOTS; i++) {
        switch (target[i]) {
            case 'G':
                net += -1;
                break;
            case 'H':
                net += -10;
                break;
            case 'Y':
                net += -100;
                break;
            case 'W':
                net += -1000;
                break;
            case 'X':
                net += -10000;
                break;
            default:
                fprintf(stderr, "invalid gene: %c", genes[i]);
                exit(1);
        }
    }

    if (net == 0) {
        return true;
    } else {
        return false;
    }

}

/**
 * Given a breed's genes, calculate its score.
 */
int get_score(const char *genes) {
    int score = 0;
    for (int i = 0; i < NUM_GENE_SLOTS; i++) {
        switch (genes[i]) {
            case 'G':
            case 'H':
            case 'Y':
                score += 1;
                break;
            case 'W':
            case 'X':
                score += 0;
                break;
            default:
                fprintf(stderr, "invalid gene: %c", genes[i]);
                exit(1);
        }
    }
    return score;
}

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
 * Try to add a new breed.
 */
bool try_add(breed_t* ordered_table[], breed_t* running_table[], int *running_table_len,
             const int batch[], int mult, char *new_genes) {

    int ordered_table_idx = get_index(new_genes);
    if (ordered_table[ordered_table_idx]) {
//            printf("exists, skipping!\n");
    } else if (get_score(new_genes) < 5) {
//            printf("too many reds\n");
    } else {

        breed_t *new_breed = malloc(sizeof(breed_t));

        // fill out parents
        new_breed->num_parents = mult;
        for (int parent_idx = 0; parent_idx < mult; parent_idx++) {
            new_breed->parents[parent_idx] = running_table[batch[parent_idx]];
        }

        // copy genes over
        strncpy(new_breed->genes_str, new_genes, NUM_GENE_SLOTS);

        // calculate score
        new_breed->score = get_score(new_breed->genes_str);

        // insert into lookup tables
        ordered_table[ordered_table_idx] = new_breed;
        running_table[*running_table_len] = new_breed;
        *running_table_len += 1;
        printf("created a new breed %s with %d parents\n", new_breed->genes_str, mult);

        if (check_good(new_breed->genes_str, "GGGYYY")) {
            printf("FOUND IT!\n");
            exit(0);
        }


    }
}


/**
 * Crossbreed a batch of breeds, and update the provided lookup tables when done.
 */
bool crossbreed(breed_t* ordered_table[], breed_t* running_table[], int *running_table_len,
                int batch[], int mult) {

    // Debugging information.
//    for (int k = 0; k < mult; k++) {
//        printf("%d: %s\t", batch[k], running_table[batch[k]]->genes_str);
//    }
//    printf("\n");

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

    char new_genes[7];
    new_genes[6] = '\0';

    // Debugging information.
    if (deterministic) {
//        printf("Deterministic!\t");
        for (int w = 0; w < NUM_GENE_SLOTS; w++) {
            new_genes[w] = GENES[argmax[w]];
        }
//        printf("%s\n", new_genes);

        try_add(ordered_table, running_table, running_table_len, batch, mult, new_genes);

    } else {
//        printf("Ambiguous!\n");
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
    FILE *file = fopen("../Data/Set 2.txt", "r");
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
        breed->score = get_score(breed->genes_str);

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

    int pivot_idx = 0;
    for (int iter = 0; iter < MAX_ITER && pivot_idx < running_table_len; iter++, pivot_idx++) {

        printf("\n\n\nIteration %d.\n", iter);

        for (int mult_curr_iter = 1; mult_curr_iter <= MAX_MULTIPLICITY; mult_curr_iter++) {

            // Temporary.
//            if (mult_curr_iter != 3) {
//                continue;
//            }

            // An index array for the next batch of breeds to be crossbred.
            int batch[mult_curr_iter];
            memset(batch, 0, sizeof(batch));
            batch[mult_curr_iter - 1] = pivot_idx;

            // Crossbreeding loop.
            while (true) {

                // Crossbreed current batch of genes.
                crossbreed(ordered_table, running_table, &running_table_len, batch, mult_curr_iter);

                // Loop variables.
                int batch_idx = 0;
                bool carry = false;
                bool done = false;

                // Increment the batch array.
                while (true) {

                    // Break if we've hit the multiplicity limit (where the last breed is a fixed pivot).
                    if (batch_idx == mult_curr_iter - 1) {
                        done = true;
                        break;
                    }

                    // Otherwise, increment the current digit in the array, carrying over if needed.
                    if (batch[batch_idx] < running_table_len - 1 && batch[batch_idx] < batch[mult_curr_iter - 1]) {
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
        }
    }

    printf("Done iterations! The total running table length is %d!\n", running_table_len);

}