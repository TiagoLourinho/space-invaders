/* Defines requests and responses format */

#ifndef COMMS_H
#define COMMS_H

#include "game_def.h"

#define PROTOCOL "tcp"
#define SERVER_IP "127.0.0.1"

/* REQREP STUFF */
#define PORT_REQREP "62762"
#define SERVER_ZMQ_REQREP_ADDRESS PROTOCOL "://" SERVER_IP ":" PORT_REQREP
#define SERVER_ZMQ_REQREP_BIND_ADDRESS PROTOCOL "://*:" PORT_REQREP

/* PUBSUB STUFF */
#define PORT_PUBSUB "62763"
#define SERVER_ZMQ_PUBSUB_ADDRESS PROTOCOL "://" SERVER_IP ":" PORT_PUBSUB
#define SERVER_ZMQ_PUBSUB_BIND_ADDRESS PROTOCOL "://*:" PORT_PUBSUB

/*
  Every message has 2 parts:
    - the type/header
    - the message contents
*/
typedef enum {
  /* Only sends the header as no extra info is needed */
  DISPLAY_CONNECT_REQUEST,
  DISPLAY_CONNECT_RESPONSE,
  /* Only sends the header as no extra info is needed */
  ASTRONAUT_CONNECT_REQUEST,
  ASTROUNAUT_CONNECT_RESPONSE,
  ACTION_REQUEST,
  /* Only status code */
  ACTION_RESPONSE,
  DISCONNECT_REQUEST,
  /* Only status code */
  DISCONNECT_RESPONSE,
  ALIENS_UPDATE_REQUEST,
  /* Only status code */
  ALIENS_UPDATE_RESPONSE,
} MESSAGE_TYPE;

typedef struct {
  /* 200 if Ok, 400 otherwise */
  int status_code;
  /* The current state of the game when it connected (without tokens) */
  game_t game;
} display_connect_response_t;

typedef struct {
  /* 200 if Ok, 400 otherwise */
  int status_code;
  /* The id assigned to the player (corresponds to the position on the players
   * array) */
  int id;
  MOVEMENT_ORIENTATION orientation;
  /* The token assigned to the player for authentication */
  int token;
} astronaut_connect_response_t;

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