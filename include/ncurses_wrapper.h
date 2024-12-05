#ifndef NCURSES_WRAPPER_H
#define NCURSES_WRAPPER_H

#include "game_def.h"
#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

/* Converts a game position to a position on the window (considering the
 * border)*/
#define POS_TO_WIN(val) ((val) + 1)

void nc_init();

WINDOW *nc_draw_space();

void nc_draw_starting_aliens(WINDOW *game_window, game_t game);

void nc_cleanup();

#endif // NCURSES_WRAPPER_H