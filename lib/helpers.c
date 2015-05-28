#include "helpers.h"
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>

ssize_t read_(int fd, void *buf, size_t count) {
    ssize_t cnt = 0;
    ssize_t offset = 0;
    do {
        cnt = read(fd, buf + offset, count);
        if (cnt < 0)
            return -1;
        count -= cnt;
        offset += cnt;
    } while (cnt > 0);
    return offset;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    ssize_t cnt = 0;
    size_t offset = 0;
    do {
        cnt = write(fd, buf + offset, count);
        if (cnt == -1)
            return -1;
        count -= cnt;
        offset += cnt;
    } while (cnt > 0);
    return offset;
}

ssize_t read_until(int fd, void *buf, size_t count, char delimiter) {
    ssize_t cnt = 0;
    char *chars = (char *) buf;
    for (size_t offset = 0;;) {
        cnt = read(fd, buf + offset, 1);
        if (cnt == -1)
            return -1;
        if (cnt == 0)
            return offset;
        if (chars[offset++] == delimiter || offset == count)
            return offset;
    }
}

int spawn(const char * file, char * const argv[]) {
	int result = -1;
	pid_t pid = fork();
	if (pid == -1) {
		perror("error on forking");
		return -1;
	}
	if (pid == 0) {
		execvp(file, argv);
		perror("execvp didn't terminate");
		return -1;
	} else if (wait(&result) == -1) {
		perror("error on waiting");
		return -1;
	}
	if (WIFEXITED(result)) {
		return WEXITSTATUS(result);
	} else {
		perror("file didn't exited normally");
		return -1;
	}
}
