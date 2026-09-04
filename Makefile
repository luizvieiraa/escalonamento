CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
CPPFLAGS := -Isrc

TARGET := scheduler
SOURCES := src/main.c src/parser.c src/scheduler.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
