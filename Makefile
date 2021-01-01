.PHONY := build

build:
	[[ -e bin ]] || mkdir bin
	gcc -o bin/minesweeper src/*.c -Iinclude -lm
