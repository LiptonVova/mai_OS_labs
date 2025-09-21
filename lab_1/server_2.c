#include <stdint.h>

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

        char answer[4096];
        int index = 0; 
        for (uint32_t i = 0; i < bytes; ++i) {
            if (i != (bytes - 1) && buf[i] == buf[i + 1] && buf[i] == ' ') {
                ++i;
                continue;
            }
            answer[index] = buf[i];
            ++index;
        }

        {
            char log_msg[8096];
            uint32_t log_length = snprintf(log_msg, sizeof(log_msg), "(Child 2)PID %d: %s", pid, answer);

            
            uint32_t written = write(file, log_msg, log_length);
            if (written != log_length) {
                const char msg[] = "error: failed to write in file\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }

            written = write(STDOUT_FILENO, answer, index);
            if (written != index) {
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