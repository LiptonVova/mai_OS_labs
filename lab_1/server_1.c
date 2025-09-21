#include <stdint.h>
#include <ctype.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


int main(int argc, char * argv[]) {
    char buf[4096];
    ssize_t bytes;

    pid_t pid = getpid();

    int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0600);
    if (file == -1) {
        const char msg[] = "error: failed to open requested file\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
        if (bytes < 0) {
            const char msg[] = "error: failed to read stdin\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }

        for (uint32_t i = 0; i < bytes; ++i) {
            buf[i] = toupper(buf[i]);
        }

        {
            char log_msg[4200];
            uint32_t log_length = snprintf(log_msg, sizeof(log_msg), "(Child 1)PID %d: %s", pid, buf);

            // записываем в логи 
            uint32_t written = write(file, log_msg, log_length);
            if (written != log_length) {
                const char msg[] = "error: failed to write in file\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }

            written = write(STDOUT_FILENO, buf, bytes);
            if (written != bytes) {
                const char msg[] = "error: failed to echo\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }
        }
    }

    if (bytes == 0) {
        const char term = '\0';
        write(file, &term, sizeof(term));
    }
    close(file);
}