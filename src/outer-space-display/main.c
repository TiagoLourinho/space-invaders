#include "threaded_mains.h"

int main() {
  threaded_mains_args_t args;
  pthread_mutex_t ncurses_lock;
  bool terminate_threads = false;

  /* Even though no thread is created here, some threads will be created later
   * to clean up the player's zaps and will use this lock */
  assert(pthread_mutex_init(&ncurses_lock, NULL) == 0);
  args.threaded = true;
  args.ncurses_lock = &ncurses_lock;
  args.terminate_threads = &terminate_threads;

  outer_space_display_main(&args);

  pthread_mutex_destroy(&ncurses_lock);

  return 0;
}