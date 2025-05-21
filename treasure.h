#define TREASURE_H

#define USER_SIZE 32
#define CLUE_SIZE 128
#define TREASURE_FILE "treasures.dat"
#define LOG_FILE "logged_hunt"

typedef struct {
    int id;
    char username[USER_SIZE];
    float latitude;
    float longitude;
    char clue[CLUE_SIZE];
    int value;
} Treasure;

void add_treasure(const char *hunt_id);
void list_treasures(const char *hunt_id);
void view_treasure(const char *hunt_id, int treasure_id);
void remove_treasure(const char *hunt_id, int treasure_id);
void remove_hunt(const char *hunt_id);
void log_operation(const char *hunt_id, const char *operation);
void create_symlink(const char *hunt_id);
