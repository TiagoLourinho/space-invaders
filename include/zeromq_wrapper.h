#ifndef ZEROMQ_WRAPPER_H
#define ZEROMQ_WRAPPER_H

#include "comms.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <zmq.h>

size_t get_msg_size(MESSAGE_TYPE type);

void *zmq_get_context();

void *zmq_create_socket(void *context, int type);

void zmq_bind_socket(void *socket, char *address);

void zmq_connect_socket(void *socket, char *address);

void *zmq_receive_msg(void *socket, MESSAGE_TYPE *msg_type);

void zmq_send_msg(void *socket, MESSAGE_TYPE msg_type, void *msg);

void zmq_cleanup(void *context, void *socket1, void *socket2);

#endif // ZEROMQ_WRAPPER_H