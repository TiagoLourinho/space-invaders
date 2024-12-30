#include "threaded_mains.h"

int main() {
  threaded_mains_args_t args;
  pthread_mutex_t ncurses_lock;
  bool terminate_threads = false;
  pthread_t astronaut_client, outer_space_display;

  assert(pthread_mutex_init(&ncurses_lock, NULL) == 0);
  args.threaded = true;
  args.ncurses_lock = &ncurses_lock;
  args.terminate_threads = &terminate_threads;

  assert(pthread_create(&outer_space_display, NULL, outer_space_display_main,
                        &args) == 0);

  /* The astronaut client should only start after the outer space display
   * is initialized, however, as active wait cant be used (to wait for a flag
   * for example) sleep a bit to allow it to finish */
  usleep(250000); // 0.25 s

  assert(pthread_create(&astronaut_client, NULL, astronaut_client_main,
                        &args) == 0);

  pthread_join(astronaut_client, NULL);
  pthread_join(outer_space_display, NULL);

  pthread_mutex_destroy(&ncurses_lock);

  return 0;
}