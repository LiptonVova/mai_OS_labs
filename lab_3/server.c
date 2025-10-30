#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <wait.h>
#include <semaphore.h>

#include <stdio.h>


#define SHM_SIZE 4096

char SHM_NAME[1024] = "shm-name";
char SEM_NAME_SERVER[1024] = "sem-server";
char SEM_NAME_CHILD_1[1024] = "sem-child-1";
char SEM_NAME_CHILD_2[1024] = "sem-child-2";


int main() {
    char unique_suffix[64] = "\0";
    snprintf(unique_suffix, sizeof(unique_suffix), "%d", getpid());

    snprintf(SHM_NAME, sizeof(SHM_NAME), "%s", unique_suffix);
    snprintf(SEM_NAME_SERVER, sizeof(SEM_NAME_SERVER), "%s", unique_suffix);
    snprintf(SEM_NAME_CHILD_1, sizeof(SEM_NAME_CHILD_1), "%s", unique_suffix);
    snprintf(SEM_NAME_CHILD_2, sizeof(SEM_NAME_CHILD_2), "%s", unique_suffix);


    int shm = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shm == -1 && errno != ENOENT) {
        // enoent - ошибка: нет такого файла или каталога
        const char msg[] = "error: failed to open SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600);
    // O_RDWR - открыть на запись и на чтение
    // O_CREAT - создать, если не существует 
    // O_TRUNC - если существовует, то стереть 
    // 0600 - права доступа, 6 = 110 
    // владелец может читать и писать

    if (shm == -1) {
        // возникла ошибка при создании разделяемой памяти
        const char msg[] = "error: failed to open SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    if (ftruncate(shm, SHM_SIZE) == -1) {
        // shm_open просто создал разделяемую память нулевой длины
        // функция ftruncate изменяет размер этой памяти до SHM_SIZE
        // возвращает -1, если возникла ошибка
        const char msg[] = "error: failed to resize SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    // отображает разделяемую память в адресное пространство процесса
    if (shm_buf == MAP_FAILED) {
        const char msg[] = "error: failed to map SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_server = sem_open(SEM_NAME_SERVER, O_RDWR | O_CREAT | O_TRUNC, 0600, 1);
    if (sem_server == SEM_FAILED) {
        const char msg[] = "error: failed to create semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_client_1 = sem_open(SEM_NAME_CHILD_1, O_RDWR | O_CREAT | O_TRUNC, 0600, 0);
    if (sem_client_1 == SEM_FAILED) {
        const char msg[] = "error: failed to create semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    sem_t *sem_client_2 = sem_open(SEM_NAME_CHILD_2, O_RDWR | O_CREAT | O_TRUNC, 0600, 0);
    if (sem_client_2 == SEM_FAILED) {
        const char msg[] = "error: failed to create semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    pid_t child_1 = fork();

    if (child_1 == 0) {
        // значит мы находимся в дочернем процессе
        char *args[] = {"child_1", SHM_NAME, SEM_NAME_SERVER, SEM_NAME_CHILD_1, SEM_NAME_CHILD_2, NULL};
        execv("./child_1", args);

        const char msg[] = "error: failed to exec\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }
    else if (child_1 == -1) {
        const char msg[] = "error: failed to fork\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    pid_t child_2 = fork();

    if (child_2 == 0) {
        // находимся в дочернем процессе
        char *args[] = {"child_2", SHM_NAME, SEM_NAME_SERVER, SEM_NAME_CHILD_1, SEM_NAME_CHILD_2, NULL};
        execv("./child_2", args);

        const char msg[] = "error: failed to exec\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }
    else if (child_2 == -1) {
        // ошибка при создании процесса
        const char msg[] = "error: failed to fork\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        _exit(EXIT_FAILURE);
    }

    // находимся в родительском процессе
    bool running = true;
    while (running) {
        sem_wait(sem_server);

        uint32_t *length = (uint32_t *)shm_buf;
        char *text = shm_buf + sizeof(uint32_t);

        if (*length == UINT32_MAX) {
            // завершение программы
            running = false;
        }
        else if (*length > 0) {
            // значит данные получены от child_2
            const char msg[] = "Result: ";
            write(STDOUT_FILENO, msg, sizeof(msg) - 1);
            write(STDOUT_FILENO, text, *length);

            *length = 0;
            sem_post(sem_server);
        }

        else {
            // length == 0
            // значит обработанных данных нет, надо получить данные от пользователя 
            const char msg[] = "Enter text (Ctrl+D for exit): ";
            write(STDOUT_FILENO, msg, sizeof(msg) - 1); 

            char buf[SHM_SIZE - sizeof(uint32_t)];
            ssize_t bytes = read(STDIN_FILENO, buf, sizeof(buf));

            if (bytes == -1) {
                const char error[] = "error: failed to read from standart input\n";
                write(STDERR_FILENO, error, sizeof(error));
                _exit(EXIT_FAILURE);
            }

            if (bytes > 0) {
                *length = bytes;
                memcpy(text, buf, bytes);
                sem_post(sem_client_1);
            }
            else {
                *length = UINT32_MAX;
                running = false;
                sem_post(sem_client_1);
            }
        }
    }

    waitpid(child_1, NULL, 0);
    waitpid(child_2, NULL, 0);

    sem_unlink(SEM_NAME_SERVER);
    sem_unlink(SEM_NAME_CHILD_1);
    sem_unlink(SEM_NAME_CHILD_2);

    sem_close(sem_server);
    sem_close(sem_client_1);
    sem_close(sem_client_2);

    munmap(shm_buf, SHM_SIZE);
    shm_unlink(SHM_NAME);
    close(shm);

    return 0;
}