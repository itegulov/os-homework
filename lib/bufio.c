#include "bufio.h"
#include "stdlib.h"
#include "string.h"

struct buf_t *buf_new(size_t capacity) {
	buf_t *rv = (buf_t *) malloc(sizeof(size_t) * 2 + capacity);
	if (rv == NULL) return NULL;
	rv->size = 0;
	rv->capacity = capacity;
	return rv;
}

void buf_free(struct buf_t *buf) {
	free(buf);
}

size_t buf_capacity(buf_t *buf) {
	#ifdef DEBUG
	if (buf == NULL) abort();
	#endif
	return buf->capacity;
}

size_t buf_size(buf_t *buf) {
	#ifdef DEBUG
	if (buf == NULL) abort();
	#endif
	return buf->size;
}

ssize_t buf_fill(fd_t fd, buf_t *buf, size_t required) {
	#ifdef DEBUG
	if (buf == NULL) abort();
	#endif
	char *chars = (char *) (buf + 2 * sizeof(size_t));
	ssize_t res = 0;
	do {
		res = read(fd, chars + buf->size, buf->capacity - buf->size);
		if (res < 0) return -1;
		buf->size += res;
	} while (buf->size < required && res > 0);
	return buf->size;
}

ssize_t buf_flush(fd_t fd, buf_t *buf, size_t required) {
	#ifdef DEBUG
	if (buf == NULL) abort();
	#endif
	char *chars = (char *) (buf + 2 * sizeof(size_t));

	size_t offset = 0;
	while (buf->size > 0 && offset < required) {
		ssize_t res = write(fd, chars + offset, buf->size - offset);
		if (res < 0) {
			memmove(chars, chars + offset, buf->size - offset);
			buf->size -= offset;
			return -1;
		}
		offset += res;
	}
	memmove(chars, chars + offset, buf->size - offset);
	buf->size -= offset;
	return offset;
}
