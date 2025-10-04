#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>

#include <errno.h>

#include <time.h>

typedef struct {
    size_t number;
    double R;
    int count_iterations;
    unsigned int seed;
} ThreadArgs;

static volatile int32_t count_in_circle = 0;
static volatile int32_t count_total = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *work(void *_args) {
    ThreadArgs *args = (ThreadArgs *)_args;
    int n = args->count_iterations;
    double x, y;
    int local_count_in_circle = 0;

    unsigned int seed = args->seed;

    for (size_t i = 0; i < n; ++i) {
        x = (double)rand_r(&seed) / RAND_MAX * args->R;
        y = (double)rand_r(&seed) / RAND_MAX * args->R;


        if (x * x + y * y <= args->R * args->R) {
            ++local_count_in_circle;
        }
    }

    pthread_mutex_lock(&mutex);
    count_total += n;
    count_in_circle += local_count_in_circle;
    pthread_mutex_unlock(&mutex);

    // printf("Threads #%ld says count_in_circle: %d, count_total: %d\n", args->number, count_in_circle, count_total);

    return NULL;
}

void print_usage(const char* program_name) {
    printf("usage: %s --threads <number>\n", program_name);
    printf("       %s -t <number>\n", program_name);
}

int validate_flags(size_t *n_threads, int argc, char* argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--threads") == 0 || strcmp(argv[1], "-t") == 0) {
        char* endptr;
        errno = 0;
        *n_threads = strtoll(argv[2], &endptr, 10);

        if (errno == ERANGE) {
            printf("Произошло переполнение\n");
            print_usage(argv[0]);
            return 1;
        }
        else if (endptr == argv[2] || *endptr != '\0') {
            printf("Введите число\n");
            print_usage(argv[0]);
            return 1;
        }
    }
    else {
        print_usage(argv[0]);
        return 1;
    }
}


int main(int argc, char* argv[]) {
    int count = 1000000000;

    size_t n_threads = 0;
    int flag = validate_flags(&n_threads, argc, argv);
    if (flag) {
        return 1;
    }

    // --n_threads; // один поток выполняет main

    double R;
    printf("Введите радиус окружности: ");
    scanf("%lf", &R);


    pthread_t *threads = malloc(n_threads * sizeof(pthread_t));
    ThreadArgs *thread_args = malloc(n_threads * sizeof(ThreadArgs));

    struct timespec start;

    clock_gettime(CLOCK_MONOTONIC, &start);


    for (size_t i = 0; i < n_threads; ++i) {
        thread_args[i] = (ThreadArgs) {
            .number = i,
            .R = R,
            .count_iterations = count / n_threads,
            .seed = rand(),
        };

        pthread_create(&threads[i], NULL, work, &thread_args[i]);
    }


    for (size_t i = 0; i < n_threads; ++i) {
        pthread_join(threads[i], NULL);
    }


    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    double square = 4 * (count_in_circle/(double)count_total) * R * R;
    printf("***\nSquare circle: %lf\n", square);
    printf("Working time: %lfs\n", time);
    printf("Count threads: %ld\n", n_threads);
    printf("***\n");

    free(thread_args);
    free(threads);

    return 0;
}