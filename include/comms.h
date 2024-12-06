/* Defines requests and responses format */

#ifndef COMMS_H
#define COMMS_H

#include "game_def.h"

#define SERVER_ADDRESS "tcp://127.0.0.1:5555"

/*
  Every message has 2 parts:
    - the type/header
    - the message contents
*/
typedef enum {
  CONNECT_REQUEST, /* Only sends the header as no extra info is needed */
  CONNECT_RESPONSE,
  ACTION_REQUEST,
  ACTION_RESPONSE, /* Only status code */
  DISCONNECT_REQUEST,
  DISCONNECT_RESPONSE, /* Only status code */
  ALIENS_UPDATE_REQUEST,
  ALIENS_UPDATE_RESPONSE, /* Only status code */
} MESSAGE_TYPE;

typedef struct {
  /* 200 if Ok, 400 otherwise */
  int status_code;
  /* The id assigned to the player (corresponds to the position on the players
   * array) */
  int id;
  MOVEMENT_ORIENTATION orientation;
  /* The token assigned to the player for authentication */
  int token;
} connect_response_t;

typedef struct {
  /* The id assigned to the player (corresponds to the position on the players
   * array) */
  int id;
  ACTION_TYPE action_type;
  /* Only used if action_type==MOVEMENT */
  MOVEMENT_DIRECTION movement_direction;
  /* The token assigned to the player for authentication */
  int token;
} action_request_t;

typedef struct {
  /* The id assigned to the player */
  int id;
  /* The token assigned to the player for authentication */
  int token;
} disconnect_request_t;

typedef struct {
  alien_t aliens[N_ALIENS];
} aliens_update_request_t;

typedef struct {
  /* 200 if Ok, 400 otherwise */
  int status_code;
} only_status_code_response_t;

#endif // COMMS_H