CC      = clang
CFLAGS  = -Wall -Wextra -std=c11 -g
UNITY   = ../../lib/unity

SDL_INC  = -I/opt/homebrew/Cellar/sdl2-compat/2.32.70/include -I/opt/homebrew/include
SDL_LIB  = -L/opt/homebrew/Cellar/sdl2-compat/2.32.70/lib -lSDL2 -Wl,-framework,Cocoa
TTF_INC  = -I/opt/homebrew/Cellar/sdl2_ttf/2.24.0/include
TTF_LIB  = -L/opt/homebrew/Cellar/sdl2_ttf/2.24.0/lib -lSDL2_ttf
FFTW_INC = -I/opt/homebrew/Cellar/fftw/3.3.11/include
FFTW_LIB = -L/opt/homebrew/Cellar/fftw/3.3.11/lib -lfftw3f

TARGET  = wave_packet
BENCH   = bench_wave_packet
TEST    = test/test_wave_packet

.PHONY: all bench test clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $(SDL_INC) $(TTF_INC) $(FFTW_INC) main.c -o $(TARGET) $(SDL_LIB) $(TTF_LIB) $(FFTW_LIB) -lm

$(BENCH): bench.c
	$(CC) -O2 -std=c11 $(FFTW_INC) bench.c -o $(BENCH) $(FFTW_LIB) -lm

bench: $(BENCH)
	./$(BENCH)

test: $(TEST)
	./$(TEST)

$(TEST): test/test_wave_packet.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) -I$(UNITY) test/test_wave_packet.c $(UNITY)/unity.c -o $(TEST) -lm

clean:
	rm -f $(TARGET) $(BENCH) $(TEST)
