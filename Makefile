CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
CPPFLAGS := -Isrc

ALVO := scheduler
FONTES := src/main.c src/parser.c src/scheduler.c src/output.c

.PHONY: all clean test

all: $(ALVO)

$(ALVO): $(FONTES)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FONTES) -o $(ALVO)

test: $(ALVO)
	sh tests/run_tests.sh

clean:
	rm -f $(ALVO) scheduler.exe rate_lhcv.out edf_lhcv.out
