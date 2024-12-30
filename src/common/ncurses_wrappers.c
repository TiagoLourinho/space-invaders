/* Contains utility wrappers around the ncurses library */

#include "ncurses_wrapper.h"

/******************** Initialization functions ********************/

/* Initializes ncurses */
void nc_init() {
  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0); // Hide the cursor

  /* Draw letters and lasers with color */
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
}

/* Draws game rectangle */
WINDOW *nc_init_space() {

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

  WINDOW *win = newwin(MAX_PLAYERS + 2 + 2 + 2, 16, 0, SPACE_SIZE + 4);
  assert(win != NULL);

  box(win, 0, 0);

  wmove(win, 1, 1);
  wprintw(win, "  SCOREBOARD  ");
  wmove(win, 2, 1);
  wprintw(win, "--------------");

  wmove(win, 11, 1);
  wprintw(win, "--------------");
  wmove(win, 12, 1);
  wprintw(win, "* ALIVE  - %d", N_ALIENS);

  wrefresh(win);

  return win;
}

/* Draws user commands */
WINDOW *nc_init_astronaut(MOVEMENT_ORIENTATION player_orientation,
                          int player_id, int starting_row) {

  WINDOW *win = newwin(11, 40, starting_row, 0);
  assert(win != NULL);

  box(win, 0, 0);

  /* Print game instructions */
  wmove(win, 1, 1);
  wprintw(win, "You are playing as player: %c", id_to_symbol(player_id));
  wmove(win, 3, 1);
  wprintw(win, "Controls:");
  wmove(win, 4, 1);
  if (player_orientation == VERTICAL) {
    wprintw(win, "\t UP ARROW\t-> Move up");
    wmove(win, 5, 1);
    wprintw(win, "\t DOWN ARROW\t-> Move down");

  } else {
    wprintw(win, "\t RIGHT ARROW\t-> Move right");
    wmove(win, 5, 1);
    wprintw(win, "\t LEFT ARROW\t-> Move left");
  }
  wmove(win, 6, 1);
  wprintw(win, "\t SPACEBAR\t-> Zap");
  wmove(win, 7, 1);
  wprintw(win, "\t q/Q\t\t-> Stop playing");

  wrefresh(win);

  return win;
}

/* Draws the elements necessary for a given game when initializing */
void nc_draw_init_game(WINDOW *game_window, WINDOW *score_window, game_t game) {

  /* Draw aliens */
  for (int i = 0; i < N_ALIENS; i++) {
    alien_t *alien = &game.aliens[i];

    if (alien->alive)
      nc_add_alien(game_window, &alien->position, false);
  }

  /* Draw players */
  for (int i = 0; i < MAX_PLAYERS; i++) {
    player_t *player = &game.players[i];

    if (player->connected)
      nc_add_player(game_window, *player);
  }

  nc_update_scoreboard(score_window, game.players, game.aliens_alive);

  wrefresh(game_window);
  wrefresh(score_window);
}

/******************** Updating screen ********************/

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
void nc_update_scoreboard(WINDOW *win, player_t *players, int aliens_alive) {

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

  /* Update alive aliens */
  wmove(win, 12, 1);
  wprintw(win, "* ALIVE  - %3d", aliens_alive);
}

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

/* Draws the zap line on the screen */
void nc_draw_zap(WINDOW *win, game_t *game, player_t *player_zap) {
  player_t *other_player;

  /* Draw laser in green (color pair 2) */
  wattron(win, COLOR_PAIR(2));
  for (int i = 0; i < SPACE_SIZE; i++) {

    if (player_zap->orientation == VERTICAL) {
      wmove(win, POS_TO_WIN(player_zap->position.row), POS_TO_WIN(i));
      waddch(win, '-');
    } else {
      wmove(win, POS_TO_WIN(i), POS_TO_WIN(player_zap->position.col));
      waddch(win, '|');
    }
  }
  wattroff(win, COLOR_PAIR(2));

  /* Add player that shot back to the screen */
  nc_add_player(win, *player_zap);

  /* Signal players that were stunned with red letters (color pair 1) */
  wattron(win, COLOR_PAIR(1));
  for (int i = 0; i < MAX_PLAYERS; i++) {
    other_player = &game->players[i];

    /* Player is stunned if aligned with the player that shot */
    if (other_player->connected && other_player->id != player_zap->id) {
      if ((player_zap->orientation == HORIZONTAL &&
           player_zap->position.col == other_player->position.col) ||
          (player_zap->orientation == VERTICAL &&
           player_zap->position.row == other_player->position.row)) {
        wmove(win, POS_TO_WIN(other_player->position.row),
              POS_TO_WIN(other_player->position.col));
        waddch(win, id_to_symbol(other_player->id) | A_BOLD);
      }
    }
  }
  wattroff(win, COLOR_PAIR(1));

  wrefresh(win);
};

/* Adds a alien to the screen */
void nc_add_alien(WINDOW *game_window, position_t *position, bool regenerated) {

  if (regenerated)
    wattron(game_window, COLOR_PAIR(3));

  wmove(game_window, POS_TO_WIN(position->row), POS_TO_WIN(position->col));
  waddch(game_window, '*' | A_BOLD);

  if (regenerated)
    wattroff(game_window, COLOR_PAIR(3));
}

/******************** Cleaning screen ********************/

/* Cleans a position from the screen */
void nc_clean_position(WINDOW *win, position_t position) {
  wmove(win, POS_TO_WIN(position.row), POS_TO_WIN(position.col));
  waddch(win, ' ' | A_BOLD);
}

/* Cleans a zap from the screen */
void nc_clean_zap(WINDOW *win, game_t *game, player_t *player_zap) {
  player_t *other_player;

  /* Clean entire row/col */
  for (int i = 0; i < SPACE_SIZE; i++) {
    if (player_zap->orientation == VERTICAL)
      wmove(win, POS_TO_WIN(player_zap->position.row), POS_TO_WIN(i));
    else
      wmove(win, POS_TO_WIN(i), POS_TO_WIN(player_zap->position.col));

    waddch(win, ' ');
  }

  /* Add back players */
  for (int i = 0; i < MAX_PLAYERS; i++) {
    other_player = &game->players[i];
    if (other_player->connected)
      nc_add_player(win, *other_player);
  }

  wrefresh(win);
};

/* Stops and cleanup ncurses */
void nc_cleanup() { endwin(); }
