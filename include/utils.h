#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

char id_to_symbol(int id);

uint64_t get_timestamp_ms();

#endif // UTILS_H