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
  int n;
  game_t *game;
  player_t *current_player;
  alien_t *alien;
  position_t old_position;
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
      /* No need to manage tokens */
      n = find_position_and_init_player(game, NULL);

      if (n != -1) {
        nc_add_player(game_window, game->players[n]);
      }

      break;
    case ACTION_REQUEST:
      action_request = (action_request_t *)temp_pointer;
      current_player = &game->players[action_request->id];

      if (action_request->action_type == MOVE) {
        /* Store old pos */
        old_position.col = current_player->position.col;
        old_position.row = current_player->position.row;

        update_position(&current_player->position,
                        action_request->movement_direction);

        nc_move_player(game_window, *current_player, old_position);
      } else if (action_request->action_type == ZAP) {
        player_zap(game_window, game, action_request->id);
        nc_draw_zap(game_window, game, current_player);
        usleep(ZAP_TIME_ON_SCREEN * 1000);
        nc_clean_zap(game_window, game, current_player);
      }

      break;
    case DISCONNECT_REQUEST:
      disconnect_request = (disconnect_request_t *)temp_pointer;

      current_player = &game->players[disconnect_request->id];
      nc_clean_position(game_window, current_player->position);
      current_player->connected = false;
      break;
    case ALIENS_UPDATE_REQUEST:
      alien_update_request = (aliens_update_request_t *)temp_pointer;

      /*
      Two loops to clean the old positions of the aliens and then put the
      new ones (can't be done in just 1 iteration because there would be
      problems with overlaps between new and old positions)
      */
      for (int i = 0; i < N_ALIENS; i++) {
        alien = &game->aliens[i];
        if (alien->alive)
          nc_clean_position(game_window, alien->position);
      }
      for (int i = 0; i < N_ALIENS; i++) {
        alien = &game->aliens[i];
        if (alien->alive) {
          alien->position.col = alien_update_request->aliens[i].position.col;
          alien->position.row = alien_update_request->aliens[i].position.row;

          nc_add_alien(game_window, &alien->position);
        }
      }

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