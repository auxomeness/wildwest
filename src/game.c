#include "game.h"
#include <stdio.h>
#include <string.h>

/* Bullet trails only exist during the resolve animation. */
static void clear_bullets(GameState *game)
{
    game->bullet1_row = 0;
    game->bullet1_col = 0;
    game->bullet1_active = 0;
    game->bullet2_row = 0;
    game->bullet2_col = 0;
    game->bullet2_active = 0;
}

static void clear_results(GameState *game)
{
    game->p1_result = RESULT_NONE;
    game->p2_result = RESULT_NONE;
}

static void update_winner(GameState *game)
{
    /* Winner is evaluated after both actions have resolved. */
    if (game->p1.hp <= 0 && game->p2.hp <= 0) {
        game->winner = 3;
    } else if (game->p2.hp <= 0) {
        game->winner = 1;
    } else if (game->p1.hp <= 0) {
        game->winner = 2;
    } else {
        game->winner = 0;
    }
}

static const Player *local_player(const GameState *game, int player_id)
{
    return player_id == 1 ? &game->p1 : &game->p2;
}

static const Player *enemy_player(const GameState *game, int player_id)
{
    return player_id == 1 ? &game->p2 : &game->p1;
}

static ResolveResult local_result(const GameState *game, int player_id)
{
    return player_id == 1 ? game->p1_result : game->p2_result;
}

static ResolveResult enemy_result(const GameState *game, int player_id)
{
    return player_id == 1 ? game->p2_result : game->p1_result;
}

void game_init(GameState *game)
{
    /* Initial state stops at the ready screen until both players confirm. */
    memset(game, 0, sizeof(*game));

    player_init(&game->p1, GRID_WIDTH / 2);
    player_init(&game->p2, GRID_WIDTH / 2);

    game->running = 1;
    game->round_number = 0;
    game->phase = PHASE_WAITING;
    game->phase_time_ms = 0;
    game->p1_ready = 0;
    game->p2_ready = 0;

    clear_bullets(game);
    clear_results(game);
}

void game_start_move_phase(GameState *game)
{
    /* Each round begins by clearing action locks and transient results. */
    player_clear_action(&game->p1);
    player_clear_action(&game->p2);
    player_unlock(&game->p1);
    player_unlock(&game->p2);

    clear_bullets(game);
    clear_results(game);
    game->p1_ready = 0;
    game->p2_ready = 0;

    game->phase = PHASE_MOVE;
    game->phase_time_ms = 0;
    game->round_number += 1;
}

void game_start_action_phase(GameState *game)
{
    /* Default action is shoot so the player can simply lock it if desired. */
    player_set_action(&game->p1, ACTION_SHOOT);
    player_set_action(&game->p2, ACTION_SHOOT);
    player_unlock(&game->p1);
    player_unlock(&game->p2);

    clear_bullets(game);

    game->phase = PHASE_ACTION;
    game->phase_time_ms = 0;
}

void game_start_resolve_phase(GameState *game)
{
    /* Resolve uses the locked positions and locked actions from this round. */
    clear_bullets(game);

    if (game->p1.action == ACTION_NONE) {
        player_set_action(&game->p1, ACTION_SHOOT);
    }

    if (game->p2.action == ACTION_NONE) {
        player_set_action(&game->p2, ACTION_SHOOT);
    }

    if (game->p1.action == ACTION_SHOOT) {
        game->bullet1_active = 1;
        game->bullet1_row = PLAYER1_ROW + 1;
        game->bullet1_col = game->p1.col;
    }

    if (game->p2.action == ACTION_SHOOT) {
        game->bullet2_active = 1;
        game->bullet2_row = PLAYER2_ROW - 1;
        game->bullet2_col = game->p2.col;
    }

    game->p1_result = player_apply_shot(&game->p1, &game->p2);
    game->p2_result = player_apply_shot(&game->p2, &game->p1);

    if (game->p1.action == ACTION_HEAL) {
        game->p1_result = player_apply_heal(&game->p1);
    }

    if (game->p2.action == ACTION_HEAL) {
        game->p2_result = player_apply_heal(&game->p2);
    }

    update_winner(game);

    game->phase = PHASE_RESOLVE;
    game->phase_time_ms = 0;
}

