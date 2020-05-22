
SRC_DIR := src
SRC := $(shell find $(SRC_DIR) -name "*.c")
HDR := $(shell find $(SRC_DIR) -name "*.h")

FLAGS := -Wall -Werror -pedantic

PHONY: lin
lin: $(SRC) $(HDR)
	gcc $(FLAGS) $(SRC) -o .bin/main

PNONY: lin_fast
lin_fast: $(SRC) $(HDR)
	gcc -Ofast $(FLAGS) $(SRC) -o .bin/main 

PHONY: debug
debug: $(SRC) $(HDR)
	gcc -g $(FLAGS) $(SRC) -o .bin/main
