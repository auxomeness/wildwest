#ifndef PLAYER_H
#define PLAYER_H

/* Player action choices during the action phase. */
typedef enum {
    ACTION_NONE = 0,
    ACTION_SHOOT = 1,
    ACTION_HEAL = 2
} Action;

/* Round outcome labels shown during resolve. */
typedef enum {
    RESULT_NONE = 0,
    RESULT_SHOT_HIT = 1,
    RESULT_SHOT_CRIT = 2,
    RESULT_SHOT_MISS = 3,
    RESULT_HEAL = 4,
    RESULT_HEAL_FAIL = 5
} ResolveResult;

/* Runtime state for one player in the duel. */
typedef struct {
    int col;
    int hp;
    int potions;
    int crit_chance;
    Action action;
    int locked;
} Player;

/* Player lifecycle and action helpers. */
void player_init(Player *player, int start_col);
void player_move(Player *player, int delta, int min_col, int max_col);
void player_lock(Player *player);
void player_unlock(Player *player);
void player_clear_action(Player *player);
void player_set_action(Player *player, Action action);
ResolveResult player_apply_shot(Player *shooter, Player *target);
ResolveResult player_apply_heal(Player *player);

#endif