void game_update_bullets(GameState *game)
{
    /* Bullet sprites move one row per render tick during resolve. */
    if (game->bullet1_active) {
        game->bullet1_row += 1;
        if (game->bullet1_row > PLAYER2_ROW) {
            game->bullet1_active = 0;
        }
    }

    if (game->bullet2_active) {
        game->bullet2_row -= 1;
        if (game->bullet2_row < PLAYER1_ROW) {
            game->bullet2_active = 0;
        }
    }
}

int game_phase_time_limit(Phase phase)
{
    if (phase == PHASE_MOVE) {
        return MOVE_PHASE_MS;
    }

    if (phase == PHASE_ACTION) {
        return ACTION_PHASE_MS;
    }

    if (phase == PHASE_RESOLVE) {
        return RESOLVE_PHASE_MS;
    }

    return 0;
}

int game_countdown_seconds(const GameState *game)
{
    int remaining = game_phase_time_limit(game->phase) - game->phase_time_ms;

    if (remaining <= 0) {
        return 0;
    }

    return (remaining + 999) / 1000;
}

const char *game_phase_label(Phase phase)
{
    if (phase == PHASE_MOVE) {
        return "MOVE";
    }

    if (phase == PHASE_ACTION) {
        return "ACTION";
    }

    if (phase == PHASE_RESOLVE) {
        return "RESOLVE";
    }

    if (phase == PHASE_GAME_OVER) {
        return "GAME OVER";
    }

    return "WAITING";
}

const char *game_action_label(Action action)
{
    if (action == ACTION_SHOOT) {
        return "SHOOT";
    }

    if (action == ACTION_HEAL) {
        return "HEAL";
    }

    return "NONE";
}

const char *game_result_label(ResolveResult result)
{
    if (result == RESULT_SHOT_HIT) {
        return "SHOT HIT";
    }

    if (result == RESULT_SHOT_CRIT) {
        return "CRITICAL HIT";
    }

    if (result == RESULT_SHOT_MISS) {
        return "SHOT MISS";
    }

    if (result == RESULT_HEAL) {
        return "HEAL +30";
    }

    if (result == RESULT_HEAL_FAIL) {
        return "HEAL FAILED";
    }

    return "NONE";
}

void game_build_display_key(const GameState *game, int player_id, char *buffer, size_t buffer_size)
{
    /* Used to avoid redrawing the same frame repeatedly. */
    const Player *self = local_player(game, player_id);
    const Player *enemy = enemy_player(game, player_id);
    int enemy_visible = (game->phase == PHASE_RESOLVE || game->phase == PHASE_GAME_OVER);
    int countdown = -1;

    if (game->phase == PHASE_MOVE || game->phase == PHASE_ACTION) {
        countdown = game_countdown_seconds(game);
    }

    snprintf(buffer,
             buffer_size,
             "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d",
             game->phase,
             game->round_number,
             game->winner,
             game->running,
             countdown,
             self->hp,
             self->potions,
             self->locked,
             self->action,
             self->col,
             enemy->hp,
             enemy->potions,
             enemy->locked,
             enemy_visible ? enemy->col : -1,
             game->p1_ready,
             game->p2_ready,
             local_result(game, player_id),
             enemy_result(game, player_id),
             game->bullet1_row,
             game->bullet1_col,
             game->bullet1_active,
             game->bullet2_row,
             game->bullet2_col,
             game->bullet2_active);
}

