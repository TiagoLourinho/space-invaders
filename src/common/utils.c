/* Defines general utilities */

#include "utils.h"

/* Converts an ID to a symbol */
char id_to_symbol(int id) { return (char)('A' + id); }

/* Returns the current timestamp in ms since epoch */
uint64_t get_timestamp_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}
