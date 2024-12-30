#ifndef NCURSES_WRAPPER_H
#define NCURSES_WRAPPER_H

#include "game_def.h"
#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils.h>

/* Converts a game position to a position on the window (considering the
 * border)*/
#define POS_TO_WIN(val) ((val) + 1)

/******************** Initialization functions ********************/

/* Initializes ncurses */
void nc_init();

/* Draws game rectangle */
WINDOW *nc_init_space();

/* Draws score rectangle */
WINDOW *nc_init_scoreboard();

/* Draws user commands */
WINDOW *nc_init_astronaut(MOVEMENT_ORIENTATION player_orientation,
                          int player_id, int starting_row);

/* Draws the elements necessary for a given game when initializing */
void nc_draw_init_game(WINDOW *game_window, WINDOW *score_window, game_t game);

/******************** Updating screen ********************/

/* Helper function to sort players based on score */
int __compare_players(const void *a, const void *b);

/* Resets and updates the scoreboard */
void nc_update_scoreboard(WINDOW *win, player_t *players, int aliens_alive);

/* Adds a player to the screen */
void nc_add_player(WINDOW *win, player_t player);

/* Move a player on the screen */
void nc_move_player(WINDOW *win, player_t player, position_t old_pos);

/* Draws the zap line on the screen */
void nc_draw_zap(WINDOW *win, game_t *game, player_t *player_zap);

/* Adds a alien to the screen */
void nc_add_alien(WINDOW *game_window, position_t *position, bool regenerated);

/******************** Cleaning screen ********************/

/* Cleans a position from the screen */
void nc_clean_position(WINDOW *win, position_t position);

/* Cleans a zap from the screen */
void nc_clean_zap(WINDOW *win, game_t *game, player_t *player_zap);

/* Stops and cleanup ncurses */
void nc_cleanup();

#endif // NCURSES_WRAPPER_H