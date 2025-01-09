/* Defines validators for each request received by the client */

#include "validators.h"

/* Validates the connect request and returns the status code  */
int validate_connect_request(game_t game) {

  /* Check if there is a free position to play */
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (!game.players[i].connected)
      return 200;
  }

  return 400;
}

/* Validates the action request and returns the status code */
int validate_action_request(action_request_t request, game_t game, int *tokens,
                            action_response_t *action_response) {

  int id = request.id;
  int request_token = request.token;
  player_t player;
  uint64_t current_ts = get_timestamp_ms();

  /* Initially, there is no delay */
  action_response->next_allowed_action_timestamp = current_ts;
  action_response->next_allowed_zap_timestamp = current_ts;

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
    if (!(current_ts - player.last_stunned > STUNNED_DELAY)) {
      action_response->next_allowed_action_timestamp =
          player.last_stunned + STUNNED_DELAY;
      return 400;
    }

    /* Player shot */
    if (!(current_ts - player.last_shot > ZAP_DELAY)) {
      action_response->next_allowed_zap_timestamp =
          player.last_shot + ZAP_DELAY;
      return 400;
    }

    break;
  case MOVE:

    /* Player is stunned */
    if (!(current_ts - player.last_stunned > STUNNED_DELAY)) {
      action_response->next_allowed_action_timestamp =
          player.last_stunned + STUNNED_DELAY;
      return 400;
    }

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