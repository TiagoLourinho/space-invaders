#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "game_def.h"
#include <stdlib.h>

void place_player(player_t *player);

void place_alien(alien_t *alien);

void init_game(game_t *game);

#endif // SERVER_UTILS_H