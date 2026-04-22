#include "./include/game.h"
#include <stdlib.h>
#include <stdio.h>

// GAME INITIALIZATION
Game* game_create() {
    Game* g = malloc(sizeof(Game));

    // Players spawn at opposite ends
    g->p1 = player_create(GRID_MIN);
    g->p2 = player_create(GRID_MAX);

    return g;
}

// BATTLEFIELD RENDER 
void game_render(Game* g) {

    printf("\n=== BATTLEFIELD ===\n");

    for (int i = GRID_MIN; i <= GRID_MAX; i++) {

        printf("Row %d | ", i);

        if (player_get_row(g->p1) == i)
            printf("P1 ");

        if (player_get_row(g->p2) == i)
            printf("P2 ");

        printf("\n");
    }

    printf("------------------\n");
    printf("P1 HP: %d\n", player_get_hp(g->p1));
    printf("P2 HP: %d\n", player_get_hp(g->p2));
    printf("==================\n");
}

// TURN RESOLUTION
void game_resolve_turn(Game* g, Action a1, Action a2) {

    // -------- MOVEMENT PHASE --------
    if (a1 == MOVE_UP)   player_move(g->p1, -1);
    if (a1 == MOVE_DOWN) player_move(g->p1, 1);

    if (a2 == MOVE_UP)   player_move(g->p2, -1);
    if (a2 == MOVE_DOWN) player_move(g->p2, 1);

    // -------- SHOOTING PHASE --------
    if (a1 == SHOOT) player_shoot(g->p1, g->p2);
    if (a2 == SHOOT) player_shoot(g->p2, g->p1);

    // -------- HEALING PHASE --------
    if (a1 == HEAL) player_heal(g->p1);
    if (a2 == HEAL) player_heal(g->p2);

    game_render(g);
}

// GAME END CONDITION
int game_is_over(Game* g) {
    return player_get_hp(g->p1) <= 0 ||
           player_get_hp(g->p2) <= 0;
}
