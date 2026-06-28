CC      = clang
CFLAGS  = -Wall -Wextra -std=c11
UNITY   = ../../../../c-examples/lib/unity

SDL_INC = -I/opt/homebrew/Cellar/sdl2-compat/2.32.70/include
SDL_LIB = -L/opt/homebrew/Cellar/sdl2-compat/2.32.70/lib -lSDL2 -Wl,-framework,Cocoa

TARGET = bounce
BENCH  = bench_bounce
TEST   = test/test_physics

.PHONY: all test bench clean

all: $(TARGET)

$(TARGET): main.c physics.c physics.h
	$(CC) $(CFLAGS) -g $(SDL_INC) main.c physics.c -o $(TARGET) $(SDL_LIB) -lm

test: $(TEST)
	./$(TEST)

$(TEST): test/test_physics.c physics.c physics.h $(UNITY)/unity.c
	$(CC) $(CFLAGS) -g -I$(UNITY) test/test_physics.c physics.c \
	      $(UNITY)/unity.c -o $(TEST) -lm

bench: $(BENCH)

$(BENCH): bench.c physics.c physics.h
	$(CC) $(CFLAGS) -O2 bench.c physics.c -o $(BENCH) -lm

clean:
	rm -f $(TARGET) $(BENCH) $(TEST)
