#ifndef MMAP_LIBC_H
#define MMAP_LIBC_H

#include <stddef.h>
#include <sys/types.h>

#define FILE_SIZE 4096
#define TEMP_FILE_NAME "simulated_data.tmp"

// The minimal libc interface
int my_open(const char *path, int flags);
long my_lseek(int idx, long offset, int whence);
ssize_t my_read(int idx, void *buf, size_t count);
ssize_t my_write(int idx, const void *buf, size_t count);
int my_close(int idx);

#endif // MMAP_LIBC_H
