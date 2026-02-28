
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

TARGET = area
OBJS = app.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

app.o: app.c
	$(CC) $(CFLAGS) -c app.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean