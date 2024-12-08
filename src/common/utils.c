/* Defines general utilities */

#include "utils.h"

/******************** Client requests handling ********************/

/* Handles the state and screen updates when a player connects */
void handle_player_connect(
    WINDOW *game_window,
    astronaut_connect_response_t *astronaut_connect_response, int *tokens,
    game_t *game) {

  int idx = find_position_and_init_player(game, tokens);
  assert(idx != -1);

  /* Display application won't send a response */
  if (astronaut_connect_response != NULL) {
    astronaut_connect_response->id = idx;
    astronaut_connect_response->token = tokens[idx];
    astronaut_connect_response->orientation = game->players[idx].orientation;
  }

  nc_add_player(game_window, game->players[idx]);
}

/* Handles the state and screen updates when a player makes an action */
void handle_player_action(action_request_t *action_request,
                          player_t *current_player, WINDOW *game_window,
                          game_t *game) {
  position_t old_position;

  if (action_request->action_type == MOVE) {
    /* Store old pos */
    old_position.col = current_player->position.col;
    old_position.row = current_player->position.row;

    update_position(&current_player->position,
                    action_request->movement_direction);

    nc_move_player(game_window, *current_player, old_position);
  } else if (action_request->action_type == ZAP) {
    player_zap(game_window, game, action_request->id);
    nc_draw_zap(game_window, game, current_player);
    usleep(ZAP_TIME_ON_SCREEN * 1000);
    nc_clean_zap(game_window, game, current_player);
  }
}

/* Handles the state and screen updates when a player disconnects */
void handle_player_disconnect(WINDOW *game_window, player_t *current_player) {
  nc_clean_position(game_window, current_player->position);
  current_player->connected = false;
}

/* Handles the state and screen updates when the aliens positions are updated */
void handle_aliens_updates(WINDOW *game_window,
                           aliens_update_request_t *alien_update_request,
                           game_t *game) {
  alien_t *alien;

  /*
    Two loops to clean the old positions of the aliens and then put the
    new ones (can't be done in just 1 iteration because there would be
    problems with overlaps between new and old positions)
  */
  for (int i = 0; i < N_ALIENS; i++) {
    alien = &game->aliens[i];
    if (alien->alive)
      nc_clean_position(game_window, alien->position);
  }
  for (int i = 0; i < N_ALIENS; i++) {
    alien = &game->aliens[i];
    if (alien->alive) {
      alien->position.col = alien_update_request->aliens[i].position.col;
      alien->position.row = alien_update_request->aliens[i].position.row;

      nc_add_alien(game_window, &alien->position);
    }
  }
}

/******************** Aliens management ********************/

/* Spawns the process responsible for updating the aliens */
void spawn_alien_update_fork(alien_t *aliens) {

  int n = fork();
  assert(n != -1);

  /* Only the child process goes in */
  if (n == 0) {
    void *zmq_context = zmq_get_context();
    void *req_socket = zmq_create_socket(zmq_context, ZMQ_REQ);
    MESSAGE_TYPE msg_type;
    int status_code;
    aliens_update_request_t aliens_update_request;
    only_status_code_response_t *status_code_response;

    zmq_connect_socket(req_socket, SERVER_ZMQ_REQREP_ADDRESS);

    /* Initial copy of the aliens to the request struct */
    for (int i = 0; i < N_ALIENS; i++) {
      aliens_update_request.aliens[i].alive = aliens[i].alive;
      aliens_update_request.aliens[i].position.col = aliens[i].position.col;
      aliens_update_request.aliens[i].position.row = aliens[i].position.row;
    }

    while (1) {
      usleep(ALIEN_UPDATE * 1000);

      /* Generate new positions for aliens */
      for (int i = 0; i < N_ALIENS; i++) {
        update_position(&aliens_update_request.aliens[i].position,
                        (MOVEMENT_DIRECTION)(rand() % 4));
      }

      zmq_send_msg(req_socket, ALIENS_UPDATE_REQUEST, &aliens_update_request);

      status_code_response =
          (only_status_code_response_t *)zmq_receive_msg(req_socket, &msg_type);

      status_code = status_code_response->status_code;
      free(status_code_response);

      if (status_code == 400) {
        /* The main process will send a 400 code when this child process should
         * stop generating positions */
        exit(0);
      }
    }
  }
}

/* Places the alien on the board */
void place_alien(alien_t *alien) {
  position_t *position = &alien->position;

  /* Alien can only be on the "inner" space square */
  position->row = rand() % (SPACE_SIZE - 4) + 2;
  position->col = rand() % (SPACE_SIZE - 4) + 2;
}

/******************** Player management ********************/

/* Places the player on the board */
void place_player(player_t *player) {
  position_t *position = &player->position;
  int id = player->id;

  // Init board like:
  //        0
  //        4
  // 3 7         5 1
  //        6
  //        2

  player->orientation = id % 2 == 0 ? HORIZONTAL : VERTICAL;

  switch (id) {
  case 0:
    position->col = SPACE_SIZE / 2;
    position->row = 0;
    break;
  case 1:
    position->col = SPACE_SIZE - 1;
    position->row = SPACE_SIZE / 2;
    break;
  case 2:
    position->col = SPACE_SIZE / 2;
    position->row = SPACE_SIZE - 1;
    break;
  case 3:
    position->col = 0;
    position->row = SPACE_SIZE / 2;
    break;
  case 4:
    position->col = SPACE_SIZE / 2;
    position->row = 1;
    break;
  case 5:
    position->col = SPACE_SIZE - 2;
    position->row = SPACE_SIZE / 2;
    break;
  case 6:
    position->col = SPACE_SIZE / 2;
    position->row = SPACE_SIZE - 2;
    break;
  case 7:
    position->col = 1;
    position->row = SPACE_SIZE / 2;
    break;
  default:
    exit(-1);
  }
}

