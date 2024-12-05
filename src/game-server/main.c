#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "server_utils.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <zmq.h>

int main() {
  game_t game;
  /* Ncurses related */
  WINDOW *game_window;
  /* ZMQ and messages related */
  void *zmq_context = zmq_get_context();
  void *rep_socket = zmq_create_socket(zmq_context, ZMQ_REP);
  MESSAGE_TYPE msg_type;

  /* Structs and temp pointer to receive/send requests/responses */
  void *temp_pointer;
  connect_response_t connect_response;
  action_request_t *action_request;
  action_response_t action_response;
  disconnect_request_t *disconnect_request;
  disconnect_response_t disconnect_response;

  nc_init();
  game_window = nc_draw_space();

  zmq_bind_socket(rep_socket, SERVER_ADDRESS);

  srand((unsigned int)time(NULL));

  init_game(&game);

  nc_draw_starting_aliens(game_window, game);

  /* Game loop */
  while (game.aliens_alive) {
    temp_pointer = zmq_receive_msg(rep_socket, &msg_type);

    switch (msg_type) {
    case CONNECT_REQUEST:
      zmq_send_msg(rep_socket, CONNECT_RESPONSE, &connect_response);
      break;
    case ACTION_REQUEST:
      action_request = (action_request_t *)temp_pointer;
      zmq_send_msg(rep_socket, ACTION_RESPONSE, &action_response);
      break;
    case DISCONNECT_REQUEST:
      disconnect_request = (disconnect_request_t *)temp_pointer;
      zmq_send_msg(rep_socket, DISCONNECT_RESPONSE, &disconnect_response);
      break;
    case ALIENS_UPDATE_REQUEST:
      break;
    default:
      continue;
    }

    if (temp_pointer != NULL)
      free(temp_pointer);
  }

  nc_cleanup();
  zmq_cleanup(zmq_context, rep_socket, NULL);
}