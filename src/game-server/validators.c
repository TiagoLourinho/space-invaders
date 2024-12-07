#include "validators.h"

/* Validate the action request and returns the status code */
int validate_action_request(action_request_t request, game_t game,
                            int *tokens) {

  int id = request.id;
  int request_token = request.token;
  player_t player;
  uint64_t current_ts = get_timestamp_ms();

  /* ID not valid */
  if (!(id >= 0 && id < MAX_PLAYERS))
    return 400;

  player = game.players[id];

  /* Player wasn't connected */
  if (!(player.connected))
    return 400;

  /* Token was invalid */
  if (!(tokens[id] == request_token))
    return 400;

  switch (request.action_type) {
  case ZAP:
    /* Player is stunned */
    if (!(current_ts - player.last_stunned > STUNNED_DELAY))
      return 400;

    /* Player shot */
    if (!(current_ts - player.last_shot > ZAP_DELAY))
      return 400;

    break;
  case MOVE:

    /* Player is stunned */
    if (!(current_ts - player.last_stunned > STUNNED_DELAY))
      return 400;

    /* Player shot and cant move while zapping */
    if (!(current_ts - player.last_shot > ZAP_TIME_ON_SCREEN))
      return 400;

    /* Check if movement direction aligns with orientation */
    if (player.orientation == HORIZONTAL) {
      if (!(request.movement_direction == LEFT ||
            request.movement_direction == RIGHT))
        return 400;
    } else if (player.orientation == VERTICAL) {
      if (!(request.movement_direction == UP ||
            request.movement_direction == DOWN))
        return 400;
    } else {
      return 400;
    }

    break;

  default:
    return 400;
  }

  return 200;
}

/* Validates  the request and returns status code */
int validate_disconnect_request(disconnect_request_t request, game_t game,
                                int *tokens) {

  int id = request.id;
  int request_token = request.token;
  player_t player;

  /* ID not valid */
  if (!(id >= 0 && id < MAX_PLAYERS))
    return 400;

  player = game.players[id];

  /* Player wasn't connected */
  if (!(player.connected))
    return 400;

  /* Token was invalid */
  if (!(tokens[id] == request_token))
    return 400;

  return 200;
}