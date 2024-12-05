#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "server_utils.h"
#include "validators.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <zmq.h>

int main() {
  game_t game;
  player_t *current_player;
  position_t old_position;
  int n, status_code;
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
  action_response_t action_response;
  disconnect_request_t *disconnect_request;
  disconnect_response_t disconnect_response;

  nc_init();
  game_window = nc_draw_space();
  score_window = nc_init_scoreboard();

  zmq_bind_socket(rep_socket, SERVER_ADDRESS);

  srand((unsigned int)time(NULL));

  init_game(&game);

  nc_draw_starting_aliens(game_window, game);

  /* Game loop */
  while (game.aliens_alive) {
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

      action_response.status_code = status_code;

      /* Move the player or shoot and update state */
      if (status_code == 200) {
        current_player = &game.players[action_request->id];

        if (action_request->action_type == MOVE) {
          /* Store old pos */
          old_position.col = current_player->position.col;
          old_position.row = current_player->position.row;

          update_player_position(current_player,
                                 action_request->movement_direction);

          nc_move_player(game_window, *current_player, old_position);
        } else if (action_request->action_type == ZAP) {
          player_zap(game_window, &game, action_request->id);
        }
      }

      zmq_send_msg(rep_socket, ACTION_RESPONSE, &action_response);
      break;

    case DISCONNECT_REQUEST:
      disconnect_request = (disconnect_request_t *)temp_pointer;

      status_code = validate_disconnect_request(*disconnect_request, game);

      disconnect_response.status_code = status_code;

      /* Remove player from screen and state */
      if (status_code == 200) {
        current_player = &game.players[disconnect_request->id];
        nc_clean_position(game_window, current_player->position);
        current_player->connected = false;
      }

      zmq_send_msg(rep_socket, DISCONNECT_RESPONSE, &disconnect_response);
      break;

    case ALIENS_UPDATE_REQUEST:
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

  nc_cleanup();
  zmq_cleanup(zmq_context, rep_socket, NULL);
}