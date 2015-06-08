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
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, argv[1], &hints, &host)) {
		perror("getaddrinfo");
		return EXIT_FAILURE;
	}

	struct addrinfo *rp;
	int sock;

	for (rp = host; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, SOCK_STREAM, IPPROTO_TCP);
		if(sock < 0) {
			continue;
		}

		int one = 1;

		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
			close(sock);
			continue;
		}

		if(bind(sock, rp->ai_addr, rp->ai_addrlen)) {
			close(sock);
			continue;
		}
		
		break;
	}

	if (rp == NULL) {
		printf("couldn't bind to address");
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
