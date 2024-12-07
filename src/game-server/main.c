#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "utils.h"
#include "validators.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

int main() {
  game_t game;
  int tokens[MAX_PLAYERS]; /* The authentication tokens used by the players */
  /* Controls if the other process has been terminated */
  bool joined_alien_process = false;
  /* Ncurses related */
  WINDOW *game_window, *score_window;
  /* ZMQ and messages related */
  void *zmq_context = zmq_get_context();
  void *rep_socket = zmq_create_socket(zmq_context, ZMQ_REP);
  void *pub_socket = zmq_create_socket(zmq_context, ZMQ_PUB);
  MESSAGE_TYPE msg_type;

  /* Structs and temp pointer to receive/send requests/responses */
  void *temp_pointer;
  astronaut_connect_response_t astronaut_connect_response;
  display_connect_response_t display_connect_response;
  action_request_t *action_request;
  disconnect_request_t *disconnect_request;
  aliens_update_request_t *alien_update_request;
  only_status_code_response_t only_status_code_response;

  zmq_bind_socket(rep_socket, SERVER_ZMQ_REQREP_BIND_ADDRESS);
  zmq_bind_socket(pub_socket, SERVER_ZMQ_PUBSUB_BIND_ADDRESS);

  nc_init();
  game_window = nc_draw_space();
  score_window = nc_init_scoreboard();

  srand((unsigned int)time(NULL));

  init_game(&game, tokens);

  nc_draw_init_game(game_window, score_window, game);

  /* Spawn the aliens update process that knows the starting position of the
   * aliens */
  spawn_alien_update_fork(game.aliens);

  /* Game loop */
  while (game.aliens_alive || !joined_alien_process) {
    temp_pointer = zmq_receive_msg(rep_socket, &msg_type);

    switch (msg_type) {
    case DISPLAY_CONNECT_REQUEST:
      display_connect_response.status_code = 200;
      copy_game_state(&display_connect_response, &game);
      zmq_send_msg(rep_socket, DISPLAY_CONNECT_RESPONSE,
                   &display_connect_response);
      break;

    case ASTRONAUT_CONNECT_REQUEST:
      astronaut_connect_response.status_code = validate_connect_request(game);

      if (astronaut_connect_response.status_code == 200) {

        /* Publish update */
        zmq_send_msg(pub_socket, ASTRONAUT_CONNECT_REQUEST, NULL);

        handle_player_connect(game_window, &astronaut_connect_response, tokens,
                              &game);
      }

      zmq_send_msg(rep_socket, ASTROUNAUT_CONNECT_RESPONSE,
                   &astronaut_connect_response);
      break;

    case ACTION_REQUEST:
      action_request = (action_request_t *)temp_pointer;

      only_status_code_response.status_code =
          validate_action_request(*action_request, game, tokens);

      if (only_status_code_response.status_code == 200) {
        /* Publish update */
        action_request->token = -1; /* Invalidate token */
        zmq_send_msg(pub_socket, ACTION_REQUEST, action_request);

        handle_player_action(action_request, &game.players[action_request->id],
                             game_window, &game);
      }

      zmq_send_msg(rep_socket, ACTION_RESPONSE, &only_status_code_response);
      break;

    case DISCONNECT_REQUEST:
      disconnect_request = (disconnect_request_t *)temp_pointer;

      only_status_code_response.status_code =
          validate_disconnect_request(*disconnect_request, game, tokens);

      if (only_status_code_response.status_code == 200) {

        /* Publish update */
        disconnect_request->token = -1; /* Invalidate token */
        zmq_send_msg(pub_socket, DISCONNECT_REQUEST, disconnect_request);

        handle_player_disconnect(game_window,
                                 &game.players[disconnect_request->id]);
      }

      zmq_send_msg(rep_socket, DISCONNECT_RESPONSE, &only_status_code_response);
      break;

    case ALIENS_UPDATE_REQUEST:
      alien_update_request = (aliens_update_request_t *)temp_pointer;

      if (game.aliens_alive) {
        only_status_code_response.status_code = 200;

        /* Publish update */
        zmq_send_msg(pub_socket, ALIENS_UPDATE_REQUEST, alien_update_request);

        handle_aliens_updates(game_window, alien_update_request, &game);

      } else {
        /* Game has ended, send 400 to terminate the other process */
        only_status_code_response.status_code = 400;
        joined_alien_process = true;
      }

      zmq_send_msg(rep_socket, ALIENS_UPDATE_RESPONSE,
                   &only_status_code_response);
      break;

    default:
      continue;
    }

    if (temp_pointer != NULL)
      free(temp_pointer);

    nc_update_scoreboard(score_window, game.players);

    nc_update_screen(game_window);
    nc_update_screen(score_window);
  }

  /* Publish update */
  zmq_send_msg(pub_socket, GAME_ENDED, NULL);

  print_winning_player(&game);

  nc_cleanup();
  zmq_cleanup(zmq_context, rep_socket, pub_socket);
}