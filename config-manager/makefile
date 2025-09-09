# Makefile for cross-platform GTK3 + JSON-C project

CC = gcc
CFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0 json-c`
LIBS   = `pkg-config --libs gtk+-3.0 json-c`
TARGET = config_manager

SRCS = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
