CC      = clang
CFLAGS  = -Wall -Wextra -std=c11 -g
UNITY   = ../../lib/unity

SDL_INC = -I/opt/homebrew/Cellar/sdl2-compat/2.32.70/include -I/opt/homebrew/include
SDL_LIB = -L/opt/homebrew/Cellar/sdl2-compat/2.32.70/lib -lSDL2 -Wl,-framework,Cocoa
TTF_INC = -I/opt/homebrew/Cellar/sdl2_ttf/2.24.0/include
TTF_LIB = -L/opt/homebrew/Cellar/sdl2_ttf/2.24.0/lib -lSDL2_ttf
GSL_INC = -I/opt/homebrew/Cellar/gsl/2.8/include
GSL_LIB = -L/opt/homebrew/Cellar/gsl/2.8/lib -lgsl -lgslcblas

TARGET  = orbitals
TEST    = test/test_orbitals

.PHONY: all test clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $(SDL_INC) $(TTF_INC) $(GSL_INC) main.c -o $(TARGET) $(SDL_LIB) $(TTF_LIB) $(GSL_LIB) -lm

test: $(TEST)
	./$(TEST)

$(TEST): test/test_orbitals.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) -I$(UNITY) test/test_orbitals.c $(UNITY)/unity.c -o $(TEST) -lm

clean:
	rm -f $(TARGET) $(TEST)
