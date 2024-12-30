/* Contains applications main functions that have the option to be ran in a
 * secondary thread, and receive threaded_mains_args_t to manage execution */

#include "threaded_mains.h"

void *astronaut_client_main(void *void_args) {

  /* Threaded args */
  threaded_mains_args_t *args = (threaded_mains_args_t *)void_args;
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
  zmq_send_msg(req_socket, ASTRONAUT_CONNECT_REQUEST, NULL, -1, NO_TOPIC);
  connect_response = (astronaut_connect_response_t *)zmq_receive_msg(
      req_socket, &msg_type, NO_TOPIC);

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
  if (args->threaded)
    pthread_mutex_lock(args->ncurses_lock);
  nc_init();
  window =
      nc_init_astronaut(player_orientation, player_id, args->threaded ? 22 : 0);
  if (args->threaded)
    pthread_mutex_unlock(args->ncurses_lock);

  /* Define known parts of the requests already */
  action_request.id = player_id;
  action_request.token = player_token;
  disconnect_request.id = player_id;
  disconnect_request.token = player_token;

  /* Game loop */
  while (!(stop_playing || (args->threaded && *args->terminate_threads))) {
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
      if (args->threaded)
        *args->terminate_threads = true;
      zmq_send_msg(req_socket, DISCONNECT_REQUEST, &disconnect_request, -1,
                   NO_TOPIC);
      status_code_and_score_response =
          (status_code_and_score_response_t *)zmq_receive_msg(
              req_socket, &msg_type, NO_TOPIC);
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
      zmq_send_msg(req_socket, ACTION_REQUEST, &action_request, -1, NO_TOPIC);

      status_code_and_score_response =
          (status_code_and_score_response_t *)zmq_receive_msg(
              req_socket, &msg_type, NO_TOPIC);
      player_score = status_code_and_score_response->player_score;
      free(status_code_and_score_response);

      send_action_message = false;
    }

    /* Position cursor and print current score */
    if (args->threaded)
      pthread_mutex_lock(args->ncurses_lock);
    wmove(window, 9, 1);
    wprintw(window, "Current score: %d", player_score);
    if (args->threaded)
      pthread_mutex_unlock(args->ncurses_lock);
  }

  /* Resources cleanup */
  zmq_cleanup(zmq_context, req_socket, NULL);
  if (args->threaded)
    pthread_mutex_lock(args->ncurses_lock);
  nc_cleanup();
  if (args->threaded)
    pthread_mutex_unlock(args->ncurses_lock);

  return NULL;
}

void *outer_space_display_main(void *void_args) {
  /* Threaded args */
  threaded_mains_args_t *args = (threaded_mains_args_t *)void_args;
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
  zmq_subscribe(sub_socket, GAME_UPDATES_TOPIC);

  /* Connect to server to get current game state */
  zmq_send_msg(req_socket, DISPLAY_CONNECT_REQUEST, NULL, -1, NO_TOPIC);
  display_connect_response = (display_connect_response_t *)zmq_receive_msg(
      req_socket, &msg_type, NO_TOPIC);
  assert(display_connect_response->status_code == 200);
  game = &display_connect_response->game;

  /* Ncurses initialization */
  if (args->threaded)
    pthread_mutex_lock(args->ncurses_lock);
  nc_init();
  game_window = nc_init_space();
  score_window = nc_init_scoreboard();
  nc_draw_init_game(game_window, score_window, *game);
  if (args->threaded)
    pthread_mutex_unlock(args->ncurses_lock);

  /* Game loop */
  while (!(game_ended || (args->threaded && *args->terminate_threads))) {
    temp_pointer = zmq_receive_msg(sub_socket, &msg_type, GAME_UPDATES_TOPIC);

    if (args->threaded)
      pthread_mutex_lock(args->ncurses_lock);

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
      if (args->threaded)
        *args->terminate_threads = true;
      break;

    default:
      if (temp_pointer != NULL)
        free(temp_pointer);
      if (args->threaded)
        pthread_mutex_unlock(args->ncurses_lock);
      continue;
    }

    if (temp_pointer != NULL)
      free(temp_pointer);

    /* Update scoreboard and refresh game windows */
    nc_update_scoreboard(score_window, game->players, game->aliens_alive);
    wrefresh(game_window);
    wrefresh(score_window);

    if (args->threaded)
      pthread_mutex_unlock(args->ncurses_lock);
  }

  /* Resources cleanup */
  if (args->threaded)
    pthread_mutex_lock(args->ncurses_lock);
  /* It might have exited without the game ending (when running
                  in threaded/joint mode and the user pressed Q) */
  if (game_ended)
    print_winning_player(game);
  nc_cleanup();
  if (args->threaded)
    pthread_mutex_unlock(args->ncurses_lock);
  free(display_connect_response);
  zmq_cleanup(zmq_context, req_socket, sub_socket);

  return NULL;
}