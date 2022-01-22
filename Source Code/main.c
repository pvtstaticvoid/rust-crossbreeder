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
#define MAX_ITER 25

// Maximum breeding multiplicity.
#define MAX_MULT 8

// A breed structure.
typedef struct breed_t {

    int num_parents;
    struct breed_t *parents[MAX_MULT];
    int score;
    int genes[7];
    char genes_str[7];
    bool has_chance;
    double chance;

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
 * Visualize a breed's lineage.
 */
void visualize(breed_t *breed, int depth) {
    for (int i = 0; i < depth; i++) {
        printf("\t");
    }
    printf("%s\n", breed->genes_str);
    for (int i = 0; i < breed->num_parents; i++) {
        visualize(breed->parents[i], depth + 1);
    }
}

/**
 * Try to add a new breed.
 */
bool try_add(breed_t* ordered_table[], breed_t* running_table[], int *running_table_len,
             const int batch[], int mult, char *new_genes,
             bool has_base, int base_idx,
             bool has_chance, double chance) {

    int ordered_table_idx = get_index(new_genes);
    if (ordered_table[ordered_table_idx]) {
//            printf("exists, skipping!\n");
    } else if (get_score(new_genes) < 5) {
//            printf("too many reds\n");
    } else {



        breed_t *new_breed = malloc(sizeof(breed_t));

        // fill out chance, if required
        if (has_chance) {
            new_breed->has_chance = true;
            new_breed->chance = chance;
        }

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

        // debugging info
        printf("Trying to add %s", new_genes);
        if (has_chance) {
            printf(" (with chance %f)", chance);
        }
        printf("\n");
        if (has_base) {
            printf("\t%s (base)\n", running_table[batch[base_idx]]->genes_str);
        }
        for (int i = 0; i < mult; i++) {
            if (has_base && i == base_idx) {
                continue;
            }
            printf("\t%s\n", running_table[batch[i]]->genes_str);
        }

//        if (mult >= 3 && batch[0] != batch[1]) {
//            printf("nice\n");
//            exit(0);
//        }

//        if (check_good(new_breed->genes_str, "GHHWHG")) {
//            printf("FOUND IT!\n");
//            visualize(new_breed, 0);
//            exit(0);
//        }


    }
}


/**
 * Crossbreed a batch of breeds, and update the provided lookup tables when done.
 */
bool crossbreed(breed_t* ordered_table[], breed_t* running_table[], int *running_table_len,
                int batch[], int mult,
                bool has_base, int base_idx) {

    // Debugging information.
//    for (int k = 0; k < mult; k++) {
//        printf("%d: %s\t", batch[k], running_table[batch[k]]->genes_str);
//    }
//    printf("\n");

    // Aggregate gene weight array.
    int W[NUM_GENE_SLOTS][NUM_UNIQ_GENES] = {0};

    // Add up all gene weights (skipping the base if there is one).
    for (int batch_idx = 0; batch_idx < mult; batch_idx++) {

        // skip the base clone if there is one
        if (has_base && batch_idx == base_idx) {
            continue;
        }

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
    int argmax2d[NUM_GENE_SLOTS][NUM_UNIQ_GENES] = {0};
    int num_maxima[NUM_GENE_SLOTS] = {0};
    int max_wt[NUM_GENE_SLOTS];

    // Calculate maximum gene weight(s) for each slot
    for (int slot = 0; slot < NUM_GENE_SLOTS; slot++) {

        max_wt[slot] = -1;

        for (int gene = 0; gene < NUM_UNIQ_GENES; gene++) {
            if (W[slot][gene] > max_wt[slot]) {

                // update maxima
                max_wt[slot] = W[slot][gene];

                // reset argmax
                argmax2d[slot][0] = gene;
                num_maxima[slot] = 1;

            } else if (W[slot][gene] == max_wt[slot]) {

                // update argmax
                argmax2d[slot][num_maxima[slot]] = gene;
                num_maxima[slot] += 1;
            }
        }

    }


    // number of ambiguous slots (limited to 1)
    int num_slots_ambig = 0;

    // the chance of this happening
    double chance = 1.0;
    bool has_chance = false;

    // the 3 possible new genes (4+ is impossible)
    char new_genes_0[7];
    new_genes_0[6] = '\0';
    char new_genes_1[7];
    new_genes_1[6] = '\0';
    char new_genes_2[7];
    new_genes_2[6] = '\0';

    // base case only
    if (has_base) {
        // strncpy
        strncpy(new_genes_0, running_table[batch[base_idx]]->genes_str, 6);
        strncpy(new_genes_1, running_table[batch[base_idx]]->genes_str, 6);
        strncpy(new_genes_2, running_table[batch[base_idx]]->genes_str, 6);
    }

    bool no_base_ignore = false;

    // Loop over each slot, and check if there is a maximum gene weight.
    for (int slot = 0; slot < NUM_GENE_SLOTS; slot++) {
        if (max_wt[slot] > 10) {
            // there is a double up
            // base breed does not matter

            if (num_maxima[slot] == 1) {
                // unique max
                new_genes_0[slot] = GENES[argmax2d[slot][0]];
                new_genes_1[slot] = GENES[argmax2d[slot][0]];
                new_genes_2[slot] = GENES[argmax2d[slot][0]];
            } else {

                // multiple maxima...

                if (W[slot][3] == max_wt[slot] && W[slot][4] == max_wt[slot]) {

                    // both reds
                    new_genes_0[slot] = 'W';
                    new_genes_1[slot] = 'X';
                    new_genes_2[slot] = 'X';
                    num_slots_ambig += 1;
                    chance = 0.5;
                    has_chance = true;

                } else if (W[slot][3] == max_wt[slot]) {
                    // W only
                    new_genes_0[slot] = 'W';
                    new_genes_1[slot] = 'W';
                    new_genes_2[slot] = 'W';

                } else if (W[slot][4] == max_wt[slot]) {
                    // X only
                    new_genes_0[slot] = 'X';
                    new_genes_1[slot] = 'X';
                    new_genes_2[slot] = 'X';
                } else if (W[slot][0] == max_wt[slot] && W[slot][1] == max_wt[slot] && W[slot][2] == max_wt[slot]) {
                    // all greens
                    new_genes_0[slot] = 'G';
                    new_genes_1[slot] = 'H';
                    new_genes_2[slot] = 'Y';
                    num_slots_ambig += 1;
                    chance = 0.33;
                    has_chance = true;

                } else if (W[slot][0] == max_wt[slot] && W[slot][1] == max_wt[slot]) {
                    // GH_
                    new_genes_0[slot] = 'G';
                    new_genes_1[slot] = 'H';
                    new_genes_2[slot] = 'H';
                    num_slots_ambig += 1;
                    chance = 0.5;
                    has_chance = true;
                } else if (W[slot][0] == max_wt[slot] && W[slot][2] == max_wt[slot]) {
                    // G_Y
                    new_genes_0[slot] = 'G';
                    new_genes_1[slot] = 'Y';
                    new_genes_2[slot] = 'Y';
                    num_slots_ambig += 1;
                    chance = 0.5;
                    has_chance = true;
                } else if (W[slot][1] == max_wt[slot] && W[slot][2] == max_wt[slot]) {
                    // _HY
                    new_genes_0[slot] = 'H';
                    new_genes_1[slot] = 'Y';
                    new_genes_2[slot] = 'Y';
                    num_slots_ambig += 1;
                    chance = 0.5;
                    has_chance = true;
                } else if (W[slot][0] == max_wt[slot]) {
                    // G only
                    new_genes_0[slot] = 'G';
                    new_genes_1[slot] = 'G';
                    new_genes_2[slot] = 'G';
                } else if (W[slot][1] == max_wt[slot]) {
                    // H only
                    new_genes_0[slot] = 'H';
                    new_genes_1[slot] = 'H';
                    new_genes_2[slot] = 'H';
                } else if (W[slot][2] == max_wt[slot]) {
                    // Y only
                    new_genes_0[slot] = 'Y';
                    new_genes_1[slot] = 'Y';
                    new_genes_2[slot] = 'Y';
                } else {
                    printf("fuck\n");
                    exit(1);
                }

            }



        } else {
            // there is not a double up, special consideration for base breeds required


            if (has_base) {

                if (W[slot][3] > 0 && W[slot][4] > 0) {

                    // W & X
                    switch (running_table[batch[base_idx]]->genes_str[slot]) {
                        case 'G':
                        case 'H':
                        case 'Y':
                            // slot overwritten to W/X
                            new_genes_0[slot] = 'W';
                            new_genes_1[slot] = 'X';
                            new_genes_2[slot] = 'X';
                            num_slots_ambig += 1;
                            chance = 0.5;
                            has_chance = true;
                            break;
                        case 'W':
                        case 'X':
                            // slot unaffected
                            break;
                        default:
                            fprintf(stderr, "Invalid gene.");
                            exit(1);
                    }

                } else if (W[slot][3] > 0) {
                    // just W
                    switch (running_table[batch[base_idx]]->genes_str[slot]) {
                        case 'G':
                        case 'H':
                        case 'Y':
                            // slot overwritten to W
                            new_genes_0[slot] = 'W';
                            new_genes_1[slot] = 'W';
                            new_genes_2[slot] = 'W';
                            break;
                        case 'W':
                        case 'X':
                            // slot unaffected
                            break;
                        default:
                            fprintf(stderr, "Invalid gene.");
                            exit(1);
                    }
                } else if (W[slot][4] > 0) {
                    // just X
                    switch (running_table[batch[base_idx]]->genes_str[slot]) {
                        case 'G':
                        case 'H':
                        case 'Y':
                            // slot overwritten to W
                            new_genes_0[slot] = 'X';
                            new_genes_1[slot] = 'X';
                            new_genes_2[slot] = 'X';
                            break;
                        case 'W':
                        case 'X':
                            // slot unaffected
                            break;
                        default:
                            fprintf(stderr, "Invalid gene.");
                            exit(1);
                    }
                } else {
                    // neither X or Y
                    // slot unaffected
                }

            } else {

                // no base
                // since there are no double ups, this entire case is dependent on the base's genes, so ignore...
                no_base_ignore = true;

            }


        }
    }

    // only handle 1 ambiguous slot for now...
    if (num_slots_ambig > 1) {
        return false;
    }

    if (has_base) {
        // has base
        try_add(ordered_table, running_table, running_table_len, batch, mult, new_genes_0, true, base_idx, has_chance, chance);
        try_add(ordered_table, running_table, running_table_len, batch, mult, new_genes_1, true, base_idx, has_chance, chance);
        try_add(ordered_table, running_table, running_table_len, batch, mult, new_genes_2, true, base_idx, has_chance, chance);
    } else {
        // no base
        if (!no_base_ignore) {
            try_add(ordered_table, running_table, running_table_len, batch, mult, new_genes_0, false, -1, has_chance, chance);
            try_add(ordered_table, running_table, running_table_len, batch, mult, new_genes_1, false, -1, has_chance, chance);
            try_add(ordered_table, running_table, running_table_len, batch, mult, new_genes_2, false, -1, has_chance, chance);
        }

    }

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
        breed->score = get_score(breed->genes_str);

        // Calculate index and fill out pointers in the tables.
        int index = get_index(breed->genes_str);
        ordered_table[index] = breed;
        running_table[running_table_len] = breed;
        running_table_len += 1;

    }

    // Print data file summary.
    printf("Number of starter seeds: %d.\n", running_table_len);
    assert(running_table_len >= MAX_MULT);

    // Main loop.

    int pivot_idx = 0;
    for (int iter = 0; iter < MAX_ITER && pivot_idx < running_table_len; iter++, pivot_idx++) {

        printf("\n\n\nIteration %d.\n", iter);

        for (int mult_curr_iter = 1; mult_curr_iter <= MAX_MULT; mult_curr_iter++) {



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
                crossbreed(ordered_table, running_table, &running_table_len, batch, mult_curr_iter, false, -1);

                // base plant might matter
                if (2 <= mult_curr_iter && mult_curr_iter <= 5) {
                    for (int base = 0; base < mult_curr_iter; base++) {
                        crossbreed(ordered_table, running_table, &running_table_len, batch, mult_curr_iter, true, base);
                    }
                }

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