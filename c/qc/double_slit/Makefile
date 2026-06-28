CC      = clang
CFLAGS  = -Wall -Wextra -std=c11 -g
UNITY   = ../../lib/unity

SDL_INC = -I/opt/homebrew/Cellar/sdl2-compat/2.32.70/include
SDL_LIB = -L/opt/homebrew/Cellar/sdl2-compat/2.32.70/lib -lSDL2 -Wl,-framework,Cocoa

TARGET  = double_slit
TEST    = test/test_double_slit

.PHONY: all test clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $(SDL_INC) main.c -o $(TARGET) $(SDL_LIB) -lm

test: $(TEST)
	./$(TEST)

$(TEST): test/test_double_slit.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) -I$(UNITY) test/test_double_slit.c $(UNITY)/unity.c -o $(TEST) -lm

clean:
	rm -f $(TARGET) $(TEST)
