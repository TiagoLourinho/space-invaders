#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "utils.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdlib.h>
#include <zmq.h>

int main() {
  /* ZeroMQ/comms related */
  void *zmq_context = zmq_get_context();
  void *req_socket = zmq_create_socket(zmq_context, ZMQ_REQ);
  MESSAGE_TYPE msg_type;
  bool send_action_message = false;
  /* Ncurses */
  WINDOW *window;
  /* Structs to receive and send the requests */
  astronaut_connect_response_t *connect_response;
  action_request_t action_request;
  status_code_and_score_response_t *status_code_and_score_response;
  disconnect_request_t disconnect_request;
  /* Game management related */
  int key_pressed;
  bool stop_playing = false;
  /* Player info (received when connected)*/
  int player_id;
  int player_token;
  int player_score = 0;
  MOVEMENT_ORIENTATION player_orientation;

  /* ZeroMQ initialization */
  zmq_connect_socket(req_socket, SERVER_ZMQ_REQREP_ADDRESS);

  /* Connect to server to get player info */
  zmq_send_msg(req_socket, ASTRONAUT_CONNECT_REQUEST, NULL);
  connect_response =
      (astronaut_connect_response_t *)zmq_receive_msg(req_socket, &msg_type);

  if (connect_response->status_code != 200) {
    printf("Game is full (%d players currently playing).\n", MAX_PLAYERS);
    zmq_cleanup(zmq_context, req_socket, NULL);
    exit(-1);
  } else {
    player_id = connect_response->id;
    player_token = connect_response->token;
    player_orientation = connect_response->orientation;
  }
  free(connect_response);

  /* Ncurses initialization */
  nc_init();
  window = nc_init_astronaut(player_orientation, player_id);

  /* Define known parts of the requests already */
  action_request.id = player_id;
  action_request.token = player_token;
  disconnect_request.id = player_id;
  disconnect_request.token = player_token;

  /* Game loop */
  while (!stop_playing) {
    key_pressed = wgetch(window);

    switch (key_pressed) {
    case 65: // KEY_UP

      if (player_orientation == HORIZONTAL) /* Player can't move vertically */
        break;
      send_action_message = true;
      action_request.action_type = MOVE;
      action_request.movement_direction = UP;
      break;

    case 66: // KEY_DOWN

      if (player_orientation == HORIZONTAL) /* Player can't move vertically */
        break;
      send_action_message = true;
      action_request.action_type = MOVE;
      action_request.movement_direction = DOWN;
      break;

    case 68: // KEY_LEFT

      if (player_orientation == VERTICAL) /* Player can't move horizontally */
        break;
      send_action_message = true;
      action_request.action_type = MOVE;
      action_request.movement_direction = LEFT;
      break;

    case 67: // KEY_RIGHT

      if (player_orientation == VERTICAL) /* Player can't move horizontally */
        break;
      send_action_message = true;
      action_request.action_type = MOVE;
      action_request.movement_direction = RIGHT;
      break;

    case 32: // Spacebar
      send_action_message = true;
      action_request.action_type = ZAP;
      action_request.movement_direction = NO_MOVEMENT;
      break;

    case 113: // q
    case 81:  // Q

      /* Send disconnect message and stop playing */
      send_action_message = false;
      stop_playing = true;
      zmq_send_msg(req_socket, DISCONNECT_REQUEST, &disconnect_request);
      status_code_and_score_response =
          (status_code_and_score_response_t *)zmq_receive_msg(req_socket,
                                                              &msg_type);
      assert(status_code_and_score_response->status_code == 200);
      player_score = status_code_and_score_response->player_score;
      free(status_code_and_score_response);
      break;

    default:
      /* No messages are sent when another key is pressed */
      send_action_message = false;
      break;
    }

    /* Only send the message if a valid action key was pressed */
    if (send_action_message) {
      zmq_send_msg(req_socket, ACTION_REQUEST, &action_request);

      status_code_and_score_response =
          (status_code_and_score_response_t *)zmq_receive_msg(req_socket,
                                                              &msg_type);
      player_score = status_code_and_score_response->player_score;
      free(status_code_and_score_response);

      send_action_message = false;
    }

    /* Position cursor and print current score */
    wmove(window, 9, 1);
    wprintw(window, "Current score: %d", player_score);
  }

  /* Resources cleanup */
  zmq_cleanup(zmq_context, req_socket, NULL);
  nc_cleanup();

  return 0;
}