#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "game_def.h"
#include "ncurses_wrapper.h"
#include <ncurses.h>
#include <stdlib.h>

void place_player(player_t *player);

void place_alien(alien_t *alien);

void init_game(game_t *game);

int find_position_and_init_player(game_t *game);

void update_player_position(player_t *player, MOVEMENT_DIRECTION direction);

void player_zap(WINDOW *win, game_t *game, int player_id);

#endif // SERVER_UTILS_H