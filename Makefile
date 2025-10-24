CC = gcc
# CFLAGS = -Wall -Wextra -g

TARGET = mmap_emu_split

SRCS = main.c mmap_libc.c
OBJS = $(SRCS:.c=.o)


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

main.o: main.c mmap_libc.h
	$(CC) $(CFLAGS) -c main.c

mmap_libc.o: mmap_libc.c mmap_libc.h
	$(CC) $(CFLAGS) -c mmap_libc.c


run: $(TARGET)
	@echo "--- Running $(TARGET) ---"
	./$(TARGET)

clean:
	@echo "Cleaning up build artifacts..."
	rm -f $(OBJS) $(TARGET) simulated_data.tmp

.PHONY: all run clean