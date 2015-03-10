#include "helpers.h"
#include "stdlib.h"

const int BUFFER_SIZE = 4096;

int main() {
	char buf[BUFFER_SIZE];
	ssize_t count = 0;
	while ((count = read_(STDIN_FILENO, buf, BUFFER_SIZE)) != 0) {
		if (count == -1) {
			perror("Input error");
			return EXIT_FAILURE;
		}
		if (write_(STDOUT_FILENO, buf, count) == -1) {
			perror("Output error");
			return EXIT_FAILURE;
		}
	}

    return EXIT_SUCCESS;
}
