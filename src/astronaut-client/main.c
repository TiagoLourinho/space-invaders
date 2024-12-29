#include "threaded_mains.h"

int main() {
  threaded_mains_args_t args;
  args.threaded = false;
  args.ncurses_lock = NULL;
  args.terminate_threads = NULL;

  astronaut_client_main(&args);
  return 0;
}