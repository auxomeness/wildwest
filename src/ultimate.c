#include "./include/ultimate.h"
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

// ULTIMATE: ONE SHOT ONE KILL
void ult_one_shot(Player* user, Player* enemy) {

    if (user->row == enemy->row) {
        enemy->hp = 0;
    } else {
        user->hp -= 20; 
    }
}


// ULTIMATE: THREE ROW BARRAGE
void ult_barrage(Player* user, Player* enemy) {

    int r = enemy->row;

    if (user->row >= r - 1 && user->row <= r + 1) {
        enemy->hp -= 15;
    }
}


// ULTIMATE: DEFLECT
void ult_deflect(Player* user, Player* enemy) {
    user->is_deflecting = 1;
}


// ULTIMATE: REVEAL POSITION
void ult_reveal(Player* user, Player* enemy) {

    printf("[ULTIMATE REVEAL] Enemy is at row: %d\n", enemy->row);
}
