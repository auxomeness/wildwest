#include "game.h"
#include "ultimate.h"
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
    game->p1_ult_result = RESULT_NONE;
    game->p2_ult_result = RESULT_NONE;
}

static void clear_sudden_death_bullets(GameState *game)
{
    int index;

    for (index = 0; index < SUDDEN_DEATH_MAX_AMMO; index++) {
        game->p1_sd_bullet_row[index] = 0;
        game->p1_sd_bullet_col[index] = 0;
        game->p1_sd_bullet_active[index] = 0;
        game->p1_sd_bullet_step_ms[index] = 0;
        game->p2_sd_bullet_row[index] = 0;
        game->p2_sd_bullet_col[index] = 0;
        game->p2_sd_bullet_active[index] = 0;
        game->p2_sd_bullet_step_ms[index] = 0;
    }
}

static void clear_damage_feedback(GameState *game)
{
    game->p1_damage_feedback = 0;
    game->p2_damage_feedback = 0;
    game->p1_damage_feedback_ms = 0;
    game->p2_damage_feedback_ms = 0;
}

static void update_damage_feedback(GameState *game, int delta_ms)
{
    if (game->p1_damage_feedback_ms > 0) {
        game->p1_damage_feedback_ms -= delta_ms;
        if (game->p1_damage_feedback_ms <= 0) {
            game->p1_damage_feedback_ms = 0;
            game->p1_damage_feedback = 0;
        }
    }

    if (game->p2_damage_feedback_ms > 0) {
        game->p2_damage_feedback_ms -= delta_ms;
        if (game->p2_damage_feedback_ms <= 0) {
            game->p2_damage_feedback_ms = 0;
            game->p2_damage_feedback = 0;
        }
    }
}

static void add_damage_feedback(GameState *game, int player_id, int damage)
{
    int *feedback = player_id == 1 ? &game->p1_damage_feedback : &game->p2_damage_feedback;
    int *feedback_ms = player_id == 1 ? &game->p1_damage_feedback_ms : &game->p2_damage_feedback_ms;

    if (damage <= 0) {
        return;
    }

    *feedback = (*feedback_ms > 0) ? (*feedback + damage) : damage;
    *feedback_ms = DAMAGE_FEEDBACK_MS;
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

static int current_grid_height(const GameState *game)
{
    if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
        return SUDDEN_DEATH_GRID_HEIGHT;
    }

    return GRID_HEIGHT;
}

static int current_player1_row(const GameState *game)
{
    if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
        return 1;
    }

    return PLAYER1_ROW;
}

static int current_player2_row(const GameState *game)
{
    if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
        return current_grid_height(game) - 2;
    }

    return PLAYER2_ROW;
}

static void clear_sudden_death_vote(GameState *game)
{
    game->p1_sudden_death_vote = 0;
    game->p2_sudden_death_vote = 0;
    game->p1_sudden_death_vote_locked = 0;
    game->p2_sudden_death_vote_locked = 0;
}

static void reset_sudden_death_battle(GameState *game)
{
    game->p1.hp = MAX_HP;
    game->p2.hp = MAX_HP;
    game->p1.col = GRID_WIDTH / 2;
    game->p2.col = GRID_WIDTH / 2;
    game->p1.action = ACTION_NONE;
    game->p2.action = ACTION_NONE;
    game->p1.locked = 0;
    game->p2.locked = 0;
    game->p1_ammo = SUDDEN_DEATH_MAX_AMMO;
    game->p2_ammo = SUDDEN_DEATH_MAX_AMMO;
    game->p1_reload_ms = 0;
    game->p2_reload_ms = 0;
    game->p1_bullet_step_ms = 0;
    game->p2_bullet_step_ms = 0;
    clear_bullets(game);
    clear_sudden_death_bullets(game);
    clear_damage_feedback(game);
    clear_results(game);
}

void game_start_sudden_death_offer(GameState *game)
{
    clear_sudden_death_vote(game);
    player_unlock(&game->p1);
    player_unlock(&game->p2);
    game->phase = PHASE_SUDDEN_DEATH_OFFER;
    game->phase_time_ms = 0;
}

void game_start_sudden_death_ready(GameState *game)
{
    reset_sudden_death_battle(game);
    game->p1_ready = 0;
    game->p2_ready = 0;
    game->phase = PHASE_SUDDEN_DEATH_READY;
    game->phase_time_ms = 0;
}

void game_start_sudden_death_battle(GameState *game)
{
    clear_bullets(game);
    game->phase = PHASE_SUDDEN_DEATH_BATTLE;
    game->phase_time_ms = 0;
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

    printf("%s", label);
    printf("%s", color_code);
    for (index = 0; index < bar_slots; index++) {
        printf("%s", index < filled ? "█" : "░");
    }
    printf("\033[0m");
}

