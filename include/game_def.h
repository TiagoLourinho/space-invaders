/* Defines game-related */

#ifndef GAME_DEF_H
#define GAME_DEF_H

#include <stdbool.h>

#define SPACE_SIZE 20
#define MAX_PLAYERS 8
#define N_ALIENS (SPACE_SIZE * SPACE_SIZE) / 3

typedef enum { VERTICAL, HORIZONTAL } MOVEMENT_ORIENTATION;
typedef enum { UP, RIGHT, DOWN, LEFT, NO_MOVEMENT } MOVEMENT_DIRECTION;
typedef enum { MOVE, ZAP } ACTION_TYPE;

typedef struct {
  int row;
  int col;
} position_t;

typedef struct {
  /* The id of the player (that corresponds to its  position on the players
   * array)*/
  int id;
  /* Defines if the player is connected/playing or if the position is free */
  bool connected;
  MOVEMENT_ORIENTATION orientation;
  position_t position;
  /* The current score (-1 if not connected) */
  int score;
  /* Contains the timestamp where the player was last stunned (starts at -1) */
  int last_stunned;
  /* Contains the timestamp where the player last shot (starts at -1) */
  int last_shot;
  /* The authentication token assigned to the player */
  int token;
} player_t;

typedef struct {
  bool alive;
  position_t position;
} alien_t;

typedef struct {
  player_t players[MAX_PLAYERS];
  /* Game ends when it reaches 0 */
  int aliens_alive;
  alien_t aliens[N_ALIENS];
} game_t;

#endif // GAME_DEF_H