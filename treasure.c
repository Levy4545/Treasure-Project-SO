#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include "treasure.h"

void log_operation(const char *hunt_id, const char *operation) {
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, LOG_FILE);
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0)
      return;

    time_t now = time(NULL);

    char log_entry[256];
    snprintf(log_entry, sizeof(log_entry), "[%s] %s\n", strtok(ctime(&now), "\n"), operation);
    write(fd, log_entry, strlen(log_entry));
    close(fd);
}

void create_symlink(const char *hunt_id) {
    char target[128], linkname[128];
    snprintf(target, sizeof(target), "%s/%s", hunt_id, LOG_FILE);
    snprintf(linkname, sizeof(linkname), "logged_hunt-%s", hunt_id);
    symlink(target, linkname);
}

void add_treasure(const char *hunt_id) {
    mkdir(hunt_id, 0755);
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, TREASURE_FILE);

    Treasure t;
    char buffer[256];

    write(1, "Enter Treasure ID: ", 20);
    read(0, buffer, sizeof(buffer));
    t.id = atoi(buffer);

    write(1, "Enter Username: ", 16);
    read(0, t.username, USER_SIZE);
    t.username[strcspn(t.username, "\n")] = 0;

    write(1, "Enter Latitude: ", 16);
    read(0, buffer, sizeof(buffer));
    t.latitude = atof(buffer);

    write(1, "Enter Longitude: ", 17);
    read(0, buffer, sizeof(buffer));
    t.longitude = atof(buffer);

    write(1, "Enter Clue: ", 12);
    read(0, t.clue, CLUE_SIZE);
    t.clue[strcspn(t.clue, "\n")] = 0;

    write(1, "Enter Value: ", 13);
    read(0, buffer, sizeof(buffer));
    t.value = atoi(buffer);

    if (t.id < 0 || t.latitude < -90 || t.latitude > 90 || t.longitude < -180 || t.longitude > 180 || t.value < 0) {
        write(1, "Invalid data.\n", 14);
        return;
    }

    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }
    write(fd, &t, sizeof(Treasure));
    close(fd);

    log_operation(hunt_id, "Added a treasure");
    create_symlink(hunt_id);
}

void list_treasures(const char *hunt_id) {
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, TREASURE_FILE);
    struct stat st;
    if (stat(filepath, &st) == -1) {
        write(1, "File not found.\n", 17);
        return;
    }

    char info[256];
    snprintf(info, sizeof(info), "Hunt: %s\nSize: %ld bytes\nLast Modified: %s", hunt_id, st.st_size, ctime(&st.st_mtime));
    write(1, info, strlen(info));

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return;

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        snprintf(info, sizeof(info), "ID: %d, User: %s, Lat: %.2f, Lon: %.2f, Clue: %s, Value: %d\n",
            t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
        write(1, info, strlen(info));
    }
    close(fd);
    log_operation(hunt_id, "Listed treasures");
}

void view_treasure(const char *hunt_id, int treasure_id) {
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, TREASURE_FILE);
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        write(1, "Could not open file.\n", 22);
        return;
    }

    Treasure t;
    int found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id == treasure_id) {
            char info[256];
            snprintf(info, sizeof(info), "ID: %d\nUser: %s\nLat: %.2f\nLon: %.2f\nClue: %s\nValue: %d\n",
                t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
            write(1, info, strlen(info));
            found = 1;
            break;
        }
    }
    if (!found) write(1, "Treasure not found.\n", 21);
    close(fd);
    log_operation(hunt_id, "Viewed treasure");
}

void remove_treasure(const char *hunt_id, int treasure_id) {
    char filepath[128], tmp[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, TREASURE_FILE);
    snprintf(tmp, sizeof(tmp), "%s/tmp.dat", hunt_id);

    int in = open(filepath, O_RDONLY);
    int out = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    Treasure t;
    int found = 0;
    while (read(in, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id != treasure_id) {
            write(out, &t, sizeof(Treasure));
        } else {
            found = 1;
        }
    }
    close(in); close(out);
    rename(tmp, filepath);
    if (found)
        log_operation(hunt_id, "Removed a treasure");
    else
        write(1, "Treasure not found.\n", 21);
}

void remove_hunt(const char *hunt_id) {
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, TREASURE_FILE);
    unlink(filepath);

    snprintf(filepath, sizeof(filepath), "%s/%s", hunt_id, LOG_FILE);
    unlink(filepath);

    snprintf(filepath, sizeof(filepath), "logged_hunt-%s", hunt_id);
    unlink(filepath);

    rmdir(hunt_id);
}
