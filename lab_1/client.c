#include <unistd.h> // системные вызовы

#include <stdint.h> // для uint32_t, int32_t 
#include <sys/wait.h> 
#include <stdio.h>
#include <stdlib.h>


static char SERVER_PROGRAM_SERVER_1[] = "server_1";
static char SERVER_PROGRAM_SERVER_2[] = "server_2";


int main(int argc, char * argv[]) {
    if (argc == 1) {
        char msg[1024];
        uint32_t len = snprintf(msg, sizeof(msg) - 1, "usage: %s filename\n", argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_FAILURE); 
    }


    char progpath[1024];
    {
        ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
        
        if (len == -1) {
            const char msg[] = "error: failed to read full program path\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        
        while (progpath[len] != '/') {
            --len;
        }
        
        progpath[len] = '\0';
    }

    int pipe_client_to_child1[2];
    int pipe_child1_to_child2[2];
    int pipe_child2_to_client[2];


    if (pipe(pipe_client_to_child1) == -1) {
        const char msg[] = "error: failed to create pipe client to server 1\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    if (pipe(pipe_child1_to_child2) == -1) {
        const char msg[] = "error: failed to create pipe server 1 to server 2\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    if (pipe(pipe_child2_to_client) == -1) {
        const char msg[] = "error: failed to create pipe server 2 to client\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }



    const pid_t child1 = fork(); // создаю новый процесс

    switch (child1) { // если ребенок 1
        case -1: {
            const char msg[] = "error: failed to spawn new process\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        } break;

        case 0: {
            close(pipe_child2_to_client[0]); // закрыл child2 - client
            close(pipe_child2_to_client[1]);

            close(pipe_client_to_child1[1]); // закрыл запись клиент - child1
            close(pipe_child1_to_child2[0]); // закрыл чтение child1 - child2

            dup2(pipe_client_to_child1[0], STDIN_FILENO); // перенаправил чтение из клиента в стандартный поток ввода
            close(pipe_client_to_child1[0]); 

            dup2(pipe_child1_to_child2[1], STDOUT_FILENO); // перенаправил запись в сервер 2 в стандартный поток вывода
            close(pipe_child1_to_child2[1]);

            {
                char path[4096];
                snprintf(path, sizeof(path) - 1, "%s/%s", progpath, SERVER_PROGRAM_SERVER_1);

                char *const args[] = {SERVER_PROGRAM_SERVER_1, argv[1], NULL};
                
                int32_t status = execv(path, args);

                if (status == -1) {
                    const char msg[] = "error: failed to exec into new executable image\n";
                    write(STDERR_FILENO, msg, sizeof(msg));
                    exit(EXIT_FAILURE);
                }
            }
        } break;
    
        default: { // если родительский процесс 
            const pid_t child2 = fork(); // создаю второго ребенка

            switch (child2) {
                case -1: {
                    const char msg[] = "error: failed to spawn new process\n";
                    write(STDERR_FILENO, msg, sizeof(msg));
                    exit(EXIT_FAILURE);
                } break;

                case 0: { // если второй ребенок
                    close(pipe_client_to_child1[0]); // закрываем pipe client-server 1
                    close(pipe_client_to_child1[1]);

                    close(pipe_child1_to_child2[1]); // закрываем запись server 1 - server 2
                    close(pipe_child2_to_client[0]); // закрываем чтение сервер 2 - клиент

                    dup2(pipe_child1_to_child2[0], STDIN_FILENO);
                    close(pipe_child1_to_child2[0]);

                    dup2(pipe_child2_to_client[1], STDOUT_FILENO);
                    close(pipe_child2_to_client[1]);

                    {
                        char path[4096];
                        snprintf(path, sizeof(path) - 1, "%s/%s", progpath, SERVER_PROGRAM_SERVER_2);

                        char *const args[] = {SERVER_PROGRAM_SERVER_2, argv[1], NULL};

                        int32_t status = execv(path, args);

                        if (status == -1) {
                            const char msg[] = "error: failed to exec into new executable image\n";
                            write(STDERR_FILENO, msg, sizeof(msg));
                            exit(EXIT_FAILURE);
                        }
                    }
                } break;

                default: { // логика родителя : 
                //  получаем данные от клиента - отправляем данные на сервер 1 - получаем данные из сервера 2 
                    pid_t pid = getpid();
                    {
                        
                        char msg[256];

                        const int32_t length = snprintf(
                            msg, sizeof(msg), 
                            "PID %d: I`m a parent, my child 1 has PID: %d, child 2 has PID: %d.\n           To exit, press CTRL+C\n", 
                            pid, child1, child2);
                        write(STDOUT_FILENO, msg, length);
                    }

                    close(pipe_client_to_child1[0]);                    
                    close(pipe_child1_to_child2[0]);
                    close(pipe_child1_to_child2[1]);
                    close(pipe_child2_to_client[1]);

                    char buf[8096];
                    ssize_t bytes;

                    char msg[1024];
                    const int32_t length = snprintf(msg, sizeof(msg),
                        "PID %d: Write a message that needs to be changed: ", pid);
                    write(STDOUT_FILENO, msg, length);

                    while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
                        if (bytes < 0) {
                            const char msg[] = "error: failed to read from stdin\n";
                            write(STDERR_FILENO, msg, sizeof(msg));
                            exit(EXIT_FAILURE);
                        }
                        else if (buf[0] == '\n') {
                            break;
                        }

                        write(pipe_client_to_child1[1], buf, bytes);

                        bytes = read(pipe_child2_to_client[0], buf, bytes);

                        char info_pid[32];
                        int32_t length_info_pid = snprintf(info_pid, sizeof(info_pid), "PID %d: ", pid);

                        write(STDOUT_FILENO, info_pid, length_info_pid);
                        write(STDOUT_FILENO, buf, bytes);
                        write(STDOUT_FILENO, msg, length);

                    }

                    close(pipe_client_to_child1[1]);
                    close(pipe_child2_to_client[0]);

                    waitpid(child1, NULL, 0);
                    waitpid(child2, NULL, 0);
                } break;
            }            
        } break;
    }
}