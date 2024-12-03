#include "comms.h"
#include "game_def.h"
#include "server_utils.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <zmq.h>

int main() {
  game_t game;

  srand((unsigned int)time(NULL));

  init_game(&game);
}