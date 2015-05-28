#include <stdio.h>
#include <stdlib.h>
#include <helpers.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
	if (argc < 2) {
		puts("Usage: filter <executable file>");
		return -1;
	}

	char buf[BUF_SIZE + 1];

	char *args[argc + 1];
	for (int i = 0; i < argc - 1; i++) {
		args[i] = argv[i + 1];
	}
	args[argc - 1] = buf;
	args[argc] = NULL;
	ssize_t n;
	while ((n = read_until(STDIN_FILENO, buf, BUF_SIZE, '\n')) != 0) {
		if (n == -1) {
			perror("couldn't read");
			return EXIT_FAILURE;
		} else {
			if (buf[n - 1] == '\n')
				buf[n - 1] = '\0';
			else
				buf[n] = '\0';
			if (spawn(argv[1], args) == 0)
				puts(buf);
		}
	}
	return EXIT_SUCCESS;
}
