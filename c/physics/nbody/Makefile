CC      = clang
CFLAGS  = -Wall -Wextra -std=c11

SDL_INC = -I/opt/homebrew/Cellar/sdl2-compat/2.32.70/include -I/opt/homebrew/include
SDL_LIB = -L/opt/homebrew/Cellar/sdl2-compat/2.32.70/lib -lSDL2 -Wl,-framework,Cocoa
TTF_INC = -I/opt/homebrew/Cellar/sdl2_ttf/2.24.0/include
TTF_LIB = -L/opt/homebrew/Cellar/sdl2_ttf/2.24.0/lib -lSDL2_ttf

TARGET = nbody
BENCH  = bench_nbody

.PHONY: all bench clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -O2 $(SDL_INC) $(TTF_INC) main.c -o $(TARGET) $(SDL_LIB) $(TTF_LIB) -lm

bench: $(BENCH)
	./$(BENCH)

$(BENCH): bench.c nbody.c nbody.h
	$(CC) $(CFLAGS) -O2 bench.c nbody.c -o $(BENCH) -lm

clean:
	rm -f $(TARGET) $(BENCH)
