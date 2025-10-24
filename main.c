#include "mmap_libc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    printf("open file: %s (size %d bytes)\n", TEMP_FILE_NAME, FILE_SIZE);

    int fd = my_open(TEMP_FILE_NAME, 0);
    if (fd < 0) {
        fprintf(stderr, "fatal: my_open failed to create/map the file.\n");
        return 1;
    }
    printf("opened and mapped file, fd = %d\n\n", fd);

    char init_buf[1024];
    memset(init_buf, 'Z', 1024);
    
    my_lseek(fd, 0, 0);
    for (int i = 0; i < FILE_SIZE / 1024; i++) {
        my_write(fd, init_buf, 1024);
    }
    printf("content initialized to 'Z' (via my_write).\n");

    const char *write_data = "TESSTTTT!";
    size_t write_len = strlen(write_data);
    
    my_lseek(fd, 500, 0);

    ssize_t bytes_written = my_write(fd, write_data, write_len);

    if (bytes_written != (ssize_t)write_len) {
        printf("error: expected %zu bytes written, got %zd\n", write_len, bytes_written);
    } else {
        printf("wrote '%s' (size %zd) to fd %d at offset 500.\n",
               write_data, bytes_written, fd);
    }

    my_lseek(fd, 500, 0);

    char read_buf[50] = {0};
    ssize_t bytes_read = my_read(fd, read_buf, write_len); 

    if (bytes_read > 0) {
        read_buf[bytes_read] = '\0';
        printf("read back data: '%s'\n\n", read_buf);
    } else {
        printf("error: reading data.\n");
    }
    
    if (my_close(fd) == 0) {
        printf("unmapped and closed fd %d.\n", fd);
    }

    if (unlink(TEMP_FILE_NAME) == 0) {
        printf("removed temporary file: %s\n", TEMP_FILE_NAME);
    } else {
        perror("warn: could not remove temporary file");
    }

    return 0;
}
