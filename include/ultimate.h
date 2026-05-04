#ifndef ULTIMATE_H
#define ULTIMATE_H

#include "player.h"

typedef enum {
    ULT_NONE,
    ULT_ONE_SHOT,
    ULT_BARRAGE,
    ULT_DEFLECT,
    ULT_REVEAL
} UltimateType;

// ULTIMATE SYSTEM API
void ultimate_set(Player* p, UltimateType type);
int ultimate_can_use(Player* p);
void ultimate_execute(Player* user, Player* enemy);

#endif
