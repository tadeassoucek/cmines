.PHONY := build

build:
	[[ -e bin ]] || mkdir bin
	gcc -o bin/cmines src/*.c -Iinclude -lm -Wall -pedantic
