#include <stdint.h>
#include <stdbool.h>

typedef struct {
  /** If true, this tile is still hidden. */
  bool masked;
  /** If -1, this tile is a mine. */
  int8_t neighboring_mines;
} Tile;

void start(uint16_t field_width, uint16_t field_height);
