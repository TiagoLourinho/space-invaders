#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "utils.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

int main() {
  /* ZeroMQ/comms related */
  void *zmq_context = zmq_get_context();
  void *req_socket = zmq_create_socket(zmq_context, ZMQ_REQ);
  void *sub_socket = zmq_create_socket(zmq_context, ZMQ_SUB);
  MESSAGE_TYPE msg_type;
  /* Structs and temp pointer to receive/send requests/responses */
  void *temp_pointer;
  display_connect_response_t *display_connect_response;
  action_request_t *action_request;
  disconnect_request_t *disconnect_request;
  aliens_update_t *alien_update_request;
  /* Ncurses related */
  WINDOW *game_window, *score_window;
  /* Game management related */
  game_t *game;
  bool game_ended = false;

  /* ZeroMQ initialization */
  zmq_connect_socket(req_socket, SERVER_ZMQ_REQREP_ADDRESS);
  zmq_connect_socket(sub_socket, SERVER_ZMQ_PUBSUB_ADDRESS);
  zmq_subscribe(sub_socket, ""); /* Receive all broadcasted messages */

  /* Connect to server to get current game state */
  zmq_send_msg(req_socket, DISPLAY_CONNECT_REQUEST, NULL);
  display_connect_response =
      (display_connect_response_t *)zmq_receive_msg(req_socket, &msg_type);
  assert(display_connect_response->status_code == 200);
  game = &display_connect_response->game;

  /* Ncurses initialization */
  nc_init();
  game_window = nc_init_space();
  score_window = nc_init_scoreboard();
  nc_draw_init_game(game_window, score_window, *game);

  /* Game loop */
  while (!game_ended) {
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

    case ALIENS_UPDATE:
      alien_update_request = (aliens_update_t *)temp_pointer;
      handle_aliens_updates(game_window, alien_update_request, game);
      break;

    case GAME_ENDED:
      game_ended = true;
      break;

    default:
      break;
    }

    if (temp_pointer != NULL)
      free(temp_pointer);

    /* Update scoreboard and refresh game windows */
    nc_update_scoreboard(score_window, game->players);
    wrefresh(game_window);
    wrefresh(score_window);
  }

  print_winning_player(game);

  /* Resources cleanup */
  free(display_connect_response);
  nc_cleanup();
  zmq_cleanup(zmq_context, req_socket, sub_socket);

  return 0;
}