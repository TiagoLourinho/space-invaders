#ifndef THREADED_FUNCTIONS_H
#define THREADED_FUNCTIONS_H

#include "comms.h"
#include "game_def.h"
#include "ncurses_wrapper.h"
#include "utils.h"
#include "zeromq_wrapper.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

int astronaut_client_main();
int outer_space_display_main();

#endif // THREADED_FUNCTIONS_H