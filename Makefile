CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -Wpedantic -O2
TARGET  := tracker
SRC_DIR := src
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:.c=.o)

.PHONY: all clean debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build with debug symbols and AddressSanitizer (detects leaks/overflows)
debug: CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined
debug: clean $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) tracker_data.txt