void game_render(const GameState *game, int player_id)
{
    /* Render one full terminal frame for the requested player perspective. */
    const Player *self = local_player(game, player_id);
    const Player *enemy = enemy_player(game, player_id);
    ResolveResult self_result = local_result(game, player_id);
    ResolveResult foe_result = enemy_result(game, player_id);
    char self_char = player_id == 1 ? '1' : '2';
    char enemy_char = player_id == 1 ? '2' : '1';
    int enemy_visible = (game->phase == PHASE_RESOLVE || game->phase == PHASE_GAME_OVER);
    int row;
    int col;

    printf("\033[2J\033[H");
    printf("Wild West Quick Draw | Player %d\n", player_id);

    if (game->phase == PHASE_MOVE || game->phase == PHASE_ACTION) {
        printf("Round: %d | Phase: %s | Countdown: %d\n",
               game->round_number,
               game_phase_label(game->phase),
               game_countdown_seconds(game));
    } else {
        printf("Round: %d | Phase: %s\n",
               game->round_number,
               game_phase_label(game->phase));
    }

    printf("You   HP: %3d | Potions: %d | Locked: %s\n",
           self->hp,
           self->potions,
           self->locked ? "YES" : "NO");
    printf("Enemy HP: %3d | Potions: %d | Locked: %s\n",
           enemy->hp,
           enemy->potions,
           enemy->locked ? "YES" : "NO");
    if (game->phase == PHASE_WAITING) {
        printf("Ready: You [%c] | Enemy [%c]\n",
               (player_id == 1 ? game->p1_ready : game->p2_ready) ? 'x' : ' ',
               (player_id == 1 ? game->p2_ready : game->p1_ready) ? 'x' : ' ');
        printf("Controls: Space = ready | q = quit\n");
    } else if (game->phase == PHASE_MOVE) {
        printf("Controls: Left/Right Arrow = move | Space = lock | q = quit\n");
    } else if (game->phase == PHASE_ACTION) {
        printf("Controls: [ = shoot | ] = heal | Space = lock | q = quit\n");
    } else {
        printf("Controls: q = quit\n");
    }
    printf("\n");

    printf("+");
    for (col = 0; col < GRID_WIDTH * 7; col++) {
        printf("=");
    }
    printf("+\n");

    for (row = 0; row < GRID_HEIGHT; row++) {
        printf("|");

        for (col = 0; col < GRID_WIDTH; col++) {
            char ch = ' ';

            if (game->bullet1_active && row == game->bullet1_row && col == game->bullet1_col) {
                ch = 'v';
            } else if (game->bullet2_active && row == game->bullet2_row && col == game->bullet2_col) {
                ch = '^';
            } else if (row == (player_id == 1 ? PLAYER1_ROW : PLAYER2_ROW) && col == self->col) {
                ch = self_char;
            } else if (enemy_visible &&
                       row == (player_id == 1 ? PLAYER2_ROW : PLAYER1_ROW) &&
                       col == enemy->col) {
                ch = enemy_char;
            }

            printf("   %c   ", ch);
        }

        printf("|\n");
    }

    printf("+");
    for (col = 0; col < GRID_WIDTH * 7; col++) {
        printf("=");
    }
    printf("+\n");

    printf("\n");

    if (game->phase == PHASE_WAITING) {
        printf("Press Space when ready. The duel starts when both players are ready.\n");
    } else if (game->phase == PHASE_MOVE) {
        printf("Enemy position is hidden. Choose a lane with Left/Right Arrow, then press Space to lock.\n");
    } else if (game->phase == PHASE_ACTION) {
        printf("SHOOT [%c]      HEAL [%c]\n",
               self->action == ACTION_SHOOT ? 'x' : ' ',
               self->action == ACTION_HEAL ? 'x' : ' ');
        printf("Enemy position is still hidden. Use '[' for Shoot and ']' for Heal, then press Space to lock.\n");
    } else if (game->phase == PHASE_RESOLVE) {
        printf("Resolve: you %s | enemy %s\n",
               game_result_label(self_result),
               game_result_label(foe_result));
    } else if (game->phase == PHASE_GAME_OVER) {
        if (game->winner == 3) {
            printf("Result: draw.\n");
        } else {
            printf("Result: Player %d wins.\n", game->winner);
        }
    }

    fflush(stdout);
}
