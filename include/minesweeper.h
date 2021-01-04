#include <stdint.h>
#include <stdbool.h>

typedef struct {
  /** If true, this tile is still hidden. */
  bool masked;
  /** If -1, this tile is a mine. */
  int8_t neighboring_mines;
} Tile;

void start(uint8_t fw, uint8_t fh, uint8_t mine_count);
