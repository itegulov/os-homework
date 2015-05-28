#include <bufio.h>
#include <stdlib.h>

int main() {
	buf_t *buf = buf_new(4096);
	while (1) {
		ssize_t res = buf_fill(STDIN_FILENO, buf, buf_capacity(buf));
		if (res < 0) {
			buf_flush(STDOUT_FILENO, buf, buf_size(buf));
			return EXIT_FAILURE;
		}
		if (res == 0)
			break;
		if (buf_flush(STDOUT_FILENO, buf, buf_size(buf)) < 0)
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
