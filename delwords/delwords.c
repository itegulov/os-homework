#include "stdio.h"
#include "stdlib.h"
#include "helpers.h"
#include "string.h"

#define BUF_SIZE 2048

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: ./delwords <word>\n");
		return EXIT_FAILURE;
	}
	char buf[BUF_SIZE];
	char *word = argv[1];

	ssize_t length = strlen(word);
	while(1) {
		ssize_t cnt = read_until_word(STDIN_FILENO, buf, BUF_SIZE, word, length);
		if (cnt == -1)
			return EXIT_FAILURE;
		if (cnt == 0)
			return EXIT_SUCCESS;
		if (cnt >= length) {
			int matched = 1;
			for (ssize_t i = 0; i < length; i++) {
				if (buf[cnt - length + i] != word[i]) {
					matched = 0;
					break;
				}
			}
			if (matched) {
				cnt -= length;
			}
		}
		if (write_(STDOUT_FILENO, buf, cnt) == -1)
			return EXIT_FAILURE;
	}
}
