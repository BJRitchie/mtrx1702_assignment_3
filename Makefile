# Makefile

CC = gcc
CFLAGS= -g -Wall -Wextra -std=c99 -Iinclude
LIBS = -lm
TARGET = rover

SRC_DIR = src
OBJ_DIR = build
INC_DIR = include

# you may need to put your extra files here
_DEPS = bitmap.h functions.h
_OBJS = rover.o bitmap.o functions.o
DEPS = $(patsubst %,$(INC_DIR)/%,$(_DEPS))
OBJS = $(patsubst %,$(OBJ_DIR)/%,$(_OBJS))


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) 
