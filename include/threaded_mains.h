#ifndef THREADED_FUNCTIONS_H
#define THREADED_FUNCTIONS_H

#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "utils.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

typedef struct {
  bool threaded;           /* Whether or not it is running in threaded mode */
  bool *terminate_threads; /* Shared variable responsible for terminating all
                              threads */
  pthread_mutex_t
      *ncurses_lock; /* The lock used to access ncurses in threaded mode */

} threaded_mains_args_t;

/* Thread ready implementation of the astronaut client main */
void *astronaut_client_main(void *void_args);

/* Thread ready implementation of the outer-space-display main */
void *outer_space_display_main(void *void_args);

#endif // THREADED_FUNCTIONS_H