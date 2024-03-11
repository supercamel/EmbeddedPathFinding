CC = gcc
CFLAGS = -g -Wall -I./ `pkg-config --cflags gtk4`
LIBS = -lm `pkg-config --libs --cflags glib-2.0 gtk4`
TARGET = main
SOURCES = main.c polygon.c visibility_graph.c $(wildcard Cts/*.c)
OBJS = $(SOURCES:.c=.o) 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
