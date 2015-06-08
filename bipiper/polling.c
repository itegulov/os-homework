#define _POSIX_C_SOURCE 201505
#define _GNU_SOURCE

#include <bufio.h>
#include <helpers.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <stdio.h>

#define BUF_SIZE 4096
#define LISTEN_CNT 10
#define CONNECT_CNT 127

struct pollfd pfd[2 + CONNECT_CNT * 2];
buf_t* pbuf[CONNECT_CNT * 2];
int pfd_cnt = 0;
int current = 0;

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

void close_pair(int i) {
	if (pfd_cnt == 2 * CONNECT_CNT)
		pfd[current].events = POLLIN;
	close(pfd[i + 2].fd);
	close(pfd[(i + 2) ^ 1].fd);
	buf_free(pbuf[i]);
	buf_free(pbuf[i ^ 1]);
	if (pfd_cnt > 3) {
		int fi = i & ~1;
		pfd[fi + 2] = pfd[pfd_cnt];
		pfd[fi + 3] = pfd[pfd_cnt + 1];
		pbuf[fi] = pbuf[pfd_cnt - 2];
		pbuf[fi + 1] = pbuf[pfd_cnt - 1];
	}
	pfd_cnt -= 2;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: bipiper <port1> <port2>");
		return EXIT_FAILURE;
	}

	signal(SIGPIPE, SIG_IGN);

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
	if (getaddrinfo(NULL, argv[2], &hints, &host2)) {
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
	
	pfd[0].fd = sock1;
	pfd[1].fd = sock2;

	pfd[0].events = POLLIN;

	int temp_fd = -1;

	while (1) {
		int cnt = poll(pfd, pfd_cnt + 2, -1);
		if (cnt < 0) {
			perror("poll");
			continue;
		}
		short ev = pfd[current].revents;
		if (ev) {
			if (ev & POLLIN) {
				pfd[current].events = 0;	
				pfd[current].revents = 0;	
				int new_fd = accept(pfd[current].fd, 0, 0);
				if (new_fd < 0) {
					perror("accept");
					continue;
				}
				if (current) {
					pbuf[pfd_cnt] = buf_new(BUF_SIZE);
					pbuf[pfd_cnt + 1] = buf_new(BUF_SIZE);
					pfd[pfd_cnt + 2].events = POLLIN;
					pfd[pfd_cnt + 3].events = POLLIN;
					pfd[pfd_cnt + 2].fd = temp_fd;
					pfd[pfd_cnt + 3].fd = new_fd;
					temp_fd = -1;
					pfd_cnt += 2;
				} else {
					temp_fd = new_fd;
				}
				current ^= 1;
				if (pfd_cnt < 2 * CONNECT_CNT)
					pfd[current].events = POLLIN;
			}
		}
		for (int i = 0; i < pfd_cnt; i++) {
			short ev = pfd[i + 2].revents;
			pfd[i + 2].revents = 0;
			if (ev) {
				if (ev & POLLOUT) {
					size_t old = buf_size(pbuf[i ^ 1]);
					if (buf_flush(pfd[i + 2].fd, pbuf[i ^ 1], 1) < 0) {
						close_pair(i);
						i = (i & ~1) - 2;
						continue;
					}
					if (buf_size(pbuf[i ^ 1]) == 0) // Don't want to write anymore
						pfd[i + 2].events &= ~POLLOUT;
					if (old == buf_capacity(pbuf[i ^ 1]) && buf_size(pbuf[i ^ 1]) < buf_capacity(pbuf[i ^ 1])) // Some space available now
						pfd[(i + 2) ^ 1].events |= POLLIN;
				}
				if (ev & POLLIN) {
					size_t old = buf_size(pbuf[i]);	
					if (buf_fill(pfd[i + 2].fd, pbuf[i], buf_size(pbuf[i]) + 1) < 0) {
						close_pair(i);
						i = (i & ~1) - 2;
						continue;
					}
					if (buf_size(pbuf[i]) == buf_capacity(pbuf[i])) // No more space
						pfd[i + 2].events &= ~POLLIN;
					if (old == 0 && buf_size(pbuf[i]) > 0) // Have some data to write now
						pfd[(i + 2) ^ 1].events |= POLLOUT;
				}
				if (ev & POLLERR) {
					i |= 1;
					close_pair(i);
					i = (i & ~1) - 2;
				}
			}
		}
	}
}
