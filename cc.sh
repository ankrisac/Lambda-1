printf '
SRC_DIR := src
SRC := $(shell find $(SRC_DIR) -name "*.c")
HDR := $(shell find $(SRC_DIR) -name "*.h")

FLAGS := -O3 -Werror 

PHONY: lin
lin: $(SRC) $(HDR)
\tgcc -Wall $(FLAGS) $(SRC) -o .bin/main
' > Makefile

clear
case $1 in
    debug)
        make lin && valgrind ./.bin/main
    ;;
    build)
        make lin && ./.bin/main
    ;;
    run)
        ./.bin/main
    ;;
esac