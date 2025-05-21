#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#define USER_SIZE 32
#define CLUE_SIZE 128
#define TREASURE_FILE "treasures.dat"
#define MAX_USERS 100

typedef struct {
    int id;
    char username[USER_SIZE];
    float latitude;
    float longitude;
    char clue[CLUE_SIZE];
    int value;
} Treasure;

typedef struct {
    char username[USER_SIZE];
    int total_score;
} UserScore;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        return 1;
    }

    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", argv[1], TREASURE_FILE);

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }

    Treasure t;
    UserScore users[MAX_USERS];
    int user_count = 0;

    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        int found = 0;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, t.username) == 0) {
                users[i].total_score += t.value;
                found = 1;
                break;
            }
        }
        if (!found && user_count < MAX_USERS) {
            strncpy(users[user_count].username, t.username, USER_SIZE);
            users[user_count].total_score = t.value;
            user_count++;
        }
    }
    close(fd);

    for (int i = 0; i < user_count; i++) {
        printf("%s: %d\n", users[i].username, users[i].total_score);
    }

    fflush(stdout);
    return 0;
}
