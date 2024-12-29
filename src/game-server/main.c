#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "utils.h"
#include "validators.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <pthread.h>
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
  status_code_and_score_response_t status_code_and_score_response;
  /* Ncurses related */
  WINDOW *game_window, *score_window;
  /* Game state and authentication management */
  game_t game;
  int tokens[MAX_PLAYERS]; /* The authentication tokens used by the players */
  /* Aliens update thread */
  pthread_t thread_id;
  aliens_update_thread_args_t thread_args;
  pthread_mutex_t lock;

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

  /* Aliens update thread creation */
  if (pthread_mutex_init(&lock, NULL) != 0)
    exit(-1);
  thread_args.game = &game;
  thread_args.game_window = game_window;
  thread_args.score_window = score_window;
  thread_args.pub_socket = pub_socket;
  thread_args.lock = &lock;
  if (pthread_create(&thread_id, NULL, aliens_update_thread, &thread_args) != 0)
    exit(-1);

  /* Game loop */
  while (game.aliens_alive) {
    temp_pointer = zmq_receive_msg(rep_socket, &msg_type);

    /*
    ========= Entering critical region =========

    The thread of the aliens update uses the game state, windows and publish
    socket, so the requests can't be handled without using those resources
    */
    pthread_mutex_lock(&lock);

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

    default:
      continue;
    }

    if (temp_pointer != NULL)
      free(temp_pointer);

    /* Update scoreboard and refresh game windows */
    nc_update_scoreboard(score_window, game.players, game.aliens_alive);
    wrefresh(game_window);
    wrefresh(score_window);

    /* ========= Leaving critical region ========= */
    pthread_mutex_unlock(&lock);
  }

  /* Publish final update because game ended */
  zmq_send_msg(pub_socket, GAME_ENDED, NULL);

  pthread_join(thread_id, NULL);
  print_winning_player(&game);

  /* Resources cleanup */
  pthread_mutex_destroy(&lock);
  nc_cleanup();
  zmq_cleanup(zmq_context, rep_socket, pub_socket);
}