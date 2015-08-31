CC=gcc
OBJECTS=$(patsubst %.c,%.o,$(wildcard *.c))
OUT=butts
CFLAGS=-Wall -Wextra

all: $(OUT)

$(OUT): $(OBJECTS)
	$(CC) -o $(OUT) $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)