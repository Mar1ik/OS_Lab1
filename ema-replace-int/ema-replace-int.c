#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFFER_SIZE (4096)  // Размер буфера для чтения

// Получение текущего времени в микросекундах
long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

// Поиск и замена значения в файле
int replace_in_file(const char *filename, int search_value, int replace_value, 
                    unsigned long long *matches_found, unsigned long long *bytes_read) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        return -1;
    }
    
    int *buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("malloc");
        close(fd);
        return -1;
    }
    
    *matches_found = 0;
    *bytes_read = 0;
    off_t position = 0;
    
    while (1) {
        // Читаем блок данных
        ssize_t bytes = read(fd, buffer, BUFFER_SIZE);
        if (bytes == -1) {
            perror("read");
            free(buffer);
            close(fd);
            return -1;
        }
        
        if (bytes == 0) {
            break;  // Конец файла
        }
        
        *bytes_read += bytes;
        int num_ints = bytes / sizeof(int);
        
        // Ищем и заменяем значения
        int found_in_block = 0;
        for (int i = 0; i < num_ints; i++) {
            if (buffer[i] == search_value) {
                buffer[i] = replace_value;
                (*matches_found)++;
                found_in_block = 1;
            }
        }
        
        // Если нашли совпадения, записываем блок обратно
        if (found_in_block) {
            if (lseek(fd, position, SEEK_SET) == -1) {
                perror("lseek");
                free(buffer);
                close(fd);
                return -1;
            }
            
            ssize_t written = write(fd, buffer, bytes);
            if (written == -1) {
                perror("write");
                free(buffer);
                close(fd);
                return -1;
            }
            
            if (written != bytes) {
                fprintf(stderr, "Error: incomplete write\n");
                free(buffer);
                close(fd);
                return -1;
            }
            
            // Возвращаемся на следующую позицию для чтения
            if (lseek(fd, position + bytes, SEEK_SET) == -1) {
                perror("lseek");
                free(buffer);
                close(fd);
                return -1;
            }
        }
        
        position += bytes;
    }
    
    free(buffer);
    close(fd);
    return 0;
}

// Получение размера файла
off_t get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("stat");
        return -1;
    }
    return st.st_size;
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <file> <size_mb> <search_value> <replace_value> <iterations>\n", argv[0]);
        fprintf(stderr, "  file          - path to the data file\n");
        fprintf(stderr, "  size_mb       - size of file in MB (for new file creation)\n");
        fprintf(stderr, "  search_value  - integer value to search for\n");
        fprintf(stderr, "  replace_value - integer value to replace with\n");
        fprintf(stderr, "  iterations    - number of search-replace iterations\n");
        return 1;
    }
    
    const char *filename = argv[1];
    int size_mb = atoi(argv[2]);
    int search_value = atoi(argv[3]);
    int replace_value = atoi(argv[4]);
    int iterations = atoi(argv[5]);
    
    if (size_mb <= 0 || iterations <= 0) {
        fprintf(stderr, "Error: size_mb and iterations must be positive\n");
        return 1;
    }
    
    printf("EMA Replace Integer\n");
    printf("===================\n");
    printf("File: %s\n", filename);
    printf("Search value: %d\n", search_value);
    printf("Replace value: %d\n", replace_value);
    printf("Iterations: %d\n", iterations);
    printf("\n");
    
    // Проверяем существование файла
    if (access(filename, F_OK) != 0) {
        printf("File does not exist. Creating new file with size %d MB...\n", size_mb);
        
        // Создаем файл
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
        
        // Заполняем файл случайными числами
        srand(time(NULL));
        unsigned long long total_bytes = (unsigned long long)size_mb * 1024 * 1024;
        unsigned long long written_bytes = 0;
        int search_count = 0;
        int target_searches = (total_bytes / sizeof(int)) / 100;  // 1% значений
        
        while (written_bytes < total_bytes) {
            int num_ints = BUFFER_SIZE / sizeof(int);
            for (int i = 0; i < num_ints; i++) {
                // Вставляем искомое значение с определенной вероятностью
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
            
            if (written_bytes % (1024 * 1024 * 10) == 0) {
                printf("Generated: %llu MB\n", written_bytes / (1024 * 1024));
            }
        }
        
        printf("File created successfully. Inserted ~%d search values.\n\n", search_count);
        free(buffer);
        close(fd);
    }
    
    // Получаем размер файла
    off_t file_size = get_file_size(filename);
    if (file_size == -1) {
        return 1;
    }
    
    printf("File size: %.2f MB (%lld bytes)\n", 
           (double)file_size / (1024.0 * 1024.0), (long long)file_size);
    printf("\n");
    
    // Выполняем поиск и замену
    long long start_time = get_time_us();
    unsigned long long total_matches = 0;
    unsigned long long total_bytes = 0;
    
    for (int i = 0; i < iterations; i++) {
        unsigned long long matches = 0;
        unsigned long long bytes = 0;
        
        if (replace_in_file(filename, search_value, replace_value, &matches, &bytes) == -1) {
            return 1;
        }
        
        total_matches += matches;
        total_bytes += bytes;
        
        printf("Iteration %d/%d: found and replaced %llu values (read %llu bytes)\n", 
               i + 1, iterations, matches, bytes);
        
        // После первой итерации все значения заменены, поэтому меняем поиск/замену местами
        if (i == 0) {
            int temp = search_value;
            search_value = replace_value;
            replace_value = temp;
        }
    }
    
    long long end_time = get_time_us();
    long long elapsed = end_time - start_time;
    
    // Статистика
    printf("\n");
    printf("Results:\n");
    printf("========\n");
    printf("Total iterations: %d\n", iterations);
    printf("Total matches found and replaced: %llu\n", total_matches);
    printf("Total bytes read: %llu (%.2f MB)\n", 
           total_bytes, (double)total_bytes / (1024.0 * 1024.0));
    printf("Execution time: %lld.%06lld seconds\n", 
           elapsed / 1000000, elapsed % 1000000);
    printf("Average time per iteration: %.6f seconds\n", 
           (double)elapsed / iterations / 1000000.0);
    printf("Read throughput: %.2f MB/s\n", 
           (double)total_bytes / (elapsed / 1000000.0) / (1024.0 * 1024.0));
    
    return 0;
}
