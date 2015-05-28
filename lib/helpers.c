#define _POSIX_SOURCE
#define _GNU_SOURCE

#include "helpers.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
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

execargs_t new_execargs(char** args) {
	size_t cnt = 0;
	for (char** i = args; *i != NULL; i++) {
		cnt++;
	}
	cnt++;
	char** res = (char**) malloc(cnt * sizeof(void*));
	if (res == NULL) {
		return NULL;
	}
	for (int i = 0; i < cnt; i++) {
		if (args[i] == NULL) {
			res[i] = NULL;
			continue;
		}	

		size_t len = strlen(args[i]);
		char* str = malloc(len + 1);
		if (str == NULL) {
			for (int j = 0; j < i; j++) {
				free(res[j]);
			}
			free(res);
			return NULL;
		}
		memcpy(str, args[i], len + 1);
		res[i] = str;
	}
	return res;
}

void free_execargs(execargs_t e) {
	for (char** i = e; *i != NULL; i++) {
		free(*i);
	}
	free(e);
}

int exec(execargs_t* e) {
	return execvp(**e, *e);
}

int runpiped(execargs_t** args, size_t n) {
	int pipes[2 * (n - 1)];
	for (size_t i = 1; i < n; i++) {
		if (pipe2(pipes + 2 * (i - 1), O_CLOEXEC)) {
			for (int j = 1; j < i; j++) {
				close(pipes[j * 2 - 2]);
				close(pipes[j * 2 - 1]);
			}
			return -1;
		}
	}

	pid_t pids[n];
	memset(pids, 0, n * sizeof(pid_t));

	sigset_t mask;
	sigset_t original_mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &original_mask);

	int error = 0;
	for (size_t i = 0; i < n; i++) {
		int pid = fork();
		if (pid < 0) {
			error = 1;
			break;
		} else if (pid) {
			pids[i] = pid;
		} else {
			if (i > 0) dup2(pipes[i * 2 - 2], STDIN_FILENO);
			if (i < n - 1) dup2(pipes[i * 2 + 1], STDOUT_FILENO);
			exec(args[i]);
			exit(-1);
		}
	}
	
	for (size_t i = 1; i < n; i++) {
		close(pipes[i * 2 - 2]);
		close(pipes[i * 2 - 1]);
	}	

	if (error) {
		for (int i = 0; i < n; i++) {
			if (pids[i]) {
				kill(pids[i], SIGKILL);
				waitpid(pids[i], 0, 0);
			}
		}
		sigprocmask(SIG_SETMASK, &original_mask, 0);
		return -1;
	}

	siginfo_t info;
	int killed = 0;
	while (1) {
		sigwaitinfo(&mask, &info);
		if (info.si_signo == SIGINT) {
			break;
		}

		if (info.si_signo == SIGCHLD) {
			int chld;
			while ((chld = waitpid(-1, 0, WNOHANG)) > 0) {
				for (int i = 0; i < n; i++) {
					if (pids[i] == chld) {
						pids[i] = 0;
						break;
					}
				}
				killed++;
				if (killed == n) {
					break;
				}
			}
		}
	}

	for (int i = 0; i < n; i++) {
		if (pids[i]) {
			kill(pids[i], SIGKILL);
			waitpid(pids[i], 0, 0);	
		}
	}

	sigprocmask(SIG_SETMASK, &original_mask, 0);
	return 0;
}
