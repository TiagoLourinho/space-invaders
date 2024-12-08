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

/******************** Client requests handling ********************/

/* Handles the state and screen updates when a player connects */
void handle_player_connect(
    WINDOW *game_window,
    astronaut_connect_response_t *astronaut_connect_response, int *tokens,
    game_t *game);

/* Handles the state and screen updates when a player makes an action */
void handle_player_action(action_request_t *action_request,
                          player_t *current_player, WINDOW *game_window,
                          game_t *game);

/* Handles the state and screen updates when a player disconnects */
void handle_player_disconnect(WINDOW *game_window, player_t *current_player);

/* Handles the state and screen updates when the aliens positions are updated */
void handle_aliens_updates(WINDOW *game_window,
                           aliens_update_request_t *alien_update_request,
                           game_t *game);

/******************** Aliens management ********************/

/* Spawns the process responsible for updating the aliens */
void spawn_alien_update_fork(alien_t *aliens);

/* Places the alien on the board */
void place_alien(alien_t *alien);

/******************** Player management ********************/

/* Places the player on the board */
void place_player(player_t *player);

/* Finds an available position for a player and initializes it */
int find_position_and_init_player(game_t *game, int *tokens);

/* Updates state when a player zaps and kills the aliens */
void player_zap(WINDOW *win, game_t *game, int player_id);

/******************** Miscellaneous ********************/

/* Inits all the players and aliens on the board */
void init_game(game_t *game, int *tokens);

/* Update the position of a player or alien */
void update_position(position_t *position, MOVEMENT_DIRECTION direction);

/* Copies the game state to the connect reply */
void copy_game_state_for_display(display_connect_response_t *response,
                                 game_t *game);

/* Finds and prints the winning player */
void print_winning_player(game_t *game);

/* Converts an ID to a symbol */
char id_to_symbol(int id);

/* Returns the current timestamp in ms since epoch */
uint64_t get_timestamp_ms();

#endif // UTILS_H