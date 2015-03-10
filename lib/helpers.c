#include "helpers.h"

ssize_t read_(int fd, void *buf, size_t count) {
    int cnt = 0;
    int offset = 0;
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
    int cnt = 0;
    int offset = 0;
    do {
        cnt = write(fd, buf + offset, count);
        if (cnt == -1) 
            return -1;
        count -= cnt;
        offset += cnt;
    } while (cnt > 0);
    return offset;
}
