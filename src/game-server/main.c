#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "server_utils.h"
#include "validators.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

int main() {
  game_t game;
  player_t *current_player;
  alien_t *alien;
  position_t old_position;
  int n, status_code;
  /* Controls if the other process has been terminated */
  bool joined_alien_process = false;
  /* Ncurses related */
  WINDOW *game_window, *score_window;
  /* ZMQ and messages related */
  void *zmq_context = zmq_get_context();
  void *rep_socket = zmq_create_socket(zmq_context, ZMQ_REP);
  MESSAGE_TYPE msg_type;

  /* Structs and temp pointer to receive/send requests/responses */
  void *temp_pointer;
  connect_response_t connect_response;
  action_request_t *action_request;
  disconnect_request_t *disconnect_request;
  aliens_update_request_t *alien_update_request;
  only_status_code_response_t only_status_code_response;

  zmq_bind_socket(rep_socket, SERVER_ZMQ_BIND_ADDRESS);

  nc_init();
  game_window = nc_draw_space();
  score_window = nc_init_scoreboard();

  srand((unsigned int)time(NULL));

  init_game(&game);

  nc_draw_starting_aliens(game_window, game);

  /* Spawn the aliens update process that knows the starting position of the
   * aliens */
  spawn_alien_update_fork(game.aliens);

  /* Game loop */
  while (game.aliens_alive || !joined_alien_process) {
    temp_pointer = zmq_receive_msg(rep_socket, &msg_type);

    switch (msg_type) {
    case CONNECT_REQUEST:
      n = find_position_and_init_player(&game);

      if (n != -1) {
        /* Return player info */
        connect_response.status_code = 200;
        connect_response.id = n;
        connect_response.token = game.players[n].token;
        connect_response.orientation = game.players[n].orientation;

        nc_add_player(game_window, game.players[n]);
      } else {
        connect_response.status_code = 400;
      }

      zmq_send_msg(rep_socket, CONNECT_RESPONSE, &connect_response);
      break;

    case ACTION_REQUEST:
      action_request = (action_request_t *)temp_pointer;

      status_code = validate_action_request(*action_request, game);

      only_status_code_response.status_code = status_code;

      /* Move the player or shoot and update state */
      if (status_code == 200) {
        current_player = &game.players[action_request->id];

        if (action_request->action_type == MOVE) {
          /* Store old pos */
          old_position.col = current_player->position.col;
          old_position.row = current_player->position.row;

          update_position(&current_player->position,
                          action_request->movement_direction);

          nc_move_player(game_window, *current_player, old_position);
        } else if (action_request->action_type == ZAP) {
          player_zap(game_window, &game, action_request->id);
          nc_draw_zap(game_window, &game, current_player);
          usleep(ZAP_TIME_ON_SCREEN * 1000);
          nc_clean_zap(game_window, &game, current_player);
        }
      }

      zmq_send_msg(rep_socket, ACTION_RESPONSE, &only_status_code_response);
      break;

    case DISCONNECT_REQUEST:
      disconnect_request = (disconnect_request_t *)temp_pointer;

      status_code = validate_disconnect_request(*disconnect_request, game);

      only_status_code_response.status_code = status_code;

      /* Remove player from screen and state */
      if (status_code == 200) {
        current_player = &game.players[disconnect_request->id];
        nc_clean_position(game_window, current_player->position);
        current_player->connected = false;
      }

      zmq_send_msg(rep_socket, DISCONNECT_RESPONSE, &only_status_code_response);
      break;

    case ALIENS_UPDATE_REQUEST:
      alien_update_request = (aliens_update_request_t *)temp_pointer;

      if (game.aliens_alive) {
        only_status_code_response.status_code = 200;

        /*
        Two loops to clean the old positions of the aliens and then put the
        new ones (can't be done in just 1 iteration because there would be
        problems with overlaps between new and old positions)
        */
        for (int i = 0; i < N_ALIENS; i++) {
          alien = &game.aliens[i];
          if (alien->alive)
            nc_clean_position(game_window, alien->position);
        }
        for (int i = 0; i < N_ALIENS; i++) {
          alien = &game.aliens[i];
          if (alien->alive) {
            alien->position.col = alien_update_request->aliens[i].position.col;
            alien->position.row = alien_update_request->aliens[i].position.row;

            nc_add_alien(game_window, &alien->position);
          }
        }

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

  /* Clean screen and print winning player */
  n = -1; // Temp variable to hold the winning player ID
  for (int i = 0; i < MAX_PLAYERS; i++) {
    current_player = &game.players[i];

    if (current_player->connected &&
        (n == -1 || current_player->score >= game.players[n].score))
      n = i;
  }
  erase(); /* Clean entire ncurses screen */
  if (n != -1)
    printw("Player %c won with %d points!\n", id_to_symbol(n),
           game.players[n].score);
  refresh(); /* Refresh entire ncurses screen */
  sleep(5);

  nc_cleanup();
  zmq_cleanup(zmq_context, rep_socket, NULL);
}