#include "ultimate.h"
#include "player.h"
#include <stdio.h>

void ultimate_set(Player* p, UltimateType type) {
    p->ultimate_type = type;
}

int ultimate_can_use(Player* p) {
    return p->ultimate_ready == 1;
}

void ultimate_execute(Player* user, Player* enemy) {
    if (!user->ultimate_ready)
        return;

    switch (user->ultimate_type) {
        case ULT_ONE_SHOT:
            ult_one_shot(user, enemy);
            break;
        case ULT_BARRAGE:
            ult_barrage(user, enemy);
            break;
        case ULT_DEFLECT:
            ult_deflect(user, enemy);
            break;
        case ULT_REVEAL:
            ult_reveal(user, enemy);
            break;
        default:
            return;
    }

    user->ultimate_ready = 0;
}

// ULTIMATE: ONE SHOT
void ult_one_shot(Player* user, Player* enemy) {
    if (user->col == enemy->col) {
        if (enemy->is_deflecting) {
            user->hp -= 100;
            if (user->hp < 0) {
                user->hp = 0;
            }
            return;
        }

        enemy->hp -= 100;
        if (enemy->hp < 0) {
            enemy->hp = 0;
        }
    } else {
        user->hp -= 10;
        if (user->hp < 0) {
            user->hp = 0;
        }
    }
}

// ULTIMATE: THREE COLUMN BARRAGE
void ult_barrage(Player* user, Player* enemy) {
    (void)user;
    (void)enemy;
}

// ULTIMATE: DEFLECT
void ult_deflect(Player* user, Player* enemy) {
    (void)enemy;
    user->is_deflecting = 1;
}

// ULTIMATE: REVEAL POSITION
void ult_reveal(Player* user, Player* enemy) {
    (void)user;
    (void)enemy;
}
