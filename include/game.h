#ifndef GAME_H
#define GAME_H

#include "player.h"

//Nigger, Please Read.
//Game Functions is here buddy, Deal with it.

typedef enum {
    MOVE_UP,
    MOVE_DOWN,
    SHOOT,
    HEAL
} Action;

typedef struct {
    Player* p1;
    Player* p2;
} Game;

Game* game_create();
void game_resolve_turn(Game* g, Action a1, Action a2);
int game_is_over(Game* g);

#endif
