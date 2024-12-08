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
void zmq_subscribe(void *socket, char *topic) {
  int rc = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, topic, strlen(topic));
  assert(rc == 0);
}

/******************** Sending and receiving messages ********************/

/*
Receive messages (first the type then the actual message)

Dynamically allocates memory for the message received. Don't forget to free.
*/
void *zmq_receive_msg(void *socket, MESSAGE_TYPE *msg_type) {
  int n;
  void *msg;
  size_t followup_msg_size;

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

/* Send messages (first the type then the actual message)*/
void zmq_send_msg(void *socket, MESSAGE_TYPE msg_type, void *msg) {
  int n;
  size_t followup_msg_size = get_msg_size(msg_type);

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
    return sizeof(status_code_and_score_response_t);
  case DISCONNECT_REQUEST:
    return sizeof(disconnect_request_t);
  case DISCONNECT_RESPONSE:
    return sizeof(status_code_and_score_response_t);
  case ALIENS_UPDATE_REQUEST:
    return sizeof(aliens_update_request_t);
  case ALIENS_UPDATE_RESPONSE:
    return sizeof(only_status_code_response_t);
  case GAME_ENDED:
    /* Game ended doesn't have any follow up message */
    return 0;
  default:
    exit(-1);
  }
}
