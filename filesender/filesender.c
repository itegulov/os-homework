#define _POSIX_C_SOURCE 201505

#include <helpers.h>
#include <bufio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BUF_SIZE 4096
#define LISTEN_CNT 10

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: filesender <port> <file>\n");
		return EXIT_FAILURE;
	}
	struct addrinfo *host;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("localhost", argv[1], &hints, &host)) {
		perror("getaddrinfo");
		return EXIT_FAILURE;
	}

	int sock = socket(host->ai_family, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	int one = 1;

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
		perror("setsockopt");
		return EXIT_FAILURE;
	}

	if(bind(sock, host->ai_addr, host->ai_addrlen)) {
		perror("bind");
		return EXIT_FAILURE;
	}

	if(listen(sock, LISTEN_CNT)) {
		perror("listen");
		return EXIT_FAILURE;
	}

	freeaddrinfo(host);

	while (1) { 
		struct sockaddr_in client;
		socklen_t sz = sizeof(client);
		int fd = accept(sock, (struct sockaddr*)&client, &sz);
		if (fd == -1) {
			perror("accept");
			continue;
		}
		pid_t pid = fork();
		if (pid < 0) {
			perror("fork");
			continue;
		}
		if (pid) {
			close(fd);
		} else {
			close(sock);
			int file_fd = open(argv[2], O_RDONLY);
			buf_t *buf = buf_new(BUF_SIZE);
			if (buf == NULL) {
				perror("buf_new");
				return EXIT_FAILURE;
			}

			while (1) {
				int off = buf_fill(file_fd, buf, 1);
				if (off < 0) {
					buf_flush(fd, buf, buf_size(buf));
					return EXIT_FAILURE;
				} else if (off == 0) return EXIT_SUCCESS;

				if (buf_flush(fd, buf, buf_size(buf)) < 0)
					return EXIT_FAILURE;
			}
		}
	}
	return EXIT_SUCCESS;
}
