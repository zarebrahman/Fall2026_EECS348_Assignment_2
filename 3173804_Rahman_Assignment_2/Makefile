CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
TARGET = assignment2

all: $(TARGET)

$(TARGET): assignment2.c
	$(CC) $(CFLAGS) assignment2.c -o $(TARGET)

test: $(TARGET)
	./$(TARGET) sample_test.txt

clean:
	rm -f $(TARGET)

.PHONY: all test clean
