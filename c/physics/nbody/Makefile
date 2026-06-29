CC     = clang
CFLAGS = -Wall -Wextra -std=c11

BENCH  = bench_nbody

.PHONY: bench clean

bench: $(BENCH)
	./$(BENCH)

$(BENCH): bench.c nbody.c nbody.h
	$(CC) $(CFLAGS) -O2 bench.c nbody.c -o $(BENCH) -lm

clean:
	rm -f $(BENCH)
