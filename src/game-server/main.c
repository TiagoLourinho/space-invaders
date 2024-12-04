#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "server_utils.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <zmq.h>

int main() {
  game_t game;
  WINDOW *game_window;

  srand((unsigned int)time(NULL));

  game_window = nc_init();

  init_game(&game);

  nc_draw_starting_aliens(game_window, game);

  nc_cleanup();
}