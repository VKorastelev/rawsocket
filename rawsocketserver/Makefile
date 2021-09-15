TARGET := rawsocketserver
srcfiles = $(shell find ./ -maxdepth 1 -name "*.c")
objfiles = $(srcfiles:.c=.o)
CC := gcc
CFLAGS = -Wall#-pthread
LDFLAGS = #-ldl# -lpthread

#$(info VAR="$(objfiles)")

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(objfiles)
		$(CC) $^ -o $@ $(LDFLAGS)

$(objfiles): $(srcfiles)
		$(CC) $(CFLAGS) $^ -c

clean:
		rm -rf $(TARGET) *.o
