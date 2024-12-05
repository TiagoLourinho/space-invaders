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

/* Draws score rectangle */
WINDOW *nc_init_scoreboard() {

  WINDOW *win = newwin(MAX_PLAYERS + 4, 16, 0, SPACE_SIZE + 5);
  assert(win != NULL);

  box(win, 0, 0);

  wmove(win, 1, 1);
  wprintw(win, "  SCOREBOARD  ");
  wmove(win, 2, 1);
  wprintw(win, "--------------");

  wrefresh(win);

  return win;
}

/* Helper function to sort players based on score */
int __compare_players(const void *a, const void *b) {
  player_t *playerA = (player_t *)a;
  player_t *playerB = (player_t *)b;

  int score_A = playerA->score;
  int score_B = playerB->score;

  /* Scores are only reset when a player connects, so this is to make non
   * connected players go to the end of the scoreboard so it gets cleaned */
  if (!playerA->connected)
    score_A = -1;
  if (!playerB->connected)
    score_B = -1;

  return score_B - score_A;
}

/* Resets and updates the scoreboard */
void nc_update_scoreboard(WINDOW *win, player_t *players) {

  player_t copy_players[MAX_PLAYERS];

  // Create copy to be sorted
  for (int i = 0; i < MAX_PLAYERS; i++) {
    /* Only these attributes will be important */
    copy_players[i].id = players[i].id;
    copy_players[i].connected = players[i].connected;
    copy_players[i].score = players[i].score;
  }

  // Sort scores
  qsort(&copy_players, MAX_PLAYERS, sizeof(player_t), __compare_players);

  // Print scoreboard
  for (int i = 0; i < MAX_PLAYERS; i++) {
    wmove(win, 3 + i, 1);
    wprintw(win, "              ");

    if (copy_players[i].connected) {
      wmove(win, 3 + i, 1);
      wprintw(win, "Player %c - %3d", id_to_symbol(copy_players[i].id),
              copy_players[i].score);
    }
  }
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

/* Updates the screen */
void nc_update_screen(WINDOW *win) { wrefresh(win); }

/* Adds a player to the screen */
void nc_add_player(WINDOW *win, player_t player) {
  wmove(win, POS_TO_WIN(player.position.row), POS_TO_WIN(player.position.col));
  waddch(win, id_to_symbol(player.id) | A_BOLD);
}

/* Move a player on the screen */
void nc_move_player(WINDOW *win, player_t player, position_t old_pos) {
  wmove(win, POS_TO_WIN(old_pos.row), POS_TO_WIN(old_pos.col));
  waddch(win, ' ' | A_BOLD);

  wmove(win, POS_TO_WIN(player.position.row), POS_TO_WIN(player.position.col));
  waddch(win, id_to_symbol(player.id) | A_BOLD);
}

/* Cleans a position from the screen */
void nc_clean_position(WINDOW *win, position_t position) {
  wmove(win, POS_TO_WIN(position.row), POS_TO_WIN(position.col));
  waddch(win, ' ' | A_BOLD);
}
