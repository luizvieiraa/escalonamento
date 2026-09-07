CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
CPPFLAGS := -Isrc

ALVO := scheduler
FONTES := src/main.c src/parser.c src/scheduler.c

.PHONY: all clean

all: $(ALVO)

$(ALVO): $(FONTES)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FONTES) -o $(ALVO)

clean:
	rm -f $(ALVO)
