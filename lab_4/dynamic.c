#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dlfcn.h>
#include <unistd.h>

typedef int prime_count_func(int , int);
typedef char* convert_func(int);

static prime_count_func* f_prime_count;
static convert_func* f_convert;

void print_usage(char *name) {
    char msg[1024];
    sprintf(msg, "%s [n] [path to library_1] [path to library_2] ... [path to library_n]\n", name);
    write(STDERR_FILENO, msg, strlen(msg));
}

int validate_arguments(int argc, char**argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return -1;
    }
    char *endptr;
    int n = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        print_usage(argv[0]);
        return -1;
    }

    if (n != argc - 2) {
        print_usage(argv[0]);
        return -1;
    }

    if (n < 1) {
        print_usage(argv[0]);
        return -1;
    }

    return 0;
}


int main(int argc, char**argv) {
    int is_validate = validate_arguments(argc, argv);
    if (is_validate) {
        const char msg[] = "-- Error: invalid arguments\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        return -1;
    }

    char* endptr;
    const int n = strtol(argv[1], &endptr, 10);
    char **paths_to_libraries = (char**)malloc(sizeof(char**) * n);
    if (!paths_to_libraries) {
        const char msg[] = "-- Error: fail to alloc memory\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        return -1;
    }
    for (int i = 0; i < n; ++i) {
        paths_to_libraries[i] = argv[i + 2];
    }

    int index_cur_library = 0;
    void *library = dlopen(paths_to_libraries[index_cur_library], RTLD_LOCAL | RTLD_LAZY);
    if (library == NULL) {
        const char msg[] = "-- Error: error while open library\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        return -1;
    }
    
    const char info[] = "0 - change library\n1 start_range end_range - count prime numbers\n2 number - convert number from 10 c/c\n";
    write(STDOUT_FILENO, info, sizeof(info));

    const char enter_info[] = "Please, enter the command: \n";
    write(STDOUT_FILENO, enter_info, sizeof(enter_info));

    char buffer[1024];
    size_t bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (bytes == -1) {
        const char msg[] = "-- Error: error while read bytes\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        return -1;
    }

    buffer[bytes - 1] = '\0';

    char *token;
    token = strtok(buffer, " ");

    int choice = strtol(token, &endptr, 10);
    if (*endptr != '\0') {
        const char msg[] = "-- Error: invalid command\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        return -1;
    }

    while (choice >= 0 && choice < 3) {
        if (choice == 0) {
            dlclose(library);
            index_cur_library = (index_cur_library + 1) % n;
            library = dlopen(paths_to_libraries[index_cur_library], RTLD_LOCAL | RTLD_LAZY);
            if (library == NULL) {
                const char msg[] = "-- Error: error while change library\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }
            const char msg[] = "-- Info: library successfully changed\n";
            write(STDOUT_FILENO, msg, sizeof(msg));
        }

        if (choice == 1) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                const char msg[] = "-- Error: invalid arguments for function 1 (search prime numbers)\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }

            int start_range = strtol(token, &endptr, 10);
            if (*endptr != '\0') {
                const char msg[] = "-- Error: invalid arguments for function 1 (search prime numbers)\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }

            token = strtok(NULL, " ");
            if (token == NULL) {
                const char msg[] = "-- Error: invalid arguments for function 1 (search prime numbers)\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }

            int end_range = strtol(token, &endptr, 10);
            if (*endptr != '\0') {
                const char msg[] = "-- Error: invalid arguments for function 1 (search prime numbers)\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }

            f_prime_count = dlsym(library, "prime_count");
            if (f_prime_count == NULL) {
                const char msg[] = "-- Error: failed to find prime_count implementation\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }
            int result = f_prime_count(start_range, end_range);
            char msg[1024];
            sprintf(msg, "result: %d\n", result);
            write(STDOUT_FILENO, msg, strlen(msg));
        }

        if (choice == 2) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                const char msg[] = "-- Error: invalid arguments for function 1 (search prime numbers)\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }

            int number = strtol(token, &endptr, 10);
            if (*endptr != '\0') {
                const char msg[] = "-- Error: invalid arguments for function 1 (search prime numbers)\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }

            f_convert = dlsym(library, "convert");
            if (f_convert == NULL) {
                const char msg[] = "-- Error: failed to find convert implementation\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                return -1;
            }
            char *result = f_convert(number);
            char msg[1024];
            sprintf(msg, "result: %s\n", result);
            write(STDOUT_FILENO, msg, strlen(msg));
            free(result);
        }

        write(STDOUT_FILENO, enter_info, sizeof(enter_info));
        bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes == -1) {
            const char msg[] = "-- Error: error while read bytes\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            return -1;
        }
        buffer[bytes - 1] = '\0';
        token = strtok(buffer, " ");
        choice = strtol(token, &endptr, 10);
    }

    const char msg[] = "-- Info: programm sucessfully closed\n";
    write(STDOUT_FILENO, msg, sizeof(msg));
    dlclose(library);
}