#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE (4096)

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <filename> <size_mb> [search_value]\n", argv[0]);
        fprintf(stderr, "  filename     - output file name\n");
        fprintf(stderr, "  size_mb      - size in megabytes\n");
        fprintf(stderr, "  search_value - value to insert (optional, 1%% of values)\n");
        return 1;
    }
    
    const char *filename = argv[1];
    int size_mb = atoi(argv[2]);
    int search_value = (argc >= 4) ? atoi(argv[3]) : 42;
    
    if (size_mb <= 0) {
        fprintf(stderr, "Error: size_mb must be positive\n");
        return 1;
    }
    
    printf("Generating data file: %s\n", filename);
    printf("Size: %d MB\n", size_mb);
    printf("Search value: %d\n", search_value);
    
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    
    int *buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("malloc");
        close(fd);
        return 1;
    }
    
    srand(time(NULL));
    unsigned long long total_bytes = (unsigned long long)size_mb * 1024 * 1024;
    unsigned long long written_bytes = 0;
    int search_count = 0;
    int target_searches = (total_bytes / sizeof(int)) / 100;
    
    while (written_bytes < total_bytes) {
        int num_ints = BUFFER_SIZE / sizeof(int);
        for (int i = 0; i < num_ints; i++) {
            if (search_count < target_searches && (rand() % 100) == 0) {
                buffer[i] = search_value;
                search_count++;
            } else {
                buffer[i] = rand();
            }
        }
        
        ssize_t to_write = BUFFER_SIZE;
        if (written_bytes + to_write > total_bytes) {
            to_write = total_bytes - written_bytes;
        }
        
        ssize_t result = write(fd, buffer, to_write);
        if (result == -1) {
            perror("write");
            free(buffer);
            close(fd);
            return 1;
        }
        
        written_bytes += result;
        
        if (written_bytes % (1024 * 1024 * 10) == 0 || written_bytes == total_bytes) {
            printf("Progress: %llu / %llu MB\r", 
                   written_bytes / (1024 * 1024), 
                   total_bytes / (1024 * 1024));
            fflush(stdout);
        }
    }
    
    printf("\nFile created successfully!\n");
    printf("Inserted %d search values (~1%% of data)\n", search_count);
    
    free(buffer);
    close(fd);
    
    return 0;
}