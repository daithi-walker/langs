CC     = clang
CFLAGS = -Wall -Wextra -std=c11 -O2
BENCH  = bench_matmul

.PHONY: bench clean

bench: $(BENCH)
	./$(BENCH)

$(BENCH): bench.c matmul.c matmul.h
	$(CC) $(CFLAGS) bench.c matmul.c -o $(BENCH) -lm

clean:
	rm -f $(BENCH)
