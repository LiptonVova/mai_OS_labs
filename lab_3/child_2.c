#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <wait.h>
#include <semaphore.h>


#define SHM_SIZE 4096

const char SHM_NAME[] = "shm-name";
const char SEM_NAME_SERVER[] = "example-1-sem-server";
const char SEM_NAME_CHILD_1[] = "example-1-sem-child-1";
const char SEM_NAME_CHILD_2[] = "example-1-sem-child-2";

int main(int argc, char**argv) {
    if (argc < 5) {
        const char msg[] = "error: not enough arguments\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    const char *SHM_NAME = argv[1];
    const char *SEM_NAME_SERVER = argv[2];
    const char *SEM_NAME_CHILD_1 = argv[3];
    const char *SEM_NAME_CHILD_2 = argv[4];


    int shm = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shm == -1) {
        const char msg[] = "error: failed to open SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) {
        const char msg[] = "error: failed to map\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_client_2 = sem_open(SEM_NAME_CHILD_2, O_RDWR);
    if (sem_client_2 == SEM_FAILED) {
        const char msg[] = "error: failed to open semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    } 
    sem_t *sem_server = sem_open(SEM_NAME_SERVER, O_RDWR);
    if (sem_server == SEM_FAILED) {
        const char msg[] = "error: failed to open semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE); 
    }

    bool running = true;
    while (running) {
        sem_wait(sem_client_2);

        uint32_t *length = (uint32_t *)shm_buf;
        char *text = shm_buf + sizeof(uint32_t);

        if (*length == UINT32_MAX) {
            running = false;
        }
        else if (*length > 0) {
            // переводим в верхний регистр
            uint32_t new_length = 0;
            bool prev_symbol_is_space = 0;
            for (uint32_t i = 0; i < *length; ++i) {
                if (text[i] == ' ') {
                    if (!prev_symbol_is_space) {
                        text[new_length++] = text[i];
                        prev_symbol_is_space = true;
                    }
                }
                else {
                    text[new_length++] = text[i];
                    prev_symbol_is_space = false;
                }
            }

            *length = new_length;
        }
        sem_post(sem_server);
    }

    sem_close(sem_client_2);
    sem_close(sem_server);

    munmap(shm_buf, SHM_SIZE);
    close(shm);

    return 0;
}