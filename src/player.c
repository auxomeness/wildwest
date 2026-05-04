#include "game.h"
#include "player.h"
#include <stdlib.h>

void player_init(Player *player, int start_col)
{
    /* Every duel starts from the same base stats. */
    player->col = start_col;
    player->hp = MAX_HP;
    player->potions = MAX_POTIONS;
    player->crit_chance = DEFAULT_CRIT_CHANCE;
    player->action = ACTION_NONE;
    player->locked = 0;
}

void player_move(Player *player, int delta, int min_col, int max_col)
{
    /* Clamp movement so the player never leaves the visible arena. */
    int next_col = player->col + delta;

    if (next_col < min_col) {
        next_col = min_col;
    }

    if (next_col > max_col) {
        next_col = max_col;
    }

    player->col = next_col;
}

void player_lock(Player *player)
{
    player->locked = 1;
}

void player_unlock(Player *player)
{
    player->locked = 0;
}

void player_clear_action(Player *player)
{
    player->action = ACTION_NONE;
}

void player_set_action(Player *player, Action action)
{
    player->action = action;
}

ResolveResult player_apply_shot(Player *shooter, Player *target)
{
    /* Shooting checks alignment, then resolves hit, crit, or backfire. */
    if (shooter->action != ACTION_SHOOT) {
        return RESULT_NONE;
    }

    if (shooter->col == target->col) {
        if ((rand() % 100) < shooter->crit_chance) {
            target->hp -= CRIT_DAMAGE;
            if (target->hp < 0) {
                target->hp = 0;
            }

            shooter->crit_chance = DEFAULT_CRIT_CHANCE;

            return RESULT_SHOT_CRIT;
        }

        /* Crit Chance stacks for every unsuccessful crit */
        if (shooter->crit_chance < 65) {
            shooter->crit_chance += 13;
        }

        target->hp -= SHOT_DAMAGE;
        if (target->hp < 0) {
            target->hp = 0;
        }
        return RESULT_SHOT_HIT;
    }

    shooter->hp -= MISS_DAMAGE;

    if (shooter->hp < 0) {
        shooter->hp = 0;
    }

    return RESULT_SHOT_MISS;
}

ResolveResult player_apply_heal(Player *player)
{
    /* Healing only works if the player has a potion and is not at full HP. */
    if (player->action != ACTION_HEAL) {
        return RESULT_NONE;
    }

    if (player->hp <= 0) {
        return RESULT_HEAL_FAIL;
    }

    if (player->potions <= 0) {
        return RESULT_HEAL_FAIL;
    }

    if (player->hp >= MAX_HP) {
        return RESULT_HEAL_FAIL;
    }

    player->hp += HEAL_AMOUNT;
    if (player->hp > MAX_HP) {
        player->hp = MAX_HP;
    }

    player->potions -= 1;
    return RESULT_HEAL;
}
