#ifndef ZEROMQ_WRAPPER_H
#define ZEROMQ_WRAPPER_H

#include "comms.h"
#include "scores.pb-c.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

/******************** Socket creation and initialization ********************/

/* Initializes zmq and gets context */
void *zmq_get_context();

/* Creates a zmq socket with the given type */
void *zmq_create_socket(void *context, int type);

/* Bind socket */
void zmq_bind_socket(void *socket, char *address);

/* Connect socket */
void zmq_connect_socket(void *socket, char *address);

/* Subscribe to publisher */
void zmq_subscribe(void *socket, PUBSUB_TOPICS topic);

/******************** Sending and receiving messages ********************/

/*
Receive messages (first the type then the actual message)

Dynamically allocates memory for the message received. Don't forget to free.

If topic==NOTOPIC then no topic is expected
*/
void *zmq_receive_msg(void *socket, MESSAGE_TYPE *msg_type,
                      PUBSUB_TOPICS topic);

/* Send messages, first the type then the actual message.

  If msg_size==-1, then it uses the get_msg_size function to get the size
  If topic==NOTOPIC then no topic is sent at the beggining
*/
void zmq_send_msg(void *socket, MESSAGE_TYPE msg_type, void *msg, int msg_size,
                  PUBSUB_TOPICS topic);

/* Broadcasts the scores updates messages using protobuf protocol */
void zmq_broadcast_scores_updates(void *pub_socket, game_t *game);

/******************** Cleanup ********************/

/* Cleanup zmq */
void zmq_cleanup(void *context, void *socket1, void *socket2);

/******************** Utilities ********************/

/* Returns the size of the followup message given the type */
size_t get_msg_size(MESSAGE_TYPE type);

#endif // ZEROMQ_WRAPPER_H