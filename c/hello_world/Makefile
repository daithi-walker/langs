CC      = clang
CFLAGS  = -Wall -Wextra -std=c11 -g
UNITY   = ../lib/unity

TARGET  = hello
TEST    = test/test_hello

.PHONY: all test clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

test: $(TEST)
	./$(TEST)

$(TEST): test/test_hello.c main.c $(UNITY)/unity.c
	$(CC) $(CFLAGS) -I$(UNITY) test/test_hello.c $(UNITY)/unity.c -o $(TEST)

clean:
	rm -f $(TARGET) $(TEST)
