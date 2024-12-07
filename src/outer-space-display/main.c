#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "utils.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

int main() {
  void *zmq_context = zmq_get_context();
  void *req_socket = zmq_create_socket(zmq_context, ZMQ_REQ);
  void *sub_socket = zmq_create_socket(zmq_context, ZMQ_SUB);
  MESSAGE_TYPE msg_type;
  game_t *game;
  WINDOW *game_window, *score_window;
  /* Structs and temp pointer to receive/send requests/responses */
  void *temp_pointer;
  display_connect_response_t *display_connect_response;
  action_request_t *action_request;
  disconnect_request_t *disconnect_request;
  aliens_update_request_t *alien_update_request;

  /* Connect sockets */
  zmq_connect_socket(req_socket, SERVER_ZMQ_REQREP_ADDRESS);
  zmq_connect_socket(sub_socket, SERVER_ZMQ_PUBSUB_ADDRESS);
  zmq_subscribe(sub_socket, ""); /* Receive all broadcasted messages */

  /* Send initial request to get game state */
  msg_type = DISPLAY_CONNECT_REQUEST;
  zmq_send_msg(req_socket, msg_type, NULL);
  /* Freed later */
  display_connect_response =
      (display_connect_response_t *)zmq_receive_msg(req_socket, &msg_type);
  assert(display_connect_response->status_code == 200);
  game = &display_connect_response->game;

  /* Init windows */
  nc_init();
  game_window = nc_draw_space();
  score_window = nc_init_scoreboard();
  nc_draw_init_game(game_window, score_window, *game);

  while (true) {
    temp_pointer = zmq_receive_msg(sub_socket, &msg_type);

    switch (msg_type) {
    case ASTRONAUT_CONNECT_REQUEST:
      /* NULL because display doesn't send a reply or manage tokens */
      handle_player_connect(game_window, NULL, NULL, game);
      break;

    case ACTION_REQUEST:
      action_request = (action_request_t *)temp_pointer;
      handle_player_action(action_request, &game->players[action_request->id],
                           game_window, game);
      break;

    case DISCONNECT_REQUEST:
      disconnect_request = (disconnect_request_t *)temp_pointer;
      handle_player_disconnect(game_window,
                               &game->players[disconnect_request->id]);
      break;

    case ALIENS_UPDATE_REQUEST:
      alien_update_request = (aliens_update_request_t *)temp_pointer;
      handle_aliens_updates(game_window, alien_update_request, game);
      break;

    default:
      break;
    }

    if (temp_pointer != NULL)
      free(temp_pointer);

    nc_update_scoreboard(score_window, game->players);

    nc_update_screen(game_window);
    nc_update_screen(score_window);
  }

  free(display_connect_response);
  nc_cleanup();
  zmq_cleanup(zmq_context, req_socket, sub_socket);

  return 0;
}