static void print_status_bar_cell(const char *label, int hp, int secondary_value,
                                  int secondary_max, const char *secondary_label,
                                  const char *secondary_color, int width)
{
    int visible_width = 0;
    int hp_percent = (hp * 100) / MAX_HP;

    printf("| ");

    printf("%-5s ", label);
    visible_width += 6;

    print_resource_bar("HP:", hp, MAX_HP, "\033[92m");
    visible_width += 8;

    printf(" %3d%%", hp_percent);
    visible_width += 5;

    printf(" ");
    visible_width += 1;

    print_resource_bar(secondary_label, secondary_value, secondary_max, secondary_color);
    visible_width += 7;

    while (visible_width < width) {
        putchar(' ');
        visible_width++;
    }

    printf(" ");
}

static void print_status_bar_row(int left_hp, int left_secondary, int right_hp, int right_secondary,
                                 int secondary_max, const char *secondary_label,
                                 const char *secondary_color, int width)
{
    print_status_bar_cell("YOU", left_hp, left_secondary, secondary_max,
                          secondary_label, secondary_color, width);
    print_status_bar_cell("ENEMY", right_hp, right_secondary, secondary_max,
                          secondary_label, secondary_color, width);
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

static void print_sudden_death_banner(void)
{
    static const char *sudden_death_lines[] = {
        "  ********   **     **   *******     *******     ********   ****     **",
        " **//////   /**    /**  /**////**   /**////**   /**/////   /**/**   /**",
        "/**         /**    /**  /**    /**  /**    /**  /**        /**//**  /**",
        "/*********  /**    /**  /**    /**  /**    /**  /*******   /** //** /**",
        "////////**  /**    /**  /**    /**  /**    /**  /**////    /**  //**/**",
        "       /**  /**    /**  /**    **   /**    **   /**        /**   //****",
        " ********   //*******   /*******    /*******    /********  /**    //***",
        "////////     ///////    ///////     ///////     ////////   //      /// ",
        " *******     ********       **       **********   **      **",
        "/**////**   /**/////       ****     /////**///   /**     /**",
        "/**    /**  /**           **//**        /**      /**     /**",
        "/**    /**  /*******     **  //**       /**      /**********",
        "/**    /**  /**////     **********      /**      /**//////**",
        "/**    **   /**        /**//////**      /**      /**     /**",
        "/*******    /********  /**     /**      /**      /**     /**",
        "///////     ////////   //      //       //       //      // "
    };
    int index;

    for (index = 0; index < (int)(sizeof(sudden_death_lines) / sizeof(sudden_death_lines[0])); index++) {
        print_banner_line(sudden_death_lines[index], 71);
    }
}

static void print_title_banner(void)
{
    static const char *title_lines[] = {
        " **       **   **   **         *******   ",
        "/**      /**  /**  /**        /**////**  ",
        "/**   *  /**  /**  /**        /**    /**",
        "/**  *** /**  /**  /**        /**    /**",
        "/** **/**/**  /**  /**        /**    /**",
        "/**** //****  /**  /**        /**    **  ",
        "/**/   ///**  /**  /********  /*******   ",
        "//       //   //   ////////   ///////    ",
        " **       **   ********    ********   **********",
        "/**      /**  /**/////    **//////   /////**/// ",
        "/**   *  /**  /**        /**             /**    ",
        "/**  *** /**  /*******   /*********      /**    ",
        "/** **/**/**  /**////    ////////**      /**    ",
        "/**** //****  /**               /**      /**    ",
        "/**/   ///**  /********   ********       /**    ",
        "//       //   ////////   ////////        //     "
    };
    int index;
    int padding = 6;

    for (index = 0; index < (int)(sizeof(title_lines) / sizeof(title_lines[0])); index++) {
        int column;

        for (column = 0; column < padding; column++) {
            putchar(' ');
        }

        printf("%s\n", title_lines[index]);
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
    game->sudden_death_declined = 0;
    clear_sudden_death_vote(game);
    game->p1_ammo = 0;
    game->p2_ammo = 0;
    game->p1_reload_ms = 0;
    game->p2_reload_ms = 0;
    game->p1_bullet_step_ms = 0;
    game->p2_bullet_step_ms = 0;
    clear_damage_feedback(game);
    clear_sudden_death_bullets(game);

    clear_bullets(game);
    clear_results(game);
    clear_damage_feedback(game);
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
    int p1_hp_before = game->p1.hp;
    int p2_hp_before = game->p2.hp;

    clear_bullets(game);
    clear_damage_feedback(game);

    if (game->p1.action == ACTION_NONE) {
        player_set_action(&game->p1, ACTION_SHOOT);
    }

    if (game->p2.action == ACTION_NONE) {
        player_set_action(&game->p2, ACTION_SHOOT);
    }

    if (game->p1.action == ACTION_SHOOT) {
        game->bullet1_active = 1;
        game->bullet1_row = current_player1_row(game) + 1;
        game->bullet1_col = game->p1.col;
    }

    if (game->p2.action == ACTION_SHOOT) {
        game->bullet2_active = 1;
        game->bullet2_row = current_player2_row(game) - 1;
        game->bullet2_col = game->p2.col;
    }

    game->p1_result = RESULT_NONE;
    game->p2_result = RESULT_NONE;
    game->p1_ult_result = RESULT_NONE;
    game->p2_ult_result = RESULT_NONE;

    /* Deflect shields reset each round. */
    game->p1.is_deflecting = 0;
    game->p2.is_deflecting = 0;

    /* Execute ultimates first so deflect shields are up before shots resolve. */
    if (game->p1.action == ACTION_ULTIMATE && ultimate_can_use(&game->p1)) {
        int ult = game->p1.ultimate_type;
        ultimate_execute(&game->p1, &game->p2);
        game->p1_hit_streak = 0;
        if (ult == ULT_ONE_SHOT)
            game->p1_ult_result = (game->p1.col == game->p2.col) ? RESULT_ULT_HIT : RESULT_ULT_MISS;
        else if (ult == ULT_BARRAGE)
            game->p1_ult_result = (game->p1.col >= game->p2.col - 1 && game->p1.col <= game->p2.col + 1) ? RESULT_ULT_HIT : RESULT_ULT_MISS;
        else if (ult == ULT_DEFLECT)
            game->p1_ult_result = RESULT_ULT_DEFLECT;
        else if (ult == ULT_REVEAL)
            game->p1_ult_result = RESULT_ULT_REVEAL;
    }

    if (game->p2.action == ACTION_ULTIMATE && ultimate_can_use(&game->p2)) {
        int ult = game->p2.ultimate_type;
        ultimate_execute(&game->p2, &game->p1);
        game->p2_hit_streak = 0;
        if (ult == ULT_ONE_SHOT)
            game->p2_ult_result = (game->p2.col == game->p1.col) ? RESULT_ULT_HIT : RESULT_ULT_MISS;
        else if (ult == ULT_BARRAGE)
            game->p2_ult_result = (game->p2.col >= game->p1.col - 1 && game->p2.col <= game->p1.col + 1) ? RESULT_ULT_HIT : RESULT_ULT_MISS;
        else if (ult == ULT_DEFLECT)
            game->p2_ult_result = RESULT_ULT_DEFLECT;
        else if (ult == ULT_REVEAL)
            game->p2_ult_result = RESULT_ULT_REVEAL;
    }

    /* Only apply shots for players not using their ultimate. */
    if (game->p1.action != ACTION_ULTIMATE) {
        game->p1_result = player_apply_shot(&game->p1, &game->p2);
    }

    if (game->p2.action != ACTION_ULTIMATE) {
        game->p2_result = player_apply_shot(&game->p2, &game->p1);
    }

    /* --- Ultimate charge conditions ---
     * Condition 1: land 3 consecutive hits (HIT or CRIT) on the enemy.
     *   - A miss resets the streak. Healing is neutral (neither advances nor breaks it).
     * Condition 2: own HP drops to 20% or below (20 HP on a 100 HP pool).
     * Once charged, ultimate_ready stays set until the player uses it.
     * Using the ultimate resets the streak so they must earn it again. */
    if (!game->p1.ultimate_ready) {
        if (game->p1.action == ACTION_SHOOT) {
            if (game->p1_result == RESULT_SHOT_HIT || game->p1_result == RESULT_SHOT_CRIT) {
                game->p1_hit_streak++;
            } else if (game->p1_result == RESULT_SHOT_MISS) {
                game->p1_hit_streak = 0;
            }
        }
        if (game->p1_hit_streak >= 3 || game->p1.hp <= (MAX_HP / 5)) {
            game->p1.ultimate_ready = 1;
            game->p1_hit_streak = 0;
        }
    }

    if (!game->p2.ultimate_ready) {
        if (game->p2.action == ACTION_SHOOT) {
            if (game->p2_result == RESULT_SHOT_HIT || game->p2_result == RESULT_SHOT_CRIT) {
                game->p2_hit_streak++;
            } else if (game->p2_result == RESULT_SHOT_MISS) {
                game->p2_hit_streak = 0;
            }
        }
        if (game->p2_hit_streak >= 3 || game->p2.hp <= (MAX_HP / 5)) {
            game->p2.ultimate_ready = 1;
            game->p2_hit_streak = 0;
        }
    }

    add_damage_feedback(game, 1, p1_hp_before - game->p1.hp);
    add_damage_feedback(game, 2, p2_hp_before - game->p2.hp);

    update_winner(game);

    game->phase = PHASE_RESOLVE;
    game->phase_time_ms = 0;
}

void game_update_bullets(GameState *game, int delta_ms)
{
    /* Bullet movement depends on whether the game is in resolve or sudden death. */
    update_damage_feedback(game, delta_ms);

    if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
        int index;

        if (game->p1_ammo == 0 && game->p1_reload_ms < SUDDEN_DEATH_RELOAD_MS) {
            game->p1_reload_ms += delta_ms;
            if (game->p1_reload_ms >= SUDDEN_DEATH_RELOAD_MS) {
                game->p1_ammo = SUDDEN_DEATH_MAX_AMMO;
                game->p1_reload_ms = 0;
            }
        }

        if (game->p2_ammo == 0 && game->p2_reload_ms < SUDDEN_DEATH_RELOAD_MS) {
            game->p2_reload_ms += delta_ms;
            if (game->p2_reload_ms >= SUDDEN_DEATH_RELOAD_MS) {
                game->p2_ammo = SUDDEN_DEATH_MAX_AMMO;
                game->p2_reload_ms = 0;
            }
        }

        for (index = 0; index < SUDDEN_DEATH_MAX_AMMO; index++) {
            if (game->p1_sd_bullet_active[index]) {
                game->p1_sd_bullet_step_ms[index] += delta_ms;
                while (game->p1_sd_bullet_active[index] &&
                       game->p1_sd_bullet_step_ms[index] >= SUDDEN_DEATH_BULLET_STEP_MS) {
                    game->p1_sd_bullet_step_ms[index] -= SUDDEN_DEATH_BULLET_STEP_MS;
                    game->p1_sd_bullet_row[index] += 1;

                    if (game->p1_sd_bullet_row[index] >= current_player2_row(game)) {
                        if (game->p1_sd_bullet_col[index] == game->p2.col) {
                            game->p2.hp -= SUDDEN_DEATH_DAMAGE;
                            if (game->p2.hp < 0) {
                                game->p2.hp = 0;
                            }
                            add_damage_feedback(game, 2, SUDDEN_DEATH_DAMAGE);
                        }
                        game->p1_sd_bullet_active[index] = 0;
                    }
                }
            }

            if (game->p2_sd_bullet_active[index]) {
                game->p2_sd_bullet_step_ms[index] += delta_ms;
                while (game->p2_sd_bullet_active[index] &&
                       game->p2_sd_bullet_step_ms[index] >= SUDDEN_DEATH_BULLET_STEP_MS) {
                    game->p2_sd_bullet_step_ms[index] -= SUDDEN_DEATH_BULLET_STEP_MS;
                    game->p2_sd_bullet_row[index] -= 1;

                    if (game->p2_sd_bullet_row[index] <= current_player1_row(game)) {
                        if (game->p2_sd_bullet_col[index] == game->p1.col) {
                            game->p1.hp -= SUDDEN_DEATH_DAMAGE;
                            if (game->p1.hp < 0) {
                                game->p1.hp = 0;
                            }
                            add_damage_feedback(game, 1, SUDDEN_DEATH_DAMAGE);
                        }
                        game->p2_sd_bullet_active[index] = 0;
                    }
                }
            }
        }

        update_winner(game);
        if (game->winner == 3) {
            game->winner = 0;
            game_start_sudden_death_ready(game);
        }
        return;
    }

    if (game->bullet1_active) {
        game->bullet1_row += 1;
        if (game->bullet1_row > current_player2_row(game)) {
            game->bullet1_active = 0;
        }
    }

    if (game->bullet2_active) {
        game->bullet2_row -= 1;
        if (game->bullet2_row < current_player1_row(game)) {
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

    if (phase == PHASE_SUDDEN_DEATH_OFFER) {
        return "SD OFFER";
    }

    if (phase == PHASE_SUDDEN_DEATH_READY) {
        return "SD READY";
    }

    if (phase == PHASE_SUDDEN_DEATH_BATTLE) {
        return "SUDDEN DEATH";
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

static const char *ult_type_label(int ult_type)
{
    switch (ult_type) {
        case ULT_ONE_SHOT: return "ONE SHOT";
        case ULT_BARRAGE:  return "BARRAGE";
        case ULT_DEFLECT:  return "DEFLECT";
        case ULT_REVEAL:   return "REVEAL";
        default:           return "NONE";
    }
}

static const char *ult_type_desc(int ult_type)
{
    switch (ult_type) {
        case ULT_ONE_SHOT: return "Same col = instant kill. Miss = -20 self.";
        case ULT_BARRAGE:  return "Hits enemy if within 1 col. -15 HP.";
        case ULT_DEFLECT:  return "Block incoming shot this round.";
        case ULT_REVEAL:   return "Reveal enemy position this resolve.";
        default:           return "";
    }
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

    if (result == RESULT_ULT_HIT) {
        return "ULTIMATE HIT";
    }

    if (result == RESULT_ULT_MISS) {
        return "ULTIMATE MISS";
    }

    if (result == RESULT_ULT_DEFLECT) {
        return "DEFLECT READY";
    }

    if (result == RESULT_ULT_REVEAL) {
        return "POSITION REVEALED";
    }

    return "NONE";
}

static void append_display_key_int(char *buffer, size_t buffer_size, int value)
{
    size_t used = strlen(buffer);

    if (used >= buffer_size) {
        return;
    }

    snprintf(buffer + used, buffer_size - used, "|%d", value);
}

static void format_player_cell(char *buffer, size_t buffer_size, char player_char)
{
    snprintf(buffer, buffer_size, "   %c   ", player_char);
}

static void format_damage_cell(char *buffer, size_t buffer_size, int damage)
{
    if (damage > 0) {
        char text[8];

        snprintf(text, sizeof(text), "-%dHP", damage);
        snprintf(buffer, buffer_size, "%-7s", text);
    } else {
        snprintf(buffer, buffer_size, "       ");
    }
}

static int damage_feedback_col(int player_col)
{
    if (player_col < GRID_WIDTH - 1) {
        return player_col + 1;
    }

    return player_col - 1;
}

void game_build_display_key(const GameState *game, int player_id, char *buffer, size_t buffer_size)
{
    /* Used to avoid redrawing the same frame repeatedly. */
    const Player *self = local_player(game, player_id);
    const Player *enemy = enemy_player(game, player_id);
    int enemy_visible = (game->phase == PHASE_RESOLVE || game->phase == PHASE_GAME_OVER);
    int countdown = -1;
    int sd_visible = (game->phase == PHASE_SUDDEN_DEATH_BATTLE);
    int index;

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
             (enemy_visible || sd_visible) ? enemy->col : -1,
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

    snprintf(buffer + strlen(buffer),
             buffer_size - strlen(buffer),
             "|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d",
             game->p1_sudden_death_vote,
             game->p2_sudden_death_vote,
             game->p1_sudden_death_vote_locked,
             game->p2_sudden_death_vote_locked,
             game->p1_ammo,
             game->p2_ammo,
             game->p1_reload_ms,
             game->p2_reload_ms,
             game->phase == PHASE_SUDDEN_DEATH_BATTLE,
             game->sudden_death_declined);

    append_display_key_int(buffer, buffer_size, game->p1_damage_feedback);
    append_display_key_int(buffer, buffer_size, game->p2_damage_feedback);

    for (index = 0; index < SUDDEN_DEATH_MAX_AMMO; index++) {
        append_display_key_int(buffer, buffer_size, game->p1_sd_bullet_row[index]);
        append_display_key_int(buffer, buffer_size, game->p1_sd_bullet_col[index]);
        append_display_key_int(buffer, buffer_size, game->p1_sd_bullet_active[index]);
        append_display_key_int(buffer, buffer_size, game->p2_sd_bullet_row[index]);
        append_display_key_int(buffer, buffer_size, game->p2_sd_bullet_col[index]);
        append_display_key_int(buffer, buffer_size, game->p2_sd_bullet_active[index]);
    }
}

void game_render(const GameState *game, int player_id, int quit_armed)
{
    /* Render one full terminal frame for the requested player perspective. */
    const Player *self = local_player(game, player_id);
    const Player *enemy = enemy_player(game, player_id);
    ResolveResult self_result = local_result(game, player_id);
    ResolveResult foe_result = enemy_result(game, player_id);
    char self_char = player_id == 1 ? '1' : '2';
    char enemy_char = player_id == 1 ? '2' : '1';
    int enemy_visible = (game->phase == PHASE_RESOLVE ||
                         game->phase == PHASE_GAME_OVER ||
                         game->phase == PHASE_SUDDEN_DEATH_BATTLE);
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
    char p1_cell[8];
    char p2_cell[8];
    char p1_damage_cell[8];
    char p2_damage_cell[8];
    const int stat_width = 33;
    const int full_width = (stat_width * 2) + 3;
    int arena_height = current_grid_height(game);
    int player1_row = current_player1_row(game);
    int player2_row = current_player2_row(game);
    int show_arena = (game->phase != PHASE_SUDDEN_DEATH_READY);
    int show_banner = (game->phase == PHASE_SUDDEN_DEATH_READY) &&
                      ((game->phase_time_ms / 300) % 2 == 0);
    int left_secondary = self->potions;
    int right_secondary = enemy->potions;
    int secondary_max = MAX_POTIONS;
    const char *secondary_label = "P:";
    const char *secondary_color = "\033[91m";
    int self_sd_vote = player_id == 1 ? game->p1_sudden_death_vote : game->p2_sudden_death_vote;
    int enemy_sd_vote = player_id == 1 ? game->p2_sudden_death_vote : game->p1_sudden_death_vote;
    int self_sd_locked = player_id == 1 ? game->p1_sudden_death_vote_locked : game->p2_sudden_death_vote_locked;
    int enemy_sd_locked = player_id == 1 ? game->p2_sudden_death_vote_locked : game->p1_sudden_death_vote_locked;
    int row;
    int col;

    printf("\033[2J\033[H");

    format_player_cell(p1_cell, sizeof(p1_cell), '1');
    format_player_cell(p2_cell, sizeof(p2_cell), '2');
    format_damage_cell(p1_damage_cell, sizeof(p1_damage_cell), game->p1_damage_feedback);
    format_damage_cell(p2_damage_cell, sizeof(p2_damage_cell), game->p2_damage_feedback);

    if (game->phase == PHASE_WAITING) {
        printf("\n\n");
        print_title_banner();
        printf("\n");
        print_banner_line("PRESS SPACE BAR TO START", 77);
        if (self_ready) {
            print_banner_line("Waiting for the other player...", 77);
        }
        if (quit_armed) {
            printf("\nPress q again to quit.\n");
        }
        fflush(stdout);
        return;
    }

    snprintf(title_left,
             sizeof(title_left),
             "Wild West Quick Draw");
    snprintf(title_right,
             sizeof(title_right),
             "Player %d",
             player_id);

    if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
        left_secondary = player_id == 1 ? game->p1_ammo : game->p2_ammo;
        right_secondary = player_id == 1 ? game->p2_ammo : game->p1_ammo;
        secondary_max = SUDDEN_DEATH_MAX_AMMO;
        secondary_label = "A:";
        secondary_color = "\033[93m";
    }

    if (game->phase == PHASE_SUDDEN_DEATH_OFFER) {
        snprintf(left_line1, sizeof(left_line1), "YOU   Vote: %-3s Locked:[%c]",
                 self_sd_vote ? "YES" : "NO",
                 self_sd_locked ? 'x' : ' ');
        snprintf(left_line2, sizeof(left_line2), "Need two YES votes");
        snprintf(right_line1, sizeof(right_line1), "ENEMY Vote: %-3s Locked:[%c]",
                 enemy_sd_vote ? "YES" : "NO",
                 enemy_sd_locked ? 'x' : ' ');
        snprintf(right_line2, sizeof(right_line2), "Need two YES votes");
    } else if (game->phase == PHASE_SUDDEN_DEATH_READY) {
        snprintf(left_line1, sizeof(left_line1), "YOU   Ready: [%c]", self_ready ? 'x' : ' ');
        snprintf(left_line2, sizeof(left_line2), "HP reset. Ammo reset.");
        snprintf(right_line1, sizeof(right_line1), "ENEMY Ready: [%c]", enemy_ready ? 'x' : ' ');
        snprintf(right_line2, sizeof(right_line2), "HP reset. Ammo reset.");
    } else if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
        snprintf(left_line1, sizeof(left_line1), "YOU   Reload: %d.%ds",
                 (player_id == 1 ? game->p1_reload_ms : game->p2_reload_ms) / 1000,
                 ((player_id == 1 ? game->p1_reload_ms : game->p2_reload_ms) % 1000) / 100);
        snprintf(left_line2, sizeof(left_line2), "Visible duel. First kill wins.");
        snprintf(right_line1, sizeof(right_line1), "ENEMY Reload: %d.%ds",
                 (player_id == 1 ? game->p2_reload_ms : game->p1_reload_ms) / 1000,
                 ((player_id == 1 ? game->p2_reload_ms : game->p1_reload_ms) % 1000) / 100);
        snprintf(right_line2, sizeof(right_line2), "Visible duel. First kill wins.");
    } else {
        snprintf(left_line1,
                 sizeof(left_line1),
                 "YOU   Locked: %-3s   Ready: [%c]",
                 self->locked ? "YES" : "NO",
                 self_ready ? 'x' : ' ');
        {
            int self_streak = player_id == 1 ? game->p1_hit_streak : game->p2_hit_streak;
            if (self->ultimate_ready) {
                snprintf(left_line2, sizeof(left_line2),
                         "Action: %-5s  [ULT READY!]",
                         game_action_label(self->action));
            } else {
                snprintf(left_line2, sizeof(left_line2),
                         "Action: %-5s  Streak:%d/3",
                         game_action_label(self->action),
                         self_streak);
            }
        }
        snprintf(right_line1,
                 sizeof(right_line1),
                 "ENEMY Locked: %-3s   Ready: [%c]",
                 enemy->locked ? "YES" : "NO",
                 enemy_ready ? 'x' : ' ');
        if (game->phase != PHASE_ACTION && enemy->ultimate_ready) {
            snprintf(right_line2, sizeof(right_line2),
                     "Action: %-5s  [ULT READY!]",
                     game_action_label(enemy->action));
        } else {
            snprintf(right_line2, sizeof(right_line2),
                     "Action: %s",
                     game->phase == PHASE_ACTION ? "HIDDEN" : game_action_label(enemy->action));
        }
    }
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
        if (self->ultimate_ready && self->action == ACTION_ULTIMATE) {
            snprintf(controls_line, sizeof(controls_line),
                     "Controls: Left/Right = cycle ult | Space = lock | S = Shoot | H = Heal | q = quit");
        } else if (self->ultimate_ready) {
            snprintf(controls_line, sizeof(controls_line),
                     "Controls: Left = shoot | Right = heal | U = ultimate | Space = lock | q = quit");
        } else {
            snprintf(controls_line, sizeof(controls_line),
                     "Controls: Left = shoot | Right = heal | Space = lock | q = quit");
        }
    } else if (game->phase == PHASE_SUDDEN_DEATH_OFFER) {
        snprintf(controls_line,
                 sizeof(controls_line),
                 "Controls: Left = YES | Right = NO | Space = lock | q = quit");
    } else if (game->phase == PHASE_SUDDEN_DEATH_READY) {
        snprintf(controls_line,
                 sizeof(controls_line),
                 "Controls: Space = ready | q = quit");
    } else if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
        snprintf(controls_line,
                 sizeof(controls_line),
                 "Controls: Left/Right = move | Space = shoot | q = quit");
    } else {
        snprintf(controls_line,
                 sizeof(controls_line),
                 "Controls: q = quit");
    }

    print_double_box_border(stat_width, stat_width);
    print_double_box_row(title_left, stat_width, title_right, stat_width);
    print_double_box_border(stat_width, stat_width);
    print_status_bar_row(self->hp, left_secondary, enemy->hp, right_secondary,
                         secondary_max, secondary_label, secondary_color, stat_width);
    print_double_box_row(left_line1, stat_width, right_line1, stat_width);
    print_double_box_row(left_line2, stat_width, right_line2, stat_width);
    print_double_box_border(stat_width, stat_width);
    print_double_box_row(left_status_line, stat_width, countdown_line, stat_width);
    print_double_box_border(stat_width, stat_width);
    print_single_box_row(controls_line, full_width);
    print_single_box_border(full_width);
    printf("\n");

    if (show_arena) {
        printf("+");
        for (col = 0; col < GRID_WIDTH * 7; col++) {
            printf("=");
        }
        printf("+\n");

        for (row = 0; row < arena_height; row++) {
            printf("|");

            for (col = 0; col < GRID_WIDTH; col++) {
                const char *cell = "       ";
                int cell_used = 0;
                int index;

                if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
                    for (index = 0; index < SUDDEN_DEATH_MAX_AMMO; index++) {
                        if (game->p1_sd_bullet_active[index] &&
                            row == game->p1_sd_bullet_row[index] &&
                            col == game->p1_sd_bullet_col[index]) {
                            cell = "   $   ";
                            cell_used = 1;
                            break;
                        }

                        if (game->p2_sd_bullet_active[index] &&
                            row == game->p2_sd_bullet_row[index] &&
                            col == game->p2_sd_bullet_col[index]) {
                            cell = "   $   ";
                            cell_used = 1;
                            break;
                        }
                    }
                }

                if (!cell_used && game->bullet1_active && row == game->bullet1_row && col == game->bullet1_col) {
                    cell = (game->phase == PHASE_SUDDEN_DEATH_BATTLE) ? "   $   " :
                           (game->p1_result == RESULT_SHOT_CRIT ? "   $   " : "   v   ");
                } else if (!cell_used && game->bullet2_active && row == game->bullet2_row && col == game->bullet2_col) {
                    cell = (game->phase == PHASE_SUDDEN_DEATH_BATTLE) ? "   $   " :
                           (game->p2_result == RESULT_SHOT_CRIT ? "   $   " : "   ^   ");
                } else if (!cell_used && row == (player_id == 1 ? player1_row : player2_row) && col == self->col) {
                    cell = self_char == '1' ? p1_cell : p2_cell;
                } else if (!cell_used &&
                           enemy_visible &&
                           row == (player_id == 1 ? player2_row : player1_row) &&
                           col == enemy->col) {
                    cell = enemy_char == '1' ? p1_cell : p2_cell;
                } else if (!cell_used &&
                           game->p1_damage_feedback > 0 &&
                           enemy_visible &&
                           row == player1_row &&
                           col == damage_feedback_col(game->p1.col)) {
                    cell = p1_damage_cell;
                } else if (!cell_used &&
                           game->p2_damage_feedback > 0 &&
                           enemy_visible &&
                           row == player2_row &&
                           col == damage_feedback_col(game->p2.col)) {
                    cell = p2_damage_cell;
                }

                printf("%s", cell);
            }

            printf("|\n");
        }

        printf("+");
        for (col = 0; col < GRID_WIDTH * 7; col++) {
            printf("=");
        }
        printf("+\n");
    } else {
        printf("\n\n\n");
    }

    printf("\n");

    if (game->phase == PHASE_WAITING) {
        printf("Press Space when ready. The duel starts when both players are ready.\n");
    } else if (game->phase == PHASE_MOVE) {
        printf("Enemy position is hidden. Choose a lane with Left/Right Arrow, then press Space to lock.\n");
    } else if (game->phase == PHASE_ACTION) {
        if (self->ultimate_ready) {
            if (self->action == ACTION_ULTIMATE) {
                /* Show ult type picker */
                printf("SHOOT [ ]      HEAL [ ]      ULTIMATE [x]\n");
                printf("  Select ultimate: < %s >   %s\n",
                       ult_type_label(self->ultimate_type),
                       ult_type_desc(self->ultimate_type));
                printf("Left/Right = cycle ult | Space = lock | S = Shoot | H = Heal\n");
            } else {
                printf("SHOOT [%c]      HEAL [%c]      ULTIMATE [ ]\n",
                       self->action == ACTION_SHOOT ? 'x' : ' ',
                       self->action == ACTION_HEAL ? 'x' : ' ');
                printf("Left = Shoot | Right = Heal | U = select Ultimate | Space = lock\n");
            }
        } else {
            printf("SHOOT [%c]      HEAL [%c]\n",
                   self->action == ACTION_SHOOT ? 'x' : ' ',
                   self->action == ACTION_HEAL ? 'x' : ' ');
            printf("Enemy position is still hidden. Use Left for Shoot and Right for Heal, then press Space to lock.\n");
        }
    } else if (game->phase == PHASE_SUDDEN_DEATH_OFFER) {
        print_sudden_death_banner();
        printf("Both players are at %d HP or below. Enter sudden death only if both vote YES.\n",
               SUDDEN_DEATH_THRESHOLD);
        printf("SUDDEN DEATH [%c]      CONTINUE [%c]\n",
               self_sd_vote == 1 ? 'x' : ' ',
               self_sd_vote == 0 ? 'x' : ' ');
        printf("You locked: [%c] | Enemy locked: [%c]\n",
               self_sd_locked ? 'x' : ' ',
               enemy_sd_locked ? 'x' : ' ');
    } else if (game->phase == PHASE_SUDDEN_DEATH_READY) {
        if (show_banner) {
            print_sudden_death_banner();
        }
        printf("Both players must press Space to begin sudden death.\n");
        printf("Ready: You [%c] | Enemy [%c]\n",
               self_ready ? 'x' : ' ',
               enemy_ready ? 'x' : ' ');
    } else if (game->phase == PHASE_SUDDEN_DEATH_BATTLE) {
        printf("Sudden death is live. Both players are visible and the first kill wins.\n");
        printf("Ammo reloads to %d after %d seconds when empty.\n",
               SUDDEN_DEATH_MAX_AMMO,
               SUDDEN_DEATH_RELOAD_MS / 1000);
    } else if (game->phase == PHASE_RESOLVE) {
        ResolveResult self_ult = player_id == 1 ? game->p1_ult_result : game->p2_ult_result;
        ResolveResult foe_ult  = player_id == 1 ? game->p2_ult_result : game->p1_ult_result;
        if (self_ult != RESULT_NONE || foe_ult != RESULT_NONE) {
            printf("Ultimate: you %s | enemy %s\n",
                   self_ult != RESULT_NONE ? game_result_label(self_ult) : "---",
                   foe_ult  != RESULT_NONE ? game_result_label(foe_ult)  : "---");
        }
        printf("Resolve: you %s | enemy %s\n",
               game_result_label(self_result),
               game_result_label(foe_result));
    } else if (game->phase == PHASE_GAME_OVER) {
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
