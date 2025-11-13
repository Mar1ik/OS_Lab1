#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <openssl/md5.h>
#include <sys/time.h>
#include <unistd.h>

#define FRAGMENTS_COUNT 1000
#define FRAGMENT_SIZE 64
#define MAX_TEXT_SIZE (FRAGMENTS_COUNT * FRAGMENT_SIZE)

// Структура для передачи данных в поток
typedef struct {
    int thread_id;
    int iterations;
    unsigned int seed;
    unsigned long long bytes_processed;
    long long start_time;
    long long end_time;
} thread_data_t;

const char *text_fragments[] = {
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do",
    "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut ",
    "enim ad minim veniam, quis nostrud exercitation ullamco laboris",
    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor ",
    "in reprehenderit in voluptate velit esse cillum dolore eu fugia",
    "Excepteur sint occaecat cupidatat non proident, sunt in culpa q",
    "officia deserunt mollit anim id est laborum. Sed ut perspiciati",
    "unde omnis iste natus error sit voluptatem accusantium doloremq",
    "laudantium totam rem aperiam eaque ipsa quae ab illo inventore ",
    "veritatis et quasi architecto beatae vitae dicta sunt explicabo"
};

void init_fragments(char fragments[][FRAGMENT_SIZE + 1], unsigned int seed) {
    srand(seed);
    for (int i = 0; i < FRAGMENTS_COUNT; i++) {
        if (i < 10) {
            strncpy(fragments[i], text_fragments[i], FRAGMENT_SIZE);
            fragments[i][FRAGMENT_SIZE] = '\0';
        } else {
            for (int j = 0; j < FRAGMENT_SIZE; j++) {
                fragments[i][j] = 'a' + (rand() % 26);
            }
            fragments[i][FRAGMENT_SIZE] = '\0';
        }
    }
}

void generate_text(char *buffer, int length, char fragments[][FRAGMENT_SIZE + 1]) {
    int pos = 0;
    while (pos < length) {
        int fragment_idx = rand() % FRAGMENTS_COUNT;
        int to_copy = FRAGMENT_SIZE;
        if (pos + to_copy > length) {
            to_copy = length - pos;
        }
        memcpy(buffer + pos, fragments[fragment_idx], to_copy);
        pos += to_copy;
    }
    buffer[length] = '\0';
}

void calculate_md5(const char *text, unsigned char *result) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, text, strlen(text));
    MD5_Final(result, &ctx);
}

long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

void *worker_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    
    // Инициализация с уникальным seed для каждого потока
    srand(data->seed);
    
    char (*fragments)[FRAGMENT_SIZE + 1] = malloc(FRAGMENTS_COUNT * (FRAGMENT_SIZE + 1));
    if (fragments == NULL) {
        perror("malloc");
        return NULL;
    }
    init_fragments(fragments, data->seed);
    
    char *text_buffer = malloc(MAX_TEXT_SIZE + 1);
    if (text_buffer == NULL) {
        perror("malloc");
        free(fragments);
        return NULL;
    }
    
    data->start_time = get_time_us();
    data->bytes_processed = 0;
    
    for (int i = 0; i < data->iterations; i++) {
        int text_length = (rand() % (MAX_TEXT_SIZE / 2)) + (MAX_TEXT_SIZE / 2);
        generate_text(text_buffer, text_length, fragments);
        data->bytes_processed += text_length;
        
        unsigned char md5_result[MD5_DIGEST_LENGTH];
        calculate_md5(text_buffer, md5_result);
        
        if ((i + 1) % (data->iterations / 5 == 0 ? 1 : data->iterations / 5) == 0) {
            printf("Thread %d: iteration %d/%d\n", 
                   data->thread_id, i + 1, data->iterations);
        }
    }
    
    data->end_time = get_time_us();
    
    free(text_buffer);
    free(fragments);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations> [threads] [seed]\n", argv[0]);
        fprintf(stderr, "  iterations - number of MD5 calculations per thread\n");
        fprintf(stderr, "  threads    - number of threads (default: CPU count)\n");
        fprintf(stderr, "  seed       - random seed (optional, default: current time)\n");
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    int num_threads = (argc >= 3) ? atoi(argv[2]) : sysconf(_SC_NPROCESSORS_ONLN);
    unsigned int seed = (argc >= 4) ? atoi(argv[3]) : (unsigned int)time(NULL);
    
    if (iterations <= 0 || num_threads <= 0) {
        fprintf(stderr, "Error: iterations and threads must be positive\n");
        return 1;
    }
    
    printf("Multi-threaded CPU MD5 Calculator\n");
    printf("==================================\n");
    printf("Iterations per thread: %d\n", iterations);
    printf("Number of threads: %d\n", num_threads);
    printf("Total iterations: %d\n", iterations * num_threads);
    printf("Seed: %u\n", seed);
    printf("\n");
    
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(num_threads * sizeof(thread_data_t));
    
    if (threads == NULL || thread_data == NULL) {
        perror("malloc");
        return 1;
    }
    
    long long total_start = get_time_us();
    
    // Создаем потоки
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations;
        thread_data[i].seed = seed + i;
        
        if (pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Ждем завершения потоков
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    long long total_end = get_time_us();
    long long total_elapsed = total_end - total_start;
    
    // Собираем статистику
    unsigned long long total_bytes = 0;
    for (int i = 0; i < num_threads; i++) {
        total_bytes += thread_data[i].bytes_processed;
    }
    
    printf("\n");
    printf("Results:\n");
    printf("========\n");
    printf("Total iterations: %d\n", iterations * num_threads);
    printf("Total bytes processed: %llu\n", total_bytes);
    printf("Total execution time: %lld.%06lld seconds\n", 
           total_elapsed / 1000000, total_elapsed % 1000000);
    printf("Average time per iteration: %.6f seconds\n", 
           (double)total_elapsed / (iterations * num_threads) / 1000000.0);
    printf("Throughput: %.2f MB/s\n", 
           (double)total_bytes / (total_elapsed / 1000000.0) / (1024.0 * 1024.0));
    
    free(threads);
    free(thread_data);
    
    return 0;
}