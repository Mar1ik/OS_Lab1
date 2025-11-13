#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE (4096)

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <file> <size_mb> <seed>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int size_mb = atoi(argv[2]);
    unsigned int seed = (unsigned int)atoi(argv[3]);

    if (size_mb <= 0) {
        fprintf(stderr, "size_mb must be positive\n");
        return 1;
    }

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    int *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("malloc");
        close(fd);
        return 1;
    }

    srand(seed);
    unsigned long long total_bytes = (unsigned long long)size_mb * 1024 * 1024;
    unsigned long long written = 0;

    while (written < total_bytes) {
        int num_ints = BUFFER_SIZE / sizeof(int);
        for (int i = 0; i < num_ints; i++) {
            buffer[i] = rand();
        }

        ssize_t to_write = BUFFER_SIZE;
        if (written + to_write > total_bytes) to_write = total_bytes - written;
        ssize_t res = write(fd, buffer, to_write);
        if (res == -1) {
            perror("write");
            free(buffer);
            close(fd);
            return 1;
        }
        written += res;
    }

    free(buffer);
    close(fd);
    printf("Generated %llu bytes into %s\n", (unsigned long long)written, filename);
    return 0;
}
