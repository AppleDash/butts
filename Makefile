CC=gcc
OBJECTS=$(patsubst %.c,%.o,$(wildcard *.c))
OUT=butts
CFLAGS=-Wall -Wextra -Llib -llinebuf

all: $(OUT)

$(OUT): $(OBJECTS)
	make liblinebuf
	$(CC) -o $(OUT) $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

liblinebuf:
	make -C linebuf
	cp linebuf/liblinebuf.a ./lib/

clean:
	rm $(OUT)
	rm *.o
	rm lib/*.a
