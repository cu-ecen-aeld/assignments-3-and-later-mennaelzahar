# Compiler settings
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror -g

# Targets and sources
TARGET := writer
SRC := finder-app/writer.c
OBJ := writer.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: finder-app/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
