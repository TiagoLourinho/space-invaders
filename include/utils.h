#ifndef UTILS_H
#define UTILS_H

#include "game_def.h"
#include "ncurses_wrapper.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

char id_to_symbol(int id);

uint64_t get_timestamp_ms();

void place_player(player_t *player);

void place_alien(alien_t *alien);

void init_game(game_t *game, int *tokens);

int find_position_and_init_player(game_t *game, int *tokens);

void update_position(position_t *position, MOVEMENT_DIRECTION direction);

void player_zap(WINDOW *win, game_t *game, int player_id);

void spawn_alien_update_fork(alien_t *aliens);

void copy_game_state(display_connect_response_t *response, game_t *game);

void handle_player_connect(
    WINDOW *game_window,
    astronaut_connect_response_t *astronaut_connect_response, int *tokens,
    game_t *game);

void handle_player_action(action_request_t *action_request,
                          player_t *current_player, WINDOW *game_window,
                          game_t *game);

void handle_player_disconnect(WINDOW *game_window, player_t *current_player);

void handle_aliens_updates(WINDOW *game_window,
                           aliens_update_request_t *alien_update_request,
                           game_t *game);
#endif // UTILS_H