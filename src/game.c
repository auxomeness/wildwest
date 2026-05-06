#include "game.h"
#include "./include/ultimate.h"
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

<<<<<<< HEAD
    if (game->p2.action == ACTION_NONE) {
        player_set_action(&game->p2, ACTION_SHOOT);
=======
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

<<<<<<< HEAD
int move_phase_done(char player_input) {
    if (player_input != ' ') {
        return 1;
=======
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

static void print_double_box_border(int left_width, int right_width)
{
    printf("+");
    for (int i = 0; i < left_width + 2; i++) {
        printf("=");
    }
    printf("+");
    for (int i = 0; i < right_width + 2; i++) {
        printf("=");
    }
    printf("+\n");
}

static void print_double_box_row(const char *left_text, int left_width,
                                 const char *right_text, int right_width)
{
    printf("| %-*s | %-*s |\n",
           left_width,
           left_text,
           right_width,
           right_text);
}

static void print_single_box_border(int width)
{
    printf("+");
    for (int i = 0; i < width + 2; i++) {
        printf("=");
    }
    printf("+\n");
}

static void print_single_box_row(const char *text, int width)
{
    printf("| %-*s |\n", width, text);
}

static void print_resource_bar(const char *label, int current, int maximum, const char *color_code)
{
    const int bar_slots = 5;
    int filled = (current * bar_slots + (maximum / 2)) / maximum;
    int index;

    if (filled < 0) {
        filled = 0;
    }

    if (filled > bar_slots) {
        filled = bar_slots;
    }

    printf("%s[", label);
    printf("%s", color_code);
    for (index = 0; index < bar_slots; index++) {
        printf("%s", index < filled ? "█" : "░");
    }
    printf("\033[0m");
    printf("]");
}

static void print_status_bar_cell(const char *label, int hp, int potions, int width)
{
    int visible_width = 0;

    printf("| %-5s ", label);
    visible_width += 8;

    print_resource_bar("HP:", hp, MAX_HP, "\033[92m");
    visible_width += 9;

    printf("  ");
    visible_width += 2;

    print_resource_bar("P:", potions, MAX_POTIONS, "\033[91m");
    visible_width += 8;

    while (visible_width < width) {
        putchar(' ');
        visible_width++;
    }

    printf(" ");
}

static void print_status_bar_row(int left_hp, int left_potions, int right_hp, int right_potions, int width)
{
    print_status_bar_cell("YOU", left_hp, left_potions, width);
    print_status_bar_cell("ENEMY", right_hp, right_potions, width);
    printf("|\n");
}

static size_t trimmed_line_length(const char *line)
{
    size_t length = strlen(line);

    while (length > 0 && line[length - 1] == ' ') {
        length--;
    }

    return length;
}

static void print_banner_line(const char *line, int width)
{
    size_t length = trimmed_line_length(line);
    int padding = 0;
    size_t index;

    if ((int)length < width) {
        padding = (width - (int)length) / 2;
    }

    for (index = 0; index < (size_t)padding; index++) {
        putchar(' ');
    }

    printf("%.*s", (int)length, line);
    putchar('\n');
}

static void print_result_banner(int winner, int player_id)
{
    static const char *draw_lines[] = {
        " *******     *******         **       **       **",
        "/**////**   /**////**       ****     /**      /**",
        "/**    /**  /**   /**      **//**    /**   *  /**",
        "/**    /**  /*******      **  //**   /**  *** /**",
        "/**    /**  /**///**     **********  /** **/**/**",
        "/**    **   /**  //**   /**//////**  /**** //****",
        "/*******    /**   //**  /**     /**  /**/   ///**",
        "///////     //     //   //      //   //       // "
    };
    static const char *winner_lines[] = {
        " **       **   **   ****     **   ****     **   ********   *******  ",
        "/**      /**  /**  /**/**   /**  /**/**   /**  /**/////   /**////** ",
        "/**   *  /**  /**  /**//**  /**  /**//**  /**  /**        /**   /** ",
        "/**  *** /**  /**  /** //** /**  /** //** /**  /*******   /*******  ",
        "/** **/**/**  /**  /**  //**/**  /**  //**/**  /**////    /**///**  ",
        "/**** //****  /**  /**   //****  /**   //****  /**        /**  //** ",
        "/**/   ///**  /**  /**    //***  /**    //***  /********  /**   //**",
        "//       //   //   //      ///   //      ///   ////////   //     // "
    };
    static const char *loser_lines[] = {
        " **           *******      ********   ********   *******  ",
        "/**          **/////**    **//////   /**/////   /**////** ",
        "/**         **     //**  /**         /**        /**   /** ",
        "/**        /**      /**  /*********  /*******   /*******  ",
        "/**        /**      /**  ////////**  /**////    /**///**  ",
        "/**        //**     **          /**  /**        /**  //** ",
        "/********   //*******     ********   /********  /**   //**",
        "////////     ///////     ////////    ////////   //     // "
    };
    const char **banner_lines;
    int line_count;
    int index;
    if (winner == 3) {
        banner_lines = draw_lines;
        line_count = (int)(sizeof(draw_lines) / sizeof(draw_lines[0]));
    } else if (winner == player_id) {
        banner_lines = winner_lines;
        line_count = (int)(sizeof(winner_lines) / sizeof(winner_lines[0]));
    } else {
        banner_lines = loser_lines;
        line_count = (int)(sizeof(loser_lines) / sizeof(loser_lines[0]));
    }

    for (index = 0; index < line_count; index++) {
        print_banner_line(banner_lines[index], 71);
    }
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
>>>>>>> 194727f (Refine terminal HUD and result banners)
>>>>>>> 642be32 (Refine terminal HUD and result banners)
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

    if (game->p1.action == ACTION_ULTIMATE && ultimate_can_use(&game->p1)) {
        ultimate_execute(&game->p1, &game->p2);
    }

    if (game->p2.action == ACTION_ULTIMATE && ultimate_can_use(&game->p2)) {
        ultimate_execute(&game->p2, &game->p1);
    }

    //preventing players from shooting and using altimate simultaneously to avoid bugs
    if (game->p1.action != ACTION_ULTIMATE) {
        game->p1_result = player_apply_shot(&game->p1, &game->p2);
    }

    if (game->p2.action != ACTION_ULTIMATE) {
        game->p2_result = player_apply_shot(&game->p2, &game->p1);
    }

    // Execute ultimates first
    if (game->p1.action == ACTION_ULTIMATE && ultimate_can_use(&game->p1)) {
        ultimate_execute(&game->p1, &game->p2);
    }

    if (game->p2.action == ACTION_ULTIMATE && ultimate_can_use(&game->p2)) {
        ultimate_execute(&game->p2, &game->p1);
    }

    // Only shoot if not using ultimate
    if (game->p1.action != ACTION_ULTIMATE) {
        game->p1_result = player_apply_shot(&game->p1, &game->p2);
    }

    if (game->p2.action != ACTION_ULTIMATE) {
        game->p2_result = player_apply_shot(&game->p2, &game->p1);
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
<<<<<<< HEAD
=======
<<<<<<< HEAD
=======
>>>>>>> 642be32 (Refine terminal HUD and result banners)

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
    if (action == ACTION_SHOOT) return "SHOOT";
    if (action == ACTION_HEAL) return "HEAL";
    if (action == ACTION_ULTIMATE) return "ULT";
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

<<<<<<< HEAD
void game_render(const GameState *game, int player_id)
=======
void game_render(const GameState *game, int player_id, int quit_armed)
>>>>>>> 642be32 (Refine terminal HUD and result banners)
{
    /* Render one full terminal frame for the requested player perspective. */
    const Player *self = local_player(game, player_id);
    const Player *enemy = enemy_player(game, player_id);
    ResolveResult self_result = local_result(game, player_id);
    ResolveResult foe_result = enemy_result(game, player_id);
    char self_char = player_id == 1 ? '1' : '2';
    char enemy_char = player_id == 1 ? '2' : '1';
    int enemy_visible = (game->phase == PHASE_RESOLVE || game->phase == PHASE_GAME_OVER);
<<<<<<< HEAD
=======
    int self_ready = player_id == 1 ? game->p1_ready : game->p2_ready;
    int enemy_ready = player_id == 1 ? game->p2_ready : game->p1_ready;
    int countdown = -1;
    char left_line1[64];
    char left_line2[64];
    char right_line1[64];
    char right_line2[64];
    char left_status_line[64];
    char countdown_line[64];
    char title_left[64];
    char title_right[64];
    char controls_line[96];
    const int stat_width = 33;
    const int full_width = (stat_width * 2) + 3;
>>>>>>> 642be32 (Refine terminal HUD and result banners)
    int row;
    int col;

    printf("\033[2J\033[H");
<<<<<<< HEAD
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
=======

    snprintf(title_left,
             sizeof(title_left),
             "Wild West Quick Draw");
    snprintf(title_right,
             sizeof(title_right),
             "Player %d",
             player_id);
    snprintf(left_line1,
             sizeof(left_line1),
             "YOU   Locked: %-3s   Ready: [%c]",
             self->locked ? "YES" : "NO",
             self_ready ? 'x' : ' ');
    snprintf(left_line2,
             sizeof(left_line2),
             "Action: %-5s",
             game_action_label(self->action));
    snprintf(right_line1,
             sizeof(right_line1),
             "ENEMY Locked: %-3s   Ready: [%c]",
             enemy->locked ? "YES" : "NO",
             enemy_ready ? 'x' : ' ');
    snprintf(right_line2,
             sizeof(right_line2),
             "Action: %-5s",
             game_action_label(enemy->action));
    snprintf(left_status_line,
             sizeof(left_status_line),
             "Round: %d   Phase: %s",
             game->round_number,
             game_phase_label(game->phase));

    if (game->phase == PHASE_MOVE || game->phase == PHASE_ACTION) {
        countdown = game_countdown_seconds(game);
        snprintf(countdown_line,
                 sizeof(countdown_line),
                 "Countdown: %2d",
                 countdown);
    } else {
        snprintf(countdown_line,
                 sizeof(countdown_line),
                 "Countdown: --");
    }

    if (game->phase == PHASE_WAITING) {
        snprintf(controls_line,
                 sizeof(controls_line),
                 "Controls: Space = ready | q = quit");
    } else if (game->phase == PHASE_MOVE) {
        snprintf(controls_line,
                 sizeof(controls_line),
                 "Controls: Left/Right Arrow = move | Space = lock | q = quit");
    } else if (game->phase == PHASE_ACTION) {
        snprintf(controls_line,
                 sizeof(controls_line),
                 "Controls: [ = shoot | ] = heal | Space = lock | q = quit");
    } else {
        snprintf(controls_line,
                 sizeof(controls_line),
                 "Controls: q = quit");
    }

    print_double_box_border(stat_width, stat_width);
    print_double_box_row(title_left, stat_width, title_right, stat_width);
    print_double_box_border(stat_width, stat_width);
    print_status_bar_row(self->hp, self->potions, enemy->hp, enemy->potions, stat_width);
    print_double_box_row(left_line1, stat_width, right_line1, stat_width);
    print_double_box_row(left_line2, stat_width, right_line2, stat_width);
    print_double_box_border(stat_width, stat_width);
    print_double_box_row(left_status_line, stat_width, countdown_line, stat_width);
    print_double_box_border(stat_width, stat_width);
    print_single_box_row(controls_line, full_width);
    print_single_box_border(full_width);
>>>>>>> 642be32 (Refine terminal HUD and result banners)
    printf("\n");

    printf("+");
    for (col = 0; col < GRID_WIDTH * 7; col++) {
        printf("=");
    }
    printf("+\n");

    for (row = 0; row < GRID_HEIGHT; row++) {
        printf("|");

        for (col = 0; col < GRID_WIDTH; col++) {
<<<<<<< HEAD
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
=======
            const char *cell = "       ";

            if (game->bullet1_active && row == game->bullet1_row && col == game->bullet1_col) {
                cell = game->p1_result == RESULT_SHOT_CRIT ? "   $   " : "   v   ";
            } else if (game->bullet2_active && row == game->bullet2_row && col == game->bullet2_col) {
                cell = game->p2_result == RESULT_SHOT_CRIT ? "   $   " : "   ^   ";
            } else if (row == (player_id == 1 ? PLAYER1_ROW : PLAYER2_ROW) && col == self->col) {
                if (self_char == '1') {
                    cell = "   1   ";
                } else {
                    cell = "   2   ";
                }
            } else if (enemy_visible &&
                       row == (player_id == 1 ? PLAYER2_ROW : PLAYER1_ROW) &&
                       col == enemy->col) {
                if (enemy_char == '1') {
                    cell = "   1   ";
                } else {
                    cell = "   2   ";
                }
            }

            printf("%s", cell);
>>>>>>> 642be32 (Refine terminal HUD and result banners)
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
<<<<<<< HEAD
        if (game->winner == 3) {
            printf("Result: draw.\n");
        } else {
            printf("Result: Player %d wins.\n", game->winner);
        }
    }

    fflush(stdout);
}
=======
        print_result_banner(game->winner, player_id);
        if (game->winner == 3) {
            printf("The duel ends in a draw.\n");
        } else if (game->winner == player_id) {
            printf("You win the duel.\n");
        } else {
            printf("You lose the duel.\n");
        }
    }

    if (quit_armed) {
        printf("Press q again to quit.\n");
    }

    fflush(stdout);
}
>>>>>>> 194727f (Refine terminal HUD and result banners)
>>>>>>> 642be32 (Refine terminal HUD and result banners)
