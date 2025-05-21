#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

#define USER_SIZE 32
#define CLUE_SIZE 128
#define TREASURE_FILE "treasures.dat"

typedef struct {
    int id;
    char username[USER_SIZE];
    float latitude;
    float longitude;
    char clue[CLUE_SIZE];
    int value;
} Treasure;

pid_t monitor_pid = -1;
int pipe_fd[2];

void read_from_monitor() {
    char buffer[512];
    ssize_t count;

    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(pipe_fd[0], &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 500000; 

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(pipe_fd[0], &readfds);

        int ready = select(pipe_fd[0] + 1, &readfds, NULL, NULL, &timeout);
        if (ready > 0) {
            count = read(pipe_fd[0], buffer, sizeof(buffer) - 1);
            if (count <= 0) break;
            buffer[count] = '\0';
            printf("%s", buffer);
        } else {
            break; 
        }
    }
}


void write_command(const char *cmd) {
    int fd = open("command.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, cmd, strlen(cmd));
        close(fd);
    }
}

void sigchld_handler(int signo) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid == monitor_pid) {
        printf("Monitor process ended.\n");
        monitor_pid = -1;
    }
}

void start_monitor() {
    if (monitor_pid > 0) {
        printf("Monitor already running.\n");
        return;
    }

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    monitor_pid = fork();
    if (monitor_pid == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        void list_hunts(int sig) {
            DIR *d = opendir(".");
            struct dirent *dir;
            printf("[Monitor] Hunts found:\n");
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")) {
                    char filepath[256];
                    snprintf(filepath, sizeof(filepath), "%s/%s", dir->d_name, TREASURE_FILE);
                    if (access(filepath, F_OK) == 0) {
                        int fd = open(filepath, O_RDONLY);
                        struct stat st;
                        fstat(fd, &st);
                        int count = st.st_size / sizeof(Treasure);
                        printf("%s: %d treasures\n", dir->d_name, count);
                        close(fd);
                    }
                }
            }
            closedir(d);
            fflush(stdout);
        }

        void list_treasures(int sig) {
            char buffer[256];
            int fd = open("command.txt", O_RDONLY);
            if (fd < 0) {
                printf("[Monitor] Failed to read command.txt\n");
                fflush(stdout);
                return;
            }
            read(fd, buffer, sizeof(buffer)-1);
            close(fd);
            buffer[255] = 0;
            char cmd[64], hunt[128];
            if (sscanf(buffer, "%s %s", cmd, hunt) != 2) {
                printf("[Monitor] Invalid command format for list_treasures.\nExpected: list_treasures <HUNT_ID>\n");
                fflush(stdout);
                return;
            }
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "%s/%s", hunt, TREASURE_FILE);
            int tf = open(filepath, O_RDONLY);
            if (tf < 0) {
                printf("[Monitor] Cannot open file for %s\n", hunt);
                fflush(stdout);
                return;
            }
            Treasure t;
            printf("[Monitor] Treasures in %s:\n", hunt);
            while (read(tf, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                printf("ID: %d | %s | %d pts\n", t.id, t.username, t.value);
            }
            close(tf);
            fflush(stdout);
        }

        void view_treasure(int sig) {
            char buffer[256];
            int fd = open("command.txt", O_RDONLY);
            if (fd < 0) {
                printf("[Monitor] Failed to read command.txt\n");
                fflush(stdout);
                return;
            }
            read(fd, buffer, sizeof(buffer)-1);
            close(fd);
            buffer[255] = 0;
            char cmd[64], hunt[128];
            int tid;
            if (sscanf(buffer, "%s %s %d", cmd, hunt, &tid) != 3) {
                printf("[Monitor] Invalid command format for view_treasure.\nExpected: view_treasure <HUNT_ID> <ID>\n");
                fflush(stdout);
                return;
            }
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "%s/%s", hunt, TREASURE_FILE);
            int tf = open(filepath, O_RDONLY);
            if (tf < 0) {
                printf("[Monitor] Cannot open file for %s\n", hunt);
                fflush(stdout);
                return;
            }
            Treasure t;
            int found = 0;
            while (read(tf, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                if (t.id == tid) {
                    printf("ID: %d\nUser: %s\nLat: %.2f\nLon: %.2f\nClue: %s\nValue: %d\n",
                        t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("[Monitor] Treasure ID %d not found in %s\n", tid, hunt);
            }
            close(tf);
            fflush(stdout);
        }

        void stop_monitor(int sig) {
            printf("[Monitor] Exiting...\n");
            fflush(stdout);
            sleep(5);
            exit(0);
        }

        sa.sa_handler = list_hunts; sigaction(SIGUSR1, &sa, NULL);
        sa.sa_handler = list_treasures; sigaction(SIGUSR2, &sa, NULL);
        sa.sa_handler = view_treasure; sigaction(SIGTERM, &sa, NULL);
        sa.sa_handler = stop_monitor; sigaction(SIGHUP, &sa, NULL);

        printf("[Monitor] Ready.\n");
        fflush(stdout);
        while (1) pause();
    } else {
        close(pipe_fd[1]);
    }
}


void calculate_score() {
    DIR *d = opendir(".");
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")) {
            char *hunt = dir->d_name;
            int fd[2];
            pipe(fd);
            pid_t pid = fork();
            if (pid == 0) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                execl("./calculate_score_exec", "calculate_score_exec", hunt, NULL);
                perror("exec failed");
                exit(1);
            } else {
                close(fd[1]);
                char buf[256];
                ssize_t r;
                printf("--- Scores for %s ---\n", hunt);
                while ((r = read(fd[0], buf, sizeof(buf)-1)) > 0) {
                    buf[r] = 0;
                    printf("%s", buf);
                }
                close(fd[0]);
                waitpid(pid, NULL, 0);
            }
        }
    }
    closedir(d);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char input[256];
    printf("Welcome to Treasure Hub.\n");

    while (1) {
        printf("hub> "); fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "start_monitor") == 0) start_monitor();
        else if (strcmp(input, "list_hunts") == 0 && monitor_pid > 0) {
            write_command("list_hunts");
            kill(monitor_pid, SIGUSR1);
            read_from_monitor();
        } else if (strncmp(input, "list_treasures ", 15) == 0 && monitor_pid > 0) {
            write_command(input);
            kill(monitor_pid, SIGUSR2);
            read_from_monitor();
        } else if (strncmp(input, "view_treasure ", 14) == 0 && monitor_pid > 0) {
            write_command(input);
            kill(monitor_pid, SIGTERM);
            read_from_monitor();
        } else if (strcmp(input, "stop_monitor") == 0 && monitor_pid > 0) kill(monitor_pid, SIGHUP);
        else if (strcmp(input, "calculate_score") == 0) calculate_score();
        else if (strcmp(input, "exit") == 0 && monitor_pid == -1) break;
        else printf("Invalid or unavailable command.\n");
    }
    return 0;
}
