#define _POSIX_C_SOURCE 201505

#include <bufio.h>
#include <helpers.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#define BUF_SIZE 4096
#define LISTEN_CNT 10

int bind_and_listen(struct addrinfo* host) {
	int sock;
	struct addrinfo* rp;
	for (rp = host; rp != NULL; rp = rp->ai_next) {
		sock = socket(host->ai_family, SOCK_STREAM, IPPROTO_TCP);
		if (sock < 0) {
			continue;
		}
		int one = 1;

		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
			close(sock);
			continue;
		}
		if (bind(sock, host->ai_addr, host->ai_addrlen)) {
			close(sock);
			continue;
		}
		break;
	}

	if (rp == NULL) {
		return -1;
	}

	if (listen(sock, LISTEN_CNT)) {
		close(sock);
		return -1;
	}
	return sock;
}

int proccess(int fd1, int fd2) {
	buf_t* buf = buf_new(BUF_SIZE);
	if (!buf) {
		perror("buf_new");
		return -1;
	}
	while (1) {
		int off = buf_fill(fd1, buf, 1);
		if (off < 0) {
			buf_flush(fd2, buf, buf_size(buf));
			return -1;
		}
		if (off == 0) break;
		if (buf_flush(fd2, buf, buf_size(buf)) < 0) {
			perror("buf_flush");
			return -1;
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: bipiper <port1> <port2>");
		return EXIT_FAILURE;
	}

	struct addrinfo *host1, *host2;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, argv[1], &hints, &host1)) {
		perror("getaddrinfo");
		return EXIT_FAILURE;
	}
	if (getaddrinfo("localhost", argv[2], &hints, &host2)) {
		perror("getaddrinfo");
		return EXIT_FAILURE;
	}

	int sock1 = bind_and_listen(host1);
	if (sock1 < 0) {
		perror("bind_and_listen");
		return EXIT_FAILURE;
	}
	int sock2 = bind_and_listen(host2);
	if (sock2 < 0) {
		perror("bind_and_listen");
		return EXIT_FAILURE;
	}

	freeaddrinfo(host1);
	freeaddrinfo(host2);

	while (1) {
		int fd1 = accept(sock1, 0, 0);
		if (fd1 < 0) {
			perror("accept");
			continue;
		}
		int fd2 = accept(sock2, 0, 0);
		if (fd2 < 0) {
			close(fd1);
			perror("accept");
			continue;
		}
		pid_t pid1 = fork();
		if (pid1 < 0) {
			close(fd1);
			close(fd2);
			perror("fork");
			continue;
		}

		if (pid1 == 0) {
			close(sock1);
			close(sock2);
			proccess(fd1, fd2);
			return EXIT_SUCCESS;
		}

		pid_t pid2 = fork();
		if (pid2 < 0) {
			close(fd1);
			close(fd2);
			kill(pid1, SIGKILL);
			perror("fork");
			continue;
		}
		if (pid2 == 0) {
			close(sock1);
			close(sock2);
			proccess(fd2, fd1);
			return EXIT_SUCCESS;
		}
		close(fd1);
		close(fd2);
	}
	return EXIT_SUCCESS;
}
