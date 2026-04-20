#ifndef PLAYER_H
#define PLAYER_H


typedef struct Player Player;

Player* player_create(int start_row);
void player_move(Player* p, int direction);
void player_shoot(Player* shooter, Player* target); 
void player_heal(Player* p);


//Player State
int player_get_hp(Player* p);
int player_get_row(Player* p);

#endif
