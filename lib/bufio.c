#include "bufio.h"
#include "stdlib.h"
#include "string.h"

struct buf_t *buf_new(size_t capacity) {
	buf_t *buf = (buf_t *) malloc(sizeof(buf_t));
	if (buf == NULL) return NULL;
	buf->chars = (char *) malloc(capacity);
	if (buf->chars == NULL) {
		free(buf);
		return NULL;
	}
	buf->size = 0;
	buf->capacity = capacity;
	return buf;
}

void buf_free(struct buf_t *buf) {
	free(buf->chars);
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
	char *chars = buf->chars;
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
	char *chars = buf->chars;

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
