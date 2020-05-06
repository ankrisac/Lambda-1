printf '
SRC_DIR := src
SRC := $(shell find $(SRC_DIR) -name "*.c")
HDR := $(shell find $(SRC_DIR) -name "*.h")

FLAGS := -O3 -Werror 

PHONY: lin
lin: $(SRC) $(HDR)
\tgcc -Wall $(FLAGS) $(SRC) -o .bin/main

PHONY: debug
debug: $(SRC) $(HDR)
\tgcc -g -Wall $(FLAGS) $(SRC) -o .bin/main
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
esac