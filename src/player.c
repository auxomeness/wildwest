#include "./include/player.h"
#include <stdlib.h>

struct Player {
    int hp;
    int row;
    int potions;
};

Player* player_create(int start_row) {
    Player* p = malloc(sizeof(Player));
    p->hp = 100;
    p->row = start_row;
    p->potions = 3;
    return p;
}

void player_move(Player* p, int direction) {
    p->row += direction;
}

void player_shoot(Player* shooter, Player* target) {
    if (shooter->row == target->row) {
        target->hp -= 20;
    } else {
        shooter->hp -= 10;
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
