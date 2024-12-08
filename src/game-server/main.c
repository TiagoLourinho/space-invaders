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
  /* ZeroMQ/comms related */
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
  status_code_and_score_response_t status_code_and_score_response;
  /* Ncurses related */
  WINDOW *game_window, *score_window;
  /* Game state and authentication management */
  game_t game;
  int tokens[MAX_PLAYERS]; /* The authentication tokens used by the players */
  bool joined_alien_process =
      false; /* Controls if the other process has been terminated */

  /* ZeroMQ initialization */
  zmq_bind_socket(rep_socket, SERVER_ZMQ_REQREP_BIND_ADDRESS);
  zmq_bind_socket(pub_socket, SERVER_ZMQ_PUBSUB_BIND_ADDRESS);

  /* Ncurses initialization */
  nc_init();
  game_window = nc_init_space();
  score_window = nc_init_scoreboard();

  /* Initialize game and spawn helper child process to manage aliens updated */
  srand((unsigned int)time(NULL)); /* Used for the aliens positions */
  init_game(&game, tokens);
  nc_draw_init_game(game_window, score_window, game);
  spawn_alien_update_fork(game.aliens);

  /* Game loop */
  while (game.aliens_alive || !joined_alien_process) {
    temp_pointer = zmq_receive_msg(rep_socket, &msg_type);

    switch (msg_type) {
    case DISPLAY_CONNECT_REQUEST: /* Received by the displays clients */
      display_connect_response.status_code = 200;
      copy_game_state_for_display(&display_connect_response, &game);
      zmq_send_msg(rep_socket, DISPLAY_CONNECT_RESPONSE,
                   &display_connect_response);
      break;

    case ASTRONAUT_CONNECT_REQUEST: /* Received by the astronaut clients */
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

    case ACTION_REQUEST: /* Received by the astronaut clients */
      action_request = (action_request_t *)temp_pointer;

      status_code_and_score_response.status_code =
          validate_action_request(*action_request, game, tokens);

      if (status_code_and_score_response.status_code == 200) {
        /* Publish update */
        action_request->token = -1; /* Invalidate token */
        zmq_send_msg(pub_socket, ACTION_REQUEST, action_request);

        handle_player_action(action_request, &game.players[action_request->id],
                             game_window, &game);

        status_code_and_score_response.player_score =
            game.players[action_request->id].score;
      }

      zmq_send_msg(rep_socket, ACTION_RESPONSE,
                   &status_code_and_score_response);
      break;

    case DISCONNECT_REQUEST: /* Received by the astronaut clients */
      disconnect_request = (disconnect_request_t *)temp_pointer;

      status_code_and_score_response.status_code =
          validate_disconnect_request(*disconnect_request, game, tokens);

      if (status_code_and_score_response.status_code == 200) {
        /* Publish update */
        disconnect_request->token = -1; /* Invalidate token */
        zmq_send_msg(pub_socket, DISCONNECT_REQUEST, disconnect_request);

        handle_player_disconnect(game_window,
                                 &game.players[disconnect_request->id]);

        status_code_and_score_response.player_score =
            game.players[disconnect_request->id].score;
      }

      zmq_send_msg(rep_socket, DISCONNECT_RESPONSE,
                   &status_code_and_score_response);
      break;

    case ALIENS_UPDATE_REQUEST: /* Received by the child process */
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

    /* Update scoreboard and refresh game windows */
    nc_update_scoreboard(score_window, game.players);
    wrefresh(game_window);
    wrefresh(score_window);
  }

  /* Publish final update because game ended */
  zmq_send_msg(pub_socket, GAME_ENDED, NULL);

  print_winning_player(&game);

  /* Resources cleanup */
  nc_cleanup();
  zmq_cleanup(zmq_context, rep_socket, pub_socket);
}