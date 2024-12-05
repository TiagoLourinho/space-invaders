#include "ncurses_wrapper.h"

/* Initializes ncurses */
void nc_init() {
  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
}

/* Draws game rectangle */
WINDOW *nc_draw_space() {

  /*
    Creates a window and draws a border
    Adding +2 on each dimension for the border
  */
  WINDOW *win = newwin(SPACE_SIZE + 2, SPACE_SIZE + 2, 0, 0);
  assert(win != NULL);

  box(win, 0, 0);
  wrefresh(win);

  return win;
}

/* Draws the starting aliens on screen */
void nc_draw_starting_aliens(WINDOW *game_window, game_t game) {

  for (int i = 0; i < N_ALIENS; i++) {
    position_t *position = &game.aliens[i].position;

    wmove(game_window, POS_TO_WIN(position->row), POS_TO_WIN(position->col));
    waddch(game_window, '*' | A_BOLD);
  }

  wrefresh(game_window);
}

/* Stops and cleanup ncurses */
void nc_cleanup() { endwin(); }
