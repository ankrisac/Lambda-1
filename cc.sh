printf '
SRC_DIR := src
SRC := $(shell find $(SRC_DIR) -name "*.c")
HDR := $(shell find $(SRC_DIR) -name "*.h")

FLAGS := -Wall -Werror -pedantic

PHONY: lin
lin: $(SRC) $(HDR)
\tgcc $(FLAGS) $(SRC) -o .bin/main

PNONY: lin_fast
lin_fast: $(SRC) $(HDR)
\tgcc -Ofast $(FLAGS) $(SRC) -o .bin/main 

PHONY: debug
debug: $(SRC) $(HDR)
\tgcc -g $(FLAGS) $(SRC) -o .bin/main
' > Makefile

clear
case $1 in
    mem)
        make lin && valgrind --leak-check=full --track-origins=yes ./.bin/main
    ;;
    debug)
        make debug && gdb ./.bin/main
    ;;
    build)
        make lin && ./.bin/main
    ;;
    run)
        ./.bin/main
    ;;
    fast)
        make lin_fast && ./.bin/main
    ;;
esac