/* Finds an available position for a player and initializes it */
int find_position_and_init_player(game_t *game, int *tokens) {

  for (int i = 0; i < MAX_PLAYERS; i++) {
    player_t *player = &game->players[i];

    if (!player->connected) {
      player->connected = true;
      player->last_shot = 0;
      player->last_stunned = 0;
      place_player(player);
      player->score = 0;

      /* Displays will use this function but don't manage authentication */
      if (tokens != NULL)
        tokens[i] = rand();

      return i;
    }
  }

  return -1;
}

/* Updates state when a player zaps and kills the aliens */
void player_zap(WINDOW *win, game_t *game, int player_id) {
  int aliens_killed = 0;
  alien_t *alien;
  player_t *player = &game->players[player_id];
  player_t *other_player;
  uint64_t current_ts = get_timestamp_ms();

  /* Check aliens that were killed */
  for (int i = 0; i < N_ALIENS; i++) {
    alien = &game->aliens[i];

    /* Alien dies if it is alive and aligned with the player zap */
    if (alien->alive && ((player->orientation == VERTICAL &&
                          alien->position.row == player->position.row) ||
                         (player->orientation == HORIZONTAL &&
                          alien->position.col == player->position.col))) {
      aliens_killed++;
      game->aliens_alive--;
      alien->alive = false;
      nc_clean_position(win, alien->position);
    }
  }

  player->last_shot = current_ts;
  player->score += aliens_killed;

  /* Check if it stunned other players */
  for (int i = 0; i < MAX_PLAYERS; i++) {
    /* Skip current player */
    if (i == player_id)
      continue;

    other_player = &game->players[i];

    /* Player is stunned if aligned with the player that shot */
    if ((player->orientation == HORIZONTAL &&
         player->position.col == other_player->position.col) ||
        (player->orientation == VERTICAL &&
         player->position.row == other_player->position.row))
      other_player->last_stunned = current_ts;
  }
}

/******************** Miscellaneous ********************/

/* Inits all the players and aliens on the board */
void init_game(game_t *game, int *tokens) {

  /* Init players */
  for (int i = 0; i < MAX_PLAYERS; i++) {
    player_t *player = &game->players[i];

    player->connected = false;
    player->id = i;
    player->last_shot = 0;
    player->last_stunned = 0;
    place_player(player);
    player->score = -1;
    tokens[i] = -1;
  }

  /* Init aliens */
  game->aliens_alive = N_ALIENS;

  for (int i = 0; i < game->aliens_alive; i++) {
    alien_t *alien = &game->aliens[i];

    alien->alive = true;
    place_alien(alien);
  }
}

/* Update the position of a player or alien */
void update_position(position_t *position, MOVEMENT_DIRECTION direction) {

  switch (direction) {
  case UP:
    position->row--;

    /* Revert if out of bounds */
    if (position->row < 2)
      position->row++;
    break;
  case DOWN:
    position->row++;

    /* Revert if out of bounds */
    if (position->row >= SPACE_SIZE - 2)
      position->row--;
    break;
  case RIGHT:
    position->col++;

    /* Revert if out of bounds */
    if (position->col >= SPACE_SIZE - 2)
      position->col--;
    break;
  case LEFT:
    position->col--;

    /* Revert if out of bounds */
    if (position->col < 2)
      position->col++;
    break;

  default:
    break;
  }
}

/* Copies the game state to the connect reply */
void copy_game_state_for_display(display_connect_response_t *response,
                                 game_t *game) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    response->game.players[i].connected = game->players[i].connected;
    response->game.players[i].id = game->players[i].id;
    response->game.players[i].last_shot = game->players[i].last_shot;
    response->game.players[i].last_stunned = game->players[i].last_stunned;
    response->game.players[i].orientation = game->players[i].orientation;
    response->game.players[i].position.row = game->players[i].position.row;
    response->game.players[i].position.col = game->players[i].position.col;
    response->game.players[i].score = game->players[i].score;
  }

  response->game.aliens_alive = game->aliens_alive;

  for (int i = 0; i < N_ALIENS; i++) {
    response->game.aliens[i].alive = game->aliens[i].alive;
    response->game.aliens[i].position.col = game->aliens[i].position.col;
    response->game.aliens[i].position.row = game->aliens[i].position.row;
  }
}

/* Finds and prints the winning player */
void print_winning_player(game_t *game) {
  int idx = -1;
  player_t *current_player;

  /* Find winning player */
  for (int i = 0; i < MAX_PLAYERS; i++) {
    current_player = &game->players[i];

    if (current_player->connected &&
        (idx == -1 || current_player->score >= game->players[idx].score))
      idx = i;
  }

  erase(); /* Clean entire ncurses screen */

  if (idx != -1)
    printw("Player %c won with %d points!\n", id_to_symbol(idx),
           game->players[idx].score);

  refresh(); /* Refresh entire ncurses screen */
  sleep(5);
}

/* Converts an ID to a symbol */
char id_to_symbol(int id) { return (char)('A' + id); }

/* Returns the current timestamp in ms since epoch */
uint64_t get_timestamp_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}
