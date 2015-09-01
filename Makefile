CC=gcc
OBJECTS=$(patsubst %.c,%.o,$(wildcard *.c))
OUT=butts
CFLAGS=-Wall -Wextra -Llib -llinebuf

all: linebuf $(OUT)

$(OUT): $(OBJECTS) lib/liblinebuf.a
	$(CC) -o $(OUT) $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

lib/liblinebuf.a:
	make -C linebuf
	cp linebuf/liblinebuf.a ./lib/

clean:
	rm $(OUT)
	rm *.o
	rm lib/*.a