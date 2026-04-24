#include "./include/player.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

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
    p->crit_chance = 12;
    return p;
}

void player_move(Player* p, int direction) {
    p->row += direction;
}

bool checkCritical(int crit_chance) {
    return (rand() % 100) < crit_chance;
}

void player_shoot(Player* shooter, Player* target) {
    if (shooter->row == target->row) { // hit
        if (checkCritical((shooter->crit_chance * shooter->miss_count)) ){
            target->hp -= 30;
            shooter->miss_count = 1;
        } else {
            target->hp -= 20;
        }
    } else { // miss
        if (shooter->hp > 10) {
            shooter->hp -= 10;
            printf("You missed! The recoil deals 10 damage.\n");
        } else {
            printf("You missed! Your survival instincts prevent your from taking anymore damage.\n");
        }
        if (shooter->miss_count < 5) {
            shooter->miss_count++;
        }
    }
}

void player_heal(Player* p) {
    if (p->potions > 0) {
        if (p->hp < 100) {
            p->hp += 30;
            p->potions--;

            if (p->hp > 100) {
                p->hp = 100;
            }
            
        } else {
            printf("You're already at full health! Potion cannot be used right now.\n");
        }
    }
}

int player_get_hp(Player* p) { return p->hp; }
int player_get_row(Player* p) { return p->row; }
