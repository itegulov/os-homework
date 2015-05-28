#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>

ssize_t read_(int fd, void *buf, size_t count);
ssize_t write_(int fd, const void *buf, size_t count);
ssize_t read_until(int fd, void *buf, size_t count, char delimiter);
int spawn(const char * file, char * const argv []);

typedef char** execargs_t;

execargs_t new_execargs(char **args);
void free_execargs(execargs_t e);

int exec(execargs_t* args);

int runpiped(execargs_t** args, size_t n);

#endif
