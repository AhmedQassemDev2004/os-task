CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)

SRCS = main.c
OBJS = $(patsubst %.c,build/%.o,$(SRCS))
EXEC = build/file_manager

all: $(EXEC)

$(EXEC): $(OBJS) | build
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

build/%.o: %.c | build
	$(CC) -c $(CFLAGS) $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build
