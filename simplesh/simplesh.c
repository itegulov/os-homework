#include <stdio.h>
#include <stdlib.h>
#include <helpers.h>
#include <bufio.h>
#include <signal.h>
#include <errno.h>

#define BUF_SIZE 4096

void print_line(int sig) {
	write_(STDOUT_FILENO, "\n", 1);
	signal(SIGINT, print_line);
}

int main(int argc, char *argv[]) {
	char buf[BUF_SIZE];

	buf_t *buff = buf_new(BUF_SIZE);
	signal(SIGINT, print_line);

	for (;;) {
		write_(STDOUT_FILENO, "$", 1);
		ssize_t res = buf_getline(STDIN_FILENO, buff, '\n', buf);
		if (res < 0) {
			if (errno == EINTR) {
				continue;
			}
			return EXIT_FAILURE;
		}
		if (res == 0) {
			return 0;
		}
		buf[res] = 0;
		int programs = 1;
		for (ssize_t i = 0; i < res; i++) {
			if (buf[i] == '|') {
				programs++;
				buf[i] = 0;
			}
		}

		execargs_t e[programs];
		execargs_t* ep[programs];
		ssize_t last_pos = 0;
		int cnt = 0;
		int ok = 1;
		for (ssize_t i = 0; i <= res; i++) {
			if (buf[i] == 0) {
				int args = (buf[i - 1] != ' ');
				ssize_t last_space = last_pos - 1;
				for (ssize_t j = last_pos; j < i; j++) {
					if (buf[j] == ' ') {
						if (last_space != j - 1) {
							args++;
						}
						buf[j] = 0;
						last_space = j;
					}
				}
				char* proc_args[args + 1];
				proc_args[args] = 0;
				ssize_t last_arg = last_pos;
				int arg_cnt = 0;
				for (ssize_t j = last_pos; j <= i; j++) {
					if (buf[j] == 0) {
						if (last_arg != j) {
							proc_args[arg_cnt++] = buf + last_arg;
						}
						last_arg = j + 1;
					}
				}

				if (!(e[cnt++] = new_execargs(proc_args))) {
					for (int j = 0; j < cnt - 1; j++) {
						free_execargs(e[j]);
					}
					write_(STDOUT_FILENO, "Ran out of memory\n", 14);
					ok = 0;
					break;
				} else {
					ep[cnt - 1] = e + cnt - 1;
				}
				last_pos = i + 1;
			}
		}

		if (!ok) {
			continue;
		}

		if (runpiped(ep, programs)) {
			write_(STDOUT_FILENO, "runpiped error\n", 15);
		}

		for (int i = 0; i < programs; i++) {
			free_execargs(e[i]);
		}
	}
}
