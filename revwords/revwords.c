#include "helpers.h"
#include "stdlib.h"
#include "stdio.h"

#define BUF_SIZE 2048 

void swap(char* a, char* b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

int main() {
    char buf[BUF_SIZE];
    ssize_t cnt = 0;
    while (1) {
        cnt = read_until(STDIN_FILENO, buf, BUF_SIZE, ' ');
        if (cnt == -1)
            return EXIT_FAILURE;
        if (cnt == 0)
            return EXIT_SUCCESS;
        int is_delimiter = (buf[cnt - 1] == ' ');
        for (size_t i = 0; i < (cnt - is_delimiter) / 2; i++) {
            swap(&buf[i], &buf[cnt - i - 1 - is_delimiter]);
        }
        if (write_(STDOUT_FILENO, buf, cnt) == -1)
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
