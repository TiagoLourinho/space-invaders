#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "utils.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdlib.h>
#include <zmq.h>

int main() {
  void *zmq_context = zmq_get_context();
  void *req_socket = zmq_create_socket(zmq_context, ZMQ_REQ);
  MESSAGE_TYPE msg_type;
  int key_pressed;
  bool send_action_message = false;
  bool stop_playing = false;

  /* Structs to receive and send the requests */
  connect_response_t *connect_response;
  action_request_t action_request;
  only_status_code_response_t *status_code_response;
  disconnect_request_t disconnect_request;

  /* Player info (received when connected)*/
  int player_id;
  int player_token;
  MOVEMENT_ORIENTATION player_orientation;

  /* Connect to server */
  zmq_connect_socket(req_socket, SERVER_ZMQ_ADDRESS);
  msg_type = CONNECT_REQUEST;
  zmq_send_msg(req_socket, msg_type, NULL);
  connect_response =
      (connect_response_t *)zmq_receive_msg(req_socket, &msg_type);

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

  nc_init();

  /* Define known parts of the requests already */
  action_request.id = player_id;
  action_request.token = player_token;
  disconnect_request.id = player_id;
  disconnect_request.token = player_token;

  /* Print instructions */
  printw("You are playing as player: %c\n\n", id_to_symbol(player_id));
  printw("Controls:\n");
  if (player_orientation == VERTICAL) {
    printw("\t UP ARROW\t-> Move up\n");
    printw("\t DOWN ARROW\t-> Move down\n");
  } else {
    printw("\t RIGHT ARROW\t-> Move right\n");
    printw("\t LEFT ARROW\t-> Move left\n");
  }
  printw("\t SPACEBAR\t-> Zap\n");
  printw("\t q/Q\t\t-> Stop playing\n");

  /* Game loop */
  while (!stop_playing) {
    key_pressed = getch();

    switch (key_pressed) {
    case KEY_UP:
      if (player_orientation == HORIZONTAL) /* Player can't move vertically */
        break;
      send_action_message = true;
      action_request.action_type = MOVE;
      action_request.movement_direction = UP;
      break;
    case KEY_DOWN:
      if (player_orientation == HORIZONTAL) /* Player can't move vertically */
        break;
      send_action_message = true;
      action_request.action_type = MOVE;
      action_request.movement_direction = DOWN;
      break;
    case KEY_LEFT:
      if (player_orientation == VERTICAL) /* Player can't move horizontally */
        break;
      send_action_message = true;
      action_request.action_type = MOVE;
      action_request.movement_direction = LEFT;
      break;
    case KEY_RIGHT:
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
      msg_type = DISCONNECT_REQUEST;
      zmq_send_msg(req_socket, msg_type, &disconnect_request);
      status_code_response =
          (only_status_code_response_t *)zmq_receive_msg(req_socket, &msg_type);
      assert(status_code_response->status_code == 200);
      free(status_code_response);
      break;

    default:
      /* No messages are sent when another key is pressed */
      send_action_message = false;
      break;
    }

    /* Only send the message if a valid action key was pressed */
    if (send_action_message) {
      msg_type = ACTION_REQUEST;
      zmq_send_msg(req_socket, msg_type, &action_request);

      status_code_response =
          (only_status_code_response_t *)zmq_receive_msg(req_socket, &msg_type);

      free(status_code_response);

      send_action_message = false;
    }
  }

  zmq_cleanup(zmq_context, req_socket, NULL);
  nc_cleanup();

  return 0;
}