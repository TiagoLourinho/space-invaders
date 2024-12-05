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

void nc_init();

WINDOW *nc_draw_space();

void nc_draw_starting_aliens(WINDOW *game_window, game_t game);

void nc_cleanup();

void nc_update_screen(WINDOW *win);

void nc_add_player(WINDOW *win, player_t player);

void nc_move_player(WINDOW *win, player_t player, position_t old_pos);

void nc_clean_position(WINDOW *win, position_t position);

#endif // NCURSES_WRAPPER_H