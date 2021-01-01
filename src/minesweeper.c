#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "minesweeper.h"
#include "constants.h"
#include "error.h"

Tile create_empty_tile() {
  Tile tile;
  tile.masked = true;
  tile.neighboring_mines = 0;
  return tile;
}

Tile *tiles;
uint16_t tiles_width, tiles_height;

static inline Tile *get_tile(uint16_t x, uint16_t y) {
  return &tiles[y * tiles_width + x];
}

/** Allocate a field of tiles. */
void alloc_tiles();
/** Free the allocated tiles. */
void free_tiles();
void print_field();
bool handle_input(uint16_t *x, uint16_t *y);

/** Start game. */
void start(uint16_t field_width, uint16_t field_height) {
  tiles_width = field_width;
  tiles_height = field_height;

  alloc_tiles();
  for (;;) {
    print_field();
    uint16_t x, y;
    if (!handle_input(&x, &y))
      break;
    get_tile(x, y)->masked = false;
  }
  free_tiles();
}

void alloc_tiles() {
  tiles = (Tile*) malloc(tiles_width * tiles_height * sizeof(Tile));
  for (int i = 0; i < tiles_width * tiles_height; i++)
    tiles[i] = create_empty_tile();
}

void free_tiles() {
  free(tiles);
}

void print_field() {
  for (int y = 0; y < tiles_height; y++)
    for (int x = 0; x < tiles_width; x++)
      printf(
        "%c%c",
        // if the tile is masked, print an 'X'
        get_tile(x, y)->masked ? MASKED_TILE_CHAR : EMPTY_TILE_CHAR,
        // if this is the last x, move to the next line
        x == tiles_width - 1 ? '\n' : ' '
      );
}

uint16_t digits_to_u16(uint8_t *buf, size_t size) {
  uint16_t res = 0;
  for (int i = 0; i < size; i++)
    res += buf[i] * pow(10, size - i - 1);
  return res;
}

uint8_t digit_buffer[100];
size_t digit_buffer_len;

bool parse_coord(uint16_t *x, uint16_t *y, bool *x_set, bool *y_set) {
  uint16_t num = digits_to_u16(digit_buffer, digit_buffer_len);
  digit_buffer_len = 0;

  if (!*x_set) {
    if (num == 0 || num > tiles_width) {
      printf(ERROR_PREFIX "x has to be between 1 and %d (got %d).\n", tiles_width, num);
      return false;
    }

    *x = num - 1;
    *x_set = true;
  } else if (!*y_set) {
    if (num == 0 || num > tiles_height) {
      printf(ERROR_PREFIX "y has to be between 1 and %d (got %d).\n", tiles_height, num);
      return false;
    }

    *y = num - 1;
    *y_set = true;
  } else {
    printf(ERROR_PREFIX "Too many coordinates; x and y expected.\n");
    return false;
  }

  return true;
}

bool parse_coords(char *s, size_t len, uint16_t *x, uint16_t *y) {
  bool x_set = false, y_set = false;
  digit_buffer_len = 0;

  for (int i = 0; i < len; i++) {
    const char c = s[i];

    if (c >= '0' && c <= '9')
      // store the digit in `num_buffer`
      digit_buffer[digit_buffer_len++] = c - '0';
    else if (c == LOCATION_DELIMITER) {
      if (!parse_coord(x, y, &x_set, &y_set))
        return false;
    }
  }

  return parse_coord(x, y, &x_set, &y_set);
}

bool handle_input(uint16_t *x, uint16_t *y) {
  for (;;) {
    char *s;
    size_t len = 0;
    printf("Coord: ");
    const ssize_t read = getline(&s, &len, stdin);

    if (read == -1)
      return false;

    if (parse_coords(s, len, x, y))
      break;
  }

  return true;
}
