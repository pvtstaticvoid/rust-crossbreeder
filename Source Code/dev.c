//
// Created by Roger Liu on 2022-01-17.
//

#include <stdio.h>
#include <math.h>



int get_digit(int dec, int idx) {
    return (dec / (int) pow(5, idx)) % 5;
}

void set_digit(int *dec, int idx, int new) {
    int old = (*dec / (int) pow(5, idx)) % 5;
    *dec += (new - old) * (int) pow(5, idx);
}


int main() {

    int dec = 450;
    for (int i = 0; i < 4; i++) {
        printf("%d\n", get_digit(dec, i));
    }

    set_digit(&dec, 2, 4);

    for (int i = 0; i < 4; i++) {
        printf("%d\n", get_digit(dec, i));
    }

    printf("hello!\n");
}
