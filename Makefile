CC = gcc
CFLAGS ?= -Wall -Wextra -Werror -pedantic -std=c99 -Iinclude -Wno-format

TARGET = ls

SRCS = src/main.c \
       src/options.c \
       src/file_entry.c \
       src/sort.c \
       src/display.c \
       src/traverse.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-cmd /c del /f /q src\*.o $(TARGET) $(TARGET).exe 2>nul
else
	rm -f $(OBJS) $(TARGET) $(TARGET).exe
endif

test: $(TARGET)
ifeq ($(OS),Windows_NT)
	@sh test_suite.sh || bash test_suite.sh
else
	@chmod +x test_suite.sh
	@./test_suite.sh
endif

.PHONY: all clean test
