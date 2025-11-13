#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/md5.h>
#include <sys/time.h>
#include <pthread.h>

// Multithreaded variant: simple example spawning worker threads that compute MD5
// NOTE: This file mirrors the single-threaded generator but distributes
// iterations across threads. Link with -lpthread -lcrypto.

typedef struct {
    int id;
    int iterations;
    unsigned int seed;
} worker_arg_t;

long long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

void *worker(void *arg) {
    worker_arg_t *w = (worker_arg_t *)arg;
    unsigned int seed = w->seed + w->id;
    srand(seed);

    for (int i = 0; i < w->iterations; i++) {
        // Simple small payload per iteration
        char buf[128];
        int len = snprintf(buf, sizeof(buf), "thread-%d-iter-%d-%u", w->id, i, rand());
        unsigned char md5[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)buf, len, md5);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <total_iterations> <threads> [seed]\n", argv[0]);
        return 1;
    }

    int total_iterations = atoi(argv[1]);
    int threads = atoi(argv[2]);
    unsigned int seed = (argc >= 4) ? atoi(argv[3]) : (unsigned int)time(NULL);

    if (total_iterations <= 0 || threads <= 0) {
        fprintf(stderr, "total_iterations and threads must be positive\n");
        return 1;
    }

    pthread_t *tids = malloc(sizeof(pthread_t) * threads);
    worker_arg_t *args = malloc(sizeof(worker_arg_t) * threads);

    int per_thread = total_iterations / threads;
    int remainder = total_iterations % threads;

    long long start = get_time_us();

    for (int i = 0; i < threads; i++) {
        args[i].id = i;
        args[i].iterations = per_thread + (i < remainder ? 1 : 0);
        args[i].seed = seed;
        pthread_create(&tids[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(tids[i], NULL);
    }

    long long end = get_time_us();
    long long elapsed = end - start;
    printf("Completed %d iterations on %d threads in %lld.%06lld seconds\n",
           total_iterations, threads, elapsed / 1000000, elapsed % 1000000);

    free(tids);
    free(args);
    return 0;
}
