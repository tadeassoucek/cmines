#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "colors.h"
#include "minesweeper.h"

#define MASKED_TILE_CHAR '+'
#define EMPTY_TILE_CHAR '.'
#define MINE_TILE_CHAR 'X'

#define LOCATION_DELIMITER ':'

#define ERROR_PREFIX COLOR_ERROR "Error:" COLOR_RESET " "

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))

Tile create_empty_tile() {
  Tile tile;
  tile.masked = true;
  tile.neighboring_mines = 0;
  return tile;
}

Tile *field;
uint16_t field_width, field_height;

static inline Tile *get_tile(uint16_t x, uint16_t y) {
  return &field[y * field_width + x];
}

/** Allocate a field of tiles. */
void alloc_field();
/** Free the allocated tiles. */
void free_field();
/** Put mines in the minefield. */
void populate_field(uint16_t mine_count, Tile *exception);
/** Print the minefield. */
void print_field(bool unmask);
/**
 * Ask for and parse coordinates of the tile that should be unmasked.
 */
bool handle_input(uint16_t *x, uint16_t *y);
/**
 * Unmask the given tile.
 * Return true if the unmasked tile is a mine.
 */
bool unmask(uint16_t x, uint16_t y);
/** Did player win? **/
bool player_won();

/** Start game. */
void start(uint16_t fw, uint16_t fh, uint16_t mine_count) {
  field_width = fw;
  field_height = fh;

  alloc_field();
  bool populate_pending = true;

  for (;;) {
    print_field(false);

    uint16_t x, y;
    if (!handle_input(&x, &y))
      break;
    else if (populate_pending) {
      populate_field(mine_count, get_tile(x, y));
      populate_pending = false;
    }

    if (unmask(x, y)) {
      printf("YOU LOST!\n");
      print_field(true);
      break;
    } else if (player_won()) {
      printf("YOU WON!\n");
      print_field(true);
      break;
    }
  }

  free_field();
}

void alloc_field() {
  field = (Tile*) malloc(field_width * field_height * sizeof(Tile));
  for (uint16_t i = 0; i < field_width * field_height; i++)
    field[i] = create_empty_tile();
}

void free_field() {
  free(field);
}

void populate_field(uint16_t mine_count, Tile *exception) {
  srand(time(NULL));

  for (uint16_t i = 0; i < mine_count; i++) {
    uint16_t x, y;
    Tile *tile;
    for (;;) {
      x = rand() % field_width;
      y = rand() % field_height;
      tile = get_tile(x, y);

      if (tile->neighboring_mines != -1 && tile != exception)
        break;
    }

    tile->neighboring_mines = -1;

    // AAAAAARGH
    for (uint16_t ny = MAX(y - 1, 0); ny < MIN(y + 2, field_height); ny++)
      for (uint16_t nx = MAX(x - 1, 0); nx < MIN(x + 2, field_width); nx++) {
        Tile *neighbor = get_tile(nx, ny);
        if (neighbor != tile && neighbor->neighboring_mines != -1)
          neighbor->neighboring_mines++;
      }
  }
}

void print_field(bool unmasked) {
  for (uint16_t y = 0; y < field_height; y++)
    for (uint16_t x = 0; x < field_width; x++) {
      Tile *tile = get_tile(x, y);
      char c;
      char *color;

      if (!unmasked && tile->masked) {
        c = MASKED_TILE_CHAR;
        color = COLOR_MASKED;
      }
      else if (tile->neighboring_mines == -1) {
        c = MINE_TILE_CHAR;
        color = COLOR_MINE;
      }
      else if (tile->neighboring_mines == 0) {
        c = EMPTY_TILE_CHAR;
        color = COLOR_EMPTY;
      }
      else {
        c = tile->neighboring_mines + '0';
        // i have no idea what i'm doing
        char *colors[8] = COLOR_NUMS;
        color = colors[tile->neighboring_mines];
      }

      printf(
        "%s%c%s%c",
        color,
        c,
        COLOR_RESET,
        // if this is the last x, move to the next line
        x == field_width - 1 ? '\n' : ' '
      );
    }
}

uint16_t digits_to_u16(uint8_t *buf, size_t size) {
  uint16_t res = 0;
  for (uint16_t i = 0; i < size; i++)
    res += buf[i] * pow(10, size - i - 1);
  return res;
}

uint8_t digit_buffer[100];
size_t digit_buffer_len;

bool parse_coord(uint16_t *x, uint16_t *y, bool *x_set, bool *y_set) {
  uint16_t num = digits_to_u16(digit_buffer, digit_buffer_len);
  digit_buffer_len = 0;

  if (!*x_set) {
    if (num == 0 || num > field_width) {
      printf(ERROR_PREFIX "x has to be between 1 and %d (got %d).\n", field_width, num);
      return false;
    }

    *x = num - 1;
    *x_set = true;
  } else if (!*y_set) {
    if (num == 0 || num > field_height) {
      printf(ERROR_PREFIX "y has to be between 1 and %d (got %d).\n", field_height, num);
      return false;
    }

    *y = num - 1;
    *y_set = true;

    if (!get_tile(*x, *y)->masked) {
      printf(ERROR_PREFIX "Tile (%d,%d) is already unmasked.\n", *x + 1, *y + 1);
      return false;
    }
  } else {
    printf(ERROR_PREFIX "Too many coordinates; x and y expected.\n");
    return false;
  }

  return true;
}

bool parse_coords(char *s, size_t len, uint16_t *x, uint16_t *y) {
  bool x_set = false, y_set = false;
  digit_buffer_len = 0;

  for (uint16_t i = 0; i < len; i++) {
    const char c = s[i];

    if (c >= '0' && c <= '9')
      // store the digit in `num_buffer`
      digit_buffer[digit_buffer_len++] = c - '0';
    else if (c == LOCATION_DELIMITER)
      if (!parse_coord(x, y, &x_set, &y_set))
        return false;
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

bool unmask(uint16_t x, uint16_t y) {
  Tile *tile = get_tile(x, y);
  tile->masked = false;

  if (tile->neighboring_mines == -1)
    return true;
  else if (tile->neighboring_mines == 0) {
    for (uint16_t ny = MAX(y - 1, 0); ny < MIN(y + 2, field_height); ny++)
      for (uint16_t nx = MAX(x - 1, 0); nx < MIN(x + 2, field_width); nx++) {
        Tile *neighbor = get_tile(nx, ny);
        if (neighbor != tile && neighbor->masked)
          unmask(nx, ny);
      }
  }

  return false;
}

bool player_won() {
  for (uint16_t y = 0; y < field_height; y++)
    for (uint16_t x = 0; x < field_width; x++) {
      Tile *tile = get_tile(x, y);
      if (tile->masked && tile->neighboring_mines != -1)
        return false;
    }
  return true;
}
