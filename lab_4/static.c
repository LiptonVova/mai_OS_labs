#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dlfcn.h>
#include <unistd.h>

int prime_count(int a, int b);
char *convert(int x);

int main() {
    const char info[] = "1 start_range end_range - count prime numbers\n2 number - convert number from 10 c/c\n";
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

    char *endptr;
    int choice = strtol(token, &endptr, 10);
    if (*endptr != '\0') {
        const char msg[] = "-- Error: invalid command\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        return -1;
    }

    while (choice > 0 && choice < 3) {
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

            int result = prime_count(start_range, end_range);
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

            char *result = convert(number);
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
}