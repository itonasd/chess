#ifndef UTILS
#define UTILS

int mathmin(int a, int b) {
    return (a > b) ? b : a;
}

int row_revert(int row, int row_size) {
    return (row_size - 1) - row;
}

#endif