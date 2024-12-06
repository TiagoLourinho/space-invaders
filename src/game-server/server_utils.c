/* Utils used only by the server */

#include "server_utils.h"

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

/* Places the alien on the board */
void place_alien(alien_t *alien) {
  position_t *position = &alien->position;

  /* Alien can only be on the "inner" space square */
  position->row = rand() % (SPACE_SIZE - 4) + 2;
  position->col = rand() % (SPACE_SIZE - 4) + 2;
}

/* Inits all the players and aliens on the board */
void init_game(game_t *game) {

  /* Init players */
  for (int i = 0; i < MAX_PLAYERS; i++) {
    player_t *player = &game->players[i];

    player->connected = false;
    player->id = i;
    player->last_shot = 0;
    player->last_stunned = 0;
    place_player(player);
    player->score = -1;
    player->token = -1;
  }

  /* Init aliens */
  game->aliens_alive = N_ALIENS;

  for (int i = 0; i < game->aliens_alive; i++) {
    alien_t *alien = &game->aliens[i];

    alien->alive = true;
    place_alien(alien);
  }
}

/* Finds an available position for a player and initializes it */
int find_position_and_init_player(game_t *game) {

  for (int i = 0; i < MAX_PLAYERS; i++) {
    player_t *player = &game->players[i];

    if (!player->connected) {
      player->connected = true;
      player->last_shot = 0;
      player->last_stunned = 0;
      place_player(player);
      player->score = 0;
      player->token = rand();
      return i;
    }
  }

  return -1;
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

/* Updates the screen and state when a player zaps */
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

    if ((player->orientation == HORIZONTAL &&
         player->position.col == other_player->position.col) ||
        (player->orientation == VERTICAL &&
         player->position.row == other_player->position.row))
      other_player->last_stunned = current_ts;
  }
}

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

    zmq_connect_socket(req_socket, SERVER_ZMQ_ADDRESS);

    /* Initial copy of the aliens to the request struct */
    for (int i = 0; i < N_ALIENS; i++) {
      aliens_update_request.aliens[i].alive = aliens[i].alive;
      aliens_update_request.aliens[i].position.col = aliens[i].position.col;
      aliens_update_request.aliens[i].position.row = aliens[i].position.row;
    }

    while (1) {
      sleep(ALIEN_UPDATE);

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