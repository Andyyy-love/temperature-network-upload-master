CC = gcc
TARGET = testserver

SRCS = socket-poll.c socket.c packet.c database.c cJSON.c
OBJS = $(SRCS:.c=.o)

CFLAGS = -Wall -I./ -I../install/include
LDFLAGS = -L../install/lib
LIBS = -lsqlite3

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "server generated: ./$(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) *.db

.PHONY: all run clean
