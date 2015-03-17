#include "helpers.h"

ssize_t read_(int fd, void *buf, size_t count) {
    size_t cnt = 0;
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
    for (size_t offset = 0;;offset++) {
        cnt = read(fd, buf + offset, 1);
        if (cnt == -1)
            return -1;
        if (cnt == 0)
            return 0;
        if (chars[offset] == delimiter || offset + 1 == count)
            return count;
    }
}
