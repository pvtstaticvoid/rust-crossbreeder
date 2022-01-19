//
// Created by Roger Liu on 2022-01-17.
//

#ifndef RUST_CROSSBREEDER_UTIL_H
#define RUST_CROSSBREEDER_UTIL_H

int get_digit(int dec, int idx) {
    return (dec / (int) pow(5, idx)) % 5;
}

void set_digit(int *dec, int idx, int new) {
    int old = (*dec / (int) pow(5, idx)) % 5;
    *dec += (new - old) * (int) pow(5, idx);
}





#endif //RUST_CROSSBREEDER_UTIL_H
