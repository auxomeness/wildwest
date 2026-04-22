#include "./include/player.h"
#include <stdlib.h>
#include <stdbool.h>

struct Player {
    int hp;
    int row;
    int potions;
    int miss_count; //pirang beses mo namiss
    int crit_chance;
};

Player* player_create(int start_row) {
    Player* p = malloc(sizeof(Player));
    p->hp = 100;
    p->row = start_row;
    p->potions = 3;
    p->miss_count = 1;
    p->crit_chance;
    return p;
}

void player_move(Player* p, int direction) {
    p->row += direction;
}

bool checkCritical(int crit_chance) {
    return (rand() % 100) < crit_chance;
}

void player_shoot(Player* shooter, Player* target) {
    if (shooter->row == target->row) {
        srand(time(NULL));
        if (checkCritical((shooter->crit_chance * shooter->miss_count)) ){
            target->hp -= 30;
            shooter->miss_count = 1;
        } else {
            target->hp -= 20;
        }
    } else {
        shooter->hp -= 10;
        if (shooter->miss_count == 5) {
            shooter->miss_count = 1;
        }
    }
}

void player_heal(Player* p) {
    if (p->potions > 0) {
        p->hp += 30;
        p->potions--;
    }
}

int player_get_hp(Player* p) { return p->hp; }
int player_get_row(Player* p) { return p->row; }
