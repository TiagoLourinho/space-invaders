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

  player->orientation = id % 2 ? HORIZONTAL : VERTICAL;

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

    player->connected = -1;
    player->id = i;
    player->last_shot = -1;
    player->last_stunned = -1;
    place_player(player);
    player->score = -1;
    player->token = -1;
  }

  /* Init aliens */
  game->aliens_alive = (SPACE_SIZE * SPACE_SIZE) / 3;

  for (int i = 0; i < game->aliens_alive; i++) {
    alien_t *alien = &game->aliens[i];

    alien->alive = 1;
    place_alien(alien);
  }
}