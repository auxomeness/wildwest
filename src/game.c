#include "./include/game.h"
#include <stdlib.h>

Game* game_create() {
    Game* g = malloc(sizeof(Game));
    g->p1 = player_create(0);
    g->p2 = player_create(5);
    return g;
}

void game_resolve_turn(Game* g, Action a1, Action a2) {
    // Movement phase
    if (a1 == MOVE_UP) player_move(g->p1, -1);
    if (a1 == MOVE_DOWN) player_move(g->p1, 1);

    if (a2 == MOVE_UP) player_move(g->p2, -1);
    if (a2 == MOVE_DOWN) player_move(g->p2, 1);

    // Shooting phase
    if (a1 == SHOOT) player_shoot(g->p1, g->p2);
    if (a2 == SHOOT) player_shoot(g->p2, g->p1);

    // Healing phase
    if (a1 == HEAL) player_heal(g->p1);
    if (a2 == HEAL) player_heal(g->p2);
}

int game_is_over(Game* g) {
    return player_get_hp(g->p1) <= 0 || player_get_hp(g->p2) <= 0;
}
