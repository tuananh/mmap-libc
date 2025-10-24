#include "mmap_libc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define MAX_FDS 10

typedef struct {
    int in_use;
    int kernel_fd;
    size_t offset;
    char *map_ptr;
} file_state_t;

static file_state_t file_descriptors[MAX_FDS];

int my_open(const char *path, int flags) {
    int idx = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (!file_descriptors[i].in_use) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        errno = ENFILE;
        return -1;
    }

    int kernel_fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (kernel_fd == -1) {
        perror("open failed");
        return -1;
    }

    if (ftruncate(kernel_fd, FILE_SIZE) == -1) {
        perror("ftruncate failed");
        close(kernel_fd);
        return -1;
    }

    char *map_ptr = mmap(
        NULL,
        FILE_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        kernel_fd,
        0
    );

    if (map_ptr == MAP_FAILED) {
        perror("mmap failed");
        close(kernel_fd);
        return -1;
    }

    file_descriptors[idx].in_use = 1;
    file_descriptors[idx].kernel_fd = kernel_fd;
    file_descriptors[idx].offset = 0;
    file_descriptors[idx].map_ptr = map_ptr;

    return idx;
}

long my_lseek(int idx, long offset, int whence) {
    if (idx < 0 || idx >= MAX_FDS || !file_descriptors[idx].in_use) {
        errno = EBADF;
        return -1;
    }

    if (whence != SEEK_SET) {
         errno = EINVAL;
         return -1;
    }

    if (offset < 0 || offset > FILE_SIZE) {
        errno = EINVAL;
        return -1;
    }

    file_descriptors[idx].offset = (size_t)offset;
    return (long)file_descriptors[idx].offset;
}

ssize_t my_read(int idx, void *buf, size_t count) {
    if (idx < 0 || idx >= MAX_FDS || !file_descriptors[idx].in_use) {
        errno = EBADF;
        return -1;
    }

    file_state_t *fs = &file_descriptors[idx];

    size_t remaining = FILE_SIZE - fs->offset;
    size_t bytes_to_read = (count < remaining) ? count : remaining;

    if (bytes_to_read == 0) {
        return 0;
    }

    memcpy(buf, fs->map_ptr + fs->offset, bytes_to_read);

    fs->offset += bytes_to_read;

    return (ssize_t)bytes_to_read;
}

ssize_t my_write(int idx, const void *buf, size_t count) {
    if (idx < 0 || idx >= MAX_FDS || !file_descriptors[idx].in_use) {
        errno = EBADF;
        return -1;
    }

    file_state_t *fs = &file_descriptors[idx];

    size_t remaining = FILE_SIZE - fs->offset;
    size_t bytes_to_write = (count < remaining) ? count : remaining;

    if (bytes_to_write == 0) {
        errno = ENOSPC;
        return -1;
    }

    memcpy(fs->map_ptr + fs->offset, buf, bytes_to_write);

    fs->offset += bytes_to_write;

    return (ssize_t)bytes_to_write;
}

int my_close(int idx) {
    if (idx < 0 || idx >= MAX_FDS || !file_descriptors[idx].in_use) {
        errno = EBADF;
        return -1;
    }

    file_state_t *fs = &file_descriptors[idx];
    int result = 0;

    if (munmap(fs->map_ptr, FILE_SIZE) == -1) {
        perror("munmap failed");
        result = -1;
    }

    if (close(fs->kernel_fd) == -1) {
        perror("close failed");
        result = -1;
    }

    fs->in_use = 0;
    fs->offset = 0;
    fs->kernel_fd = -1;
    fs->map_ptr = NULL;

    return result;
}
