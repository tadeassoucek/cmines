// This is dumb.

#ifdef COLOR_DISABLED

#define COLOR_ERROR ""
#define COLOR_MINE ""
#define COLOR_MASKED ""
#define COLOR_EMPTY ""
char *MINE_PROX_COLORS[] = {
  "", "",
  "", "",
  "", "",
  "", ""
};
#define COLOR_RESET ""

#else

#define COLOR_ERROR "\033[31m"
#define COLOR_MINE "\033[7;31m"
#define COLOR_MASKED ""
#define COLOR_EMPTY "\033[90m"
char *MINE_PROX_COLORS[] = {
  "\033[33m", "\033[93m",
  "\033[32m", "\033[92m",
  "\033[35m", "\033[95m",
  "\033[34m", "\033[94m"
};
#define COLOR_RESET "\033[0m"

#endif
