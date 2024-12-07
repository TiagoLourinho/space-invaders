#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "comms.h"
#include "game_def.h"
#include "utils.h"

int validate_action_request(action_request_t request, game_t game, int *tokens);

int validate_disconnect_request(disconnect_request_t request, game_t game,
                                int *tokens);

#endif // VALIDATORS_H