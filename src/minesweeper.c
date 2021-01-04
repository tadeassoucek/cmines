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

#define ERROR_PREFIX COLOR_ERROR "Error:" COLOR_RESET " "

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))

// for ever => for (;;)
#define ever (;;)

Tile create_empty_tile() {
  Tile tile;
  tile.masked = true;
  tile.neighboring_mines = 0;
  return tile;
}

Tile *field;
uint8_t field_width, field_height;

static inline Tile *get_tile(uint8_t x, uint8_t y) {
  return &field[y * field_width + x];
}

/** Allocate a field of tiles. */
void alloc_field();
/** Free the allocated tiles. */
void free_field();
/** Put mines in the minefield. */
void populate_field(uint8_t mine_count, uint8_t exc_x, uint8_t exc_y);
/** Print the minefield. */
void print_field(bool unmask);
/**
 * Ask for and parse coordinates of the tile that should be unmasked.
 */
bool handle_input(uint8_t *x, uint8_t *y);
/**
 * Unmask the given tile.
 * Return true if the unmasked tile is a mine.
 */
bool unmask(uint8_t x, uint8_t y);
/** Did player win? **/
bool player_won();

/** Start game. */
void start(uint8_t fw, uint8_t fh, uint8_t mine_count) {
  field_width = fw;
  field_height = fh;

  alloc_field();
  bool populate_pending = true;

  for ever {
    print_field(false);

    uint8_t x, y;
    if (!handle_input(&x, &y))
      break;
    else if (populate_pending) {
      populate_field(mine_count, x, y);
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
  for (uint8_t i = 0; i < field_width * field_height; i++)
    field[i] = create_empty_tile();
}

void free_field() {
  free(field);
}

void populate_field(uint8_t mine_count, uint8_t exc_x, uint8_t exc_y) {
  srand(time(NULL));

  for (uint8_t i = 0; i < mine_count; i++) {
    uint8_t x, y;
    Tile *tile;

    for ever {
      x = rand() % field_width;
      y = rand() % field_height;

      tile = get_tile(x, y);

      if (tile->neighboring_mines != -1 &&
          !((exc_x >= x - 1 && exc_x <= x + 1) ||
            (exc_y >= y - 1 && exc_y <= y + 1)))
        break;
    }

    tile->neighboring_mines = -1;

    // AAAAAARGH
    for (uint8_t ny = MAX(y - 1, 0); ny < MIN(y + 2, field_height); ny++)
      for (uint8_t nx = MAX(x - 1, 0); nx < MIN(x + 2, field_width); nx++) {
        Tile *neighbor = get_tile(nx, ny);
        if (neighbor != tile && neighbor->neighboring_mines != -1)
          neighbor->neighboring_mines++;
      }
  }
}

void print_field(bool unmasked) {
  printf("   ");
  for (uint8_t x = 0; x < field_width; x++)
    printf("%c ", x + 'A');
  printf("\n");

  for (uint8_t y = 0; y < field_height; y++) {
    printf("%2d ", y + 1);

    for (uint8_t x = 0; x < field_width; x++) {
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
        color = MINE_PROX_COLORS[tile->neighboring_mines];
      }

      printf(
        "%s%c%s ",
        color,
        c,
        COLOR_RESET
      );
    }

    printf("%-2d\n", y + 1);
  }

  printf("   ");
  for (uint8_t x = 0; x < field_width; x++)
    printf("%c ", x + 'A');
  printf("\n");
}

uint8_t digits_to_u16(uint8_t *buf, size_t size) {
  uint8_t res = 0;
  for (uint8_t i = 0; i < size; i++)
    res += buf[i] * pow(10, size - i - 1);
  return res;
}

bool parse_coords(char *s, size_t len, uint8_t *x, uint8_t *y) {
  bool expect_x = true;
  uint8_t digit_buf[100];
  size_t digit_buf_len = 0;

  for (uint8_t i = 0; i < len; i++) {
    char c = s[i];

    switch (c) {
      case ' ':
      case '\t':
      case '\n':
      case '\b':
      case '\v':
      case '\f':
      case '\0':
        continue;
    }

    if (expect_x) {
      if (c >= 'A' && c <= 'Z')
        c += 'a' - 'A';

      if (c >= 'a' && c <= 'z') {
        *x = c - 'a';
        expect_x = false;
      }
    }
    else {
      if (c >= '0' && c <= '9')
        digit_buf[digit_buf_len++] = c - '0';
      else {
        printf(ERROR_PREFIX "Unexpected character '%c' at pos %d\n", c, i + 1);
        return false;
      }
    }
  }

  *y = digits_to_u16(digit_buf, digit_buf_len) - 1;

  return true;
}

bool handle_input(uint8_t *x, uint8_t *y) {
  for ever {
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

bool unmask(uint8_t x, uint8_t y) {
  Tile *tile = get_tile(x, y);
  tile->masked = false;

  if (tile->neighboring_mines == -1)
    return true;
  else if (tile->neighboring_mines == 0) {
    for (uint8_t ny = MAX(y - 1, 0); ny < MIN(y + 2, field_height); ny++)
      for (uint8_t nx = MAX(x - 1, 0); nx < MIN(x + 2, field_width); nx++) {
        Tile *neighbor = get_tile(nx, ny);
        if (neighbor != tile && neighbor->masked)
          unmask(nx, ny);
      }
  }

  return false;
}

bool player_won() {
  for (uint8_t y = 0; y < field_height; y++)
    for (uint8_t x = 0; x < field_width; x++) {
      Tile *tile = get_tile(x, y);
      if (tile->masked && tile->neighboring_mines != -1)
        return false;
    }
  return true;
}
