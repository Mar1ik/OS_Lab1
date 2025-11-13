#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/md5.h>
#include <sys/time.h>

#define FRAGMENTS_COUNT 1000
#define FRAGMENT_SIZE 64
#define MAX_TEXT_SIZE (FRAGMENTS_COUNT * FRAGMENT_SIZE)

// Набор текстовых фрагментов для генерации данных
const char *text_fragments[FRAGMENTS_COUNT] = {
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
    // ... остальные фрагменты будут генерироваться автоматически
};

// Инициализация текстовых фрагментов
void init_fragments(char fragments[][FRAGMENT_SIZE + 1]) {
    for (int i = 0; i < FRAGMENTS_COUNT; i++) {
        if (i < 10) {
            strncpy(fragments[i], text_fragments[i], FRAGMENT_SIZE);
            fragments[i][FRAGMENT_SIZE] = '\0';
        } else {
            // Генерируем случайные фрагменты для остальных
            for (int j = 0; j < FRAGMENT_SIZE; j++) {
                fragments[i][j] = 'a' + (rand() % 26);
            }
            fragments[i][FRAGMENT_SIZE] = '\0';
        }
    }
}

// Генерация текста из случайных фрагментов
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

// Вычисление MD5 хеша
void calculate_md5(const char *text, unsigned char *result) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, text, strlen(text));
    MD5_Final(result, &ctx);
}

// Преобразование MD5 в строку
void md5_to_string(unsigned char *md5, char *output) {
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", md5[i]);
    }
    output[32] = '\0';
}

// Получение текущего времени в микросекундах
long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <iterations> [seed]\n", argv[0]);
        fprintf(stderr, "  iterations - number of MD5 calculations to perform\n");
        fprintf(stderr, "  seed       - random seed (optional, default: current time)\n");
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        fprintf(stderr, "Error: iterations must be positive\n");
        return 1;
    }
    
    // Инициализация генератора случайных чисел
    unsigned int seed = (argc >= 3) ? atoi(argv[2]) : (unsigned int)time(NULL);
    srand(seed);
    
    printf("CPU MD5 Calculator\n");
    printf("==================\n");
    printf("Iterations: %d\n", iterations);
    printf("Seed: %u\n", seed);
    printf("Text size: ~%d bytes per iteration\n", MAX_TEXT_SIZE);
    printf("\n");
    
    // Инициализация фрагментов текста
    char (*fragments)[FRAGMENT_SIZE + 1] = malloc(FRAGMENTS_COUNT * (FRAGMENT_SIZE + 1));
    if (fragments == NULL) {
        perror("malloc");
        return 1;
    }
    init_fragments(fragments);
    
    // Буфер для генерируемого текста
    char *text_buffer = malloc(MAX_TEXT_SIZE + 1);
    if (text_buffer == NULL) {
        perror("malloc");
        free(fragments);
        return 1;
    }
    
    // Засекаем время
    long long start_time = get_time_us();
    
    // Основной цикл вычислений
    unsigned long long total_bytes = 0;
    for (int i = 0; i < iterations; i++) {
        // Генерируем случайную длину текста
        int text_length = (rand() % (MAX_TEXT_SIZE / 2)) + (MAX_TEXT_SIZE / 2);
        
        // Генерируем текст
        generate_text(text_buffer, text_length, fragments);
        total_bytes += text_length;
        
        // Вычисляем MD5
        unsigned char md5_result[MD5_DIGEST_LENGTH];
        calculate_md5(text_buffer, md5_result);
        
        // Периодически выводим прогресс
        if ((i + 1) % (iterations / 10 == 0 ? 1 : iterations / 10) == 0 || i == 0) {
            char md5_string[33];
            md5_to_string(md5_result, md5_string);
            printf("Iteration %d/%d: MD5 = %s (text length: %d)\n", 
                   i + 1, iterations, md5_string, text_length);
        }
    }
    
    long long end_time = get_time_us();
    long long elapsed = end_time - start_time;
    
    // Выводим статистику
    printf("\n");
    printf("Results:\n");
    printf("========\n");
    printf("Total iterations: %d\n", iterations);
    printf("Total bytes processed: %llu\n", total_bytes);
    printf("Execution time: %lld.%06lld seconds\n", 
           elapsed / 1000000, elapsed % 1000000);
    printf("Average time per iteration: %.6f seconds\n", 
           (double)elapsed / iterations / 1000000.0);
    printf("Throughput: %.2f MB/s\n", 
           (double)total_bytes / (elapsed / 1000000.0) / (1024.0 * 1024.0));
    
    free(text_buffer);
    free(fragments);
    
    return 0;
}
