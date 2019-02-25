SRCS := dummy_app.c
BINS := $(SRCS:.c=.bin)
# $(SRCS:.c=.bin) means, take the variable value ${SRCS}, which is a string composed of words
# separated by spaces, and for each word, replace the suffix .c with .bin => %.bin: target will
# run

all: ${BINS}

show: dummy_app_disasm

dummy_app.elf: start.o vector.o crt0.o board.o

include rules.mk
