#include <stdint.h>
#include <stdbool.h>

typedef struct {
  /** If true, this tile is still hidden. */
  bool masked;
  /** If -1, this tile is a mine. */
  int8_t neighboring_mines;
} Tile;

void start(uint16_t fw, uint16_t fh, uint16_t mine_count);
