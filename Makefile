
SRC_DIR := src
SRC := $(shell find $(SRC_DIR) -name "*.c")
HDR := $(shell find $(SRC_DIR) -name "*.h")

FLAGS := -O3 -Werror 

PHONY: lin
lin: $(SRC) $(HDR)
	gcc -Wall $(FLAGS) $(SRC) -o .bin/main

PHONY: debug
debug: $(SRC) $(HDR)
	gcc -g -Wall $(FLAGS) $(SRC) -o .bin/main
