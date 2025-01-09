/* Contains utility wrappers around the zeromq library */

#include "zeromq_wrapper.h"

/******************** Socket creation and initialization ********************/

/* Initializes zmq and gets context */
void *zmq_get_context() {
  void *context = zmq_ctx_new();
  assert(context != NULL);
  return context;
}

/* Creates a zmq socket with the given type */
void *zmq_create_socket(void *context, int type) {
  void *socket = zmq_socket(context, type);
  assert(socket != NULL);
  return socket;
}

/* Bind socket */
void zmq_bind_socket(void *socket, char *address) {
  int rc = zmq_bind(socket, address);
  if (rc != 0) {
    printf("Couln't bind socket to %s. Another zombie process caused by an "
           "abrupt exit might still be running. Check for 'game-server' "
           "processes with top/htop.\n",
           address);
    exit(-1);
  };
}

/* Connect socket */
void zmq_connect_socket(void *socket, char *address) {
  int rc = zmq_connect(socket, address);
  assert(rc == 0);
}

/* Subscribe to publisher */
void zmq_subscribe(void *socket, PUBSUB_TOPICS topic) {
  int rc = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, (void *)&topic,
                          sizeof(PUBSUB_TOPICS));
  assert(rc == 0);
}

/******************** Sending and receiving messages ********************/

/*
Receive messages (first the type then the actual message)

Dynamically allocates memory for the message received. Don't forget to free.

If topic==NOTOPIC then no topic is expected
*/
void *zmq_receive_msg(void *socket, MESSAGE_TYPE *msg_type,
                      PUBSUB_TOPICS topic) {
  int n;
  void *msg;
  size_t followup_msg_size;
  PUBSUB_TOPICS temp;

  /* Receive the topic and discard it as it isn't needed */
  if (topic != NO_TOPIC) {
    n = zmq_recv(socket, &temp, sizeof(PUBSUB_TOPICS), 0);
    assert(n != -1);
  }

  /* Receive message type/header */
  n = zmq_recv(socket, msg_type, sizeof(MESSAGE_TYPE), 0);
  assert(n != -1);

  followup_msg_size = get_msg_size(*msg_type);

  /* Receive actual message */
  if (followup_msg_size > 0) {
    msg = malloc(followup_msg_size);
    assert(msg != NULL);

    n = zmq_recv(socket, msg, followup_msg_size, 0);
    assert(n != -1);

    return msg;
  } else
    return NULL;
}

/* Send messages, first the type then the actual message.

  If msg_size==-1, then it uses the get_msg_size function to get the size
  If topic==NOTOPIC then no topic is sent at the beggining
*/
void zmq_send_msg(void *socket, MESSAGE_TYPE msg_type, void *msg, int msg_size,
                  PUBSUB_TOPICS topic) {
  int n;
  size_t followup_msg_size =
      (msg_size != -1) ? (size_t)msg_size : get_msg_size(msg_type);

  /* Send topic if needed */
  if (topic != NO_TOPIC) {
    n = zmq_send(socket, &topic, sizeof(PUBSUB_TOPICS), ZMQ_SNDMORE);
    assert(n != -1);
  }

  /* Send message type/header */
  n = zmq_send(socket, &msg_type, sizeof(MESSAGE_TYPE),
               followup_msg_size > 0 ? ZMQ_SNDMORE : 0);
  assert(n != -1);

  /* Send actual message */
  if (followup_msg_size > 0) {
    n = zmq_send(socket, msg, followup_msg_size, 0);
    assert(n != -1);
  }
}

/* Broadcasts the scores updates messages using protobuf protocol */
void zmq_broadcast_scores_updates(void *pub_socket, game_t *game) {
  ScoresMessage scores_message = SCORES_MESSAGE__INIT;
  int scores[MAX_PLAYERS];
  size_t packed_size;
  uint8_t *buffer;

  /* Build scores array (-1 for not connected players) */
  for (int i = 0; i < MAX_PLAYERS; i++) {
    scores[i] = game->players[i].connected ? game->players[i].score : -1;
  }

  /* Define protobuf message */
  scores_message.n_scores = MAX_PLAYERS;
  scores_message.scores = scores;

  /* Pack message */
  packed_size = scores_message__get_packed_size(&scores_message);
  buffer = (uint8_t *)malloc(packed_size);
  assert(buffer != NULL);
  scores_message__pack(&scores_message, buffer);

  /* Send and free */
  zmq_send_msg(pub_socket, SCORES_UPDATE, buffer, packed_size,
               SCORES_UPDATES_TOPIC);
  free(buffer);
};

/******************** Cleanup ********************/

/* Cleanup zmq */
void zmq_cleanup(void *context, void *socket1, void *socket2) {
  int n;

  if (socket1 != NULL) {
    n = zmq_close(socket1);
    assert(n == 0);
  }

  if (socket2 != NULL) {
    n = zmq_close(socket2);
    assert(n == 0);
  }

  n = zmq_ctx_destroy(context);
  assert(n == 0);
}

/******************** Utilities ********************/

/* Returns the size of the followup message given the type */
size_t get_msg_size(MESSAGE_TYPE type) {

  /* Scores update uses protobuf and as such the message size is variable and
   * should be sent manually in the other function */
  assert(type != SCORES_UPDATE);

  switch (type) {
  case DISPLAY_CONNECT_REQUEST:
    /* Connect request doens't have a followup message with more info */
    return 0;
  case DISPLAY_CONNECT_RESPONSE:
    return sizeof(display_connect_response_t);
  case ASTRONAUT_CONNECT_REQUEST:
    /* Connect request doens't have a followup message with more info */
    return 0;
  case ASTROUNAUT_CONNECT_RESPONSE:
    return sizeof(astronaut_connect_response_t);
  case ACTION_REQUEST:
    return sizeof(action_request_t);
  case ACTION_RESPONSE:
    return sizeof(action_response_t);
  case DISCONNECT_REQUEST:
    return sizeof(disconnect_request_t);
  case DISCONNECT_RESPONSE:
    return sizeof(status_code_and_score_response_t);
  case GAME_ENDED:
    /* Game ended doesn't have any follow up message */
    return 0;
  case ALIENS_UPDATE:
    return sizeof(aliens_update_t);

  default:
    exit(-1);
  }
}
