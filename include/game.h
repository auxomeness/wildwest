#ifndef GAME_H
#define GAME_H

#include <stddef.h>
#include "player.h"

/* Shared gameplay constants used by both server and client. */
#define DEFAULT_PORT 51717
#define GRID_HEIGHT 10
#define GRID_WIDTH 5

#define PLAYER1_ROW 1
#define PLAYER2_ROW (GRID_HEIGHT - 2)

#define MAX_HP 100
#define MAX_POTIONS 3
#define SHOT_DAMAGE 20
#define CRIT_DAMAGE 30
#define MISS_DAMAGE 10
#define HEAL_AMOUNT 30
#define DEFAULT_CRIT_CHANCE 13

#define MOVE_PHASE_MS 15000
#define ACTION_PHASE_MS 10000
#define RESOLVE_PHASE_MS 2000

/* Match phases controlled by the server. */
typedef enum {
    PHASE_WAITING = 0,
    PHASE_MOVE = 1,
    PHASE_ACTION = 2,
    PHASE_RESOLVE = 3,
    PHASE_GAME_OVER = 4
} Phase;

/* Full replicated game state sent from server to client. */
typedef struct {
    Player p1;
    Player p2;
    int p1_ready;
    int p2_ready;
    ResolveResult p1_result;
    ResolveResult p2_result;
    int bullet1_row;
    int bullet1_col;
    int bullet1_active;
    int bullet2_row;
    int bullet2_col;
    int bullet2_active;
    Phase phase;
    int phase_time_ms;
    int round_number;
    int winner;
    int running;
} GameState;

/* Core game lifecycle and rendering helpers. */
void game_init(GameState *game);
void game_start_move_phase(GameState *game);
void game_start_action_phase(GameState *game);
void game_start_resolve_phase(GameState *game);
void game_update_bullets(GameState *game);
int game_phase_time_limit(Phase phase);
int game_countdown_seconds(const GameState *game);
const char *game_phase_label(Phase phase);
const char *game_action_label(Action action);
const char *game_result_label(ResolveResult result);
void game_build_display_key(const GameState *game, int player_id, char *buffer, size_t buffer_size);
void game_render(const GameState *game, int player_id);

#endif
