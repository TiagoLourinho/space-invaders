#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "comms.h"
#include "game_def.h"
#include "utils.h"

/* Validates the connect request and returns the status code  */
int validate_connect_request(game_t game);

/* Validates the action request and returns the status code */
int validate_action_request(action_request_t request, game_t game, int *tokens,
                            action_response_t *action_response);

/* Validates  the request and returns status code */
int validate_disconnect_request(disconnect_request_t request, game_t game,
                                int *tokens);
#endif // VALIDATORS_H