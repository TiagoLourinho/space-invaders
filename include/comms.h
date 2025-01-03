/* Defines requests and responses format */

#ifndef COMMS_H
#define COMMS_H

#include "game_def.h"
#include <ncurses.h>
#include <pthread.h>

#define PROTOCOL "tcp"
#define SERVER_IP "127.0.0.1"

/* REQREP related */
#define PORT_REQREP "62762"
#define SERVER_ZMQ_REQREP_ADDRESS PROTOCOL "://" SERVER_IP ":" PORT_REQREP
#define SERVER_ZMQ_REQREP_BIND_ADDRESS PROTOCOL "://*:" PORT_REQREP

/* PUBSUB related */
#define PORT_PUBSUB "62763"
#define SERVER_ZMQ_PUBSUB_ADDRESS PROTOCOL "://" SERVER_IP ":" PORT_PUBSUB
#define SERVER_ZMQ_PUBSUB_BIND_ADDRESS PROTOCOL "://*:" PORT_PUBSUB

/*
  Every message (request or response) has 2 parts:
    - the type/header (defined by MESSAGE_TYPE)
    - the message contents (defined by the requests/responses structs)

  The "connection" between message types and requests/responses structs used can
  be seen in the function `get_msg_size` in the `zeromq_wrappers.c`

  Interactions with the server:
    - Client sends a request to the server with the 2 parts mentioned above
      (except the connect, as it only contains the header) and the server
      responds in the same way -> Uses only REQREP.

    - Display servers sends an initial connect message with no followup part to
      get the current game status (using REQREP) and after that starts listening
      to all the messages broadcasted by the server using PUBSUB. Here, the
      server simple publishes all messages received from the clients,
      invalidating the tokens so that sensitive information isn't broadcasted

    - To generate the aliens updates the server creates a fork and the new
      process uses the ALIENS_UPDATE_REQUEST to notify the main server -> Uses
      only REQREP.

*/
typedef enum {
  /*
    REQREP messages
    (the requests are still broadcasted after being validated to update the
    displays)
  */
  DISPLAY_CONNECT_REQUEST,     /* No followup message needed */
  DISPLAY_CONNECT_RESPONSE,    /* Follows display_connect_response_t */
  ASTRONAUT_CONNECT_REQUEST,   /* No followup message needed */
  ASTROUNAUT_CONNECT_RESPONSE, /* Follows astronaut_connect_response_t */
  ACTION_REQUEST,              /* Follows action_request_t */
  ACTION_RESPONSE,             /* Follows status_code_and_score_response_t */
  DISCONNECT_REQUEST,          /* Follows disconnect_request_t */
  DISCONNECT_RESPONSE,         /* Follows status_code_and_score_response_t */
  /* PUBSUB only messages */
  GAME_ENDED,    /* No followup message needed */
  ALIENS_UPDATE, /* Follows aliens_update_t */
  SCORES_UPDATE  /* Follows ScoresMessage (defined in src/proto/scores.proto) */
} MESSAGE_TYPE;

typedef enum {
  /* Don't send a topic */
  NO_TOPIC,
  /* Contains all game related updates such as connect, disconnect, zap, etc */
  GAME_UPDATES_TOPIC,
  /* Contains only the scores updates using protobuf protocol */
  SCORES_UPDATES_TOPIC
} PUBSUB_TOPICS;

/******************** Requests structs ********************/

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

/******************** Response structs ********************/

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
  /* 200 if Ok, 400 otherwise */
  int status_code;
} only_status_code_response_t;

typedef struct {
  /* 200 if Ok, 400 otherwise */
  int status_code;
  int player_score;
} status_code_and_score_response_t;

/******************** Other broadcasted structs ********************/

typedef struct {
  alien_t aliens[N_ALIENS];
} aliens_update_t;

/******************** Thread args structs ********************/

typedef struct {
  game_t *game;
  WINDOW *game_window;
  WINDOW *score_window;
  void *pub_socket;
  pthread_mutex_t *lock;
} aliens_update_thread_args_t;

#endif // COMMS_H