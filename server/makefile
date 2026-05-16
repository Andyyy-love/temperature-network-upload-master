CC = gcc
TARGET_POLL = testserver
TARGET_EPOLL = testserver-epoll

SRCS_COMMON = socket.c packet.c database.c cJSON.c
SRCS_POLL = socket-poll.c $(SRCS_COMMON)
SRCS_EPOLL = socket-epoll.c $(SRCS_COMMON)
OBJS_POLL = $(SRCS_POLL:.c=.o)
OBJS_EPOLL = $(SRCS_EPOLL:.c=.o)

CFLAGS = -Wall -I./ -I../install/include
LDFLAGS = -L../install/lib
LIBS = -lsqlite3

all: $(TARGET_POLL) $(TARGET_EPOLL)

$(TARGET_POLL): $(OBJS_POLL)
	$(CC) $(OBJS_POLL) -o $(TARGET_POLL) $(LDFLAGS) $(LIBS)
	@echo "poll server generated: ./$(TARGET_POLL)"

$(TARGET_EPOLL): $(OBJS_EPOLL)
	$(CC) $(OBJS_EPOLL) -o $(TARGET_EPOLL) $(LDFLAGS) $(LIBS)
	@echo "epoll server generated: ./$(TARGET_EPOLL)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET_POLL)
	./$(TARGET_POLL)

run-epoll: $(TARGET_EPOLL)
	./$(TARGET_EPOLL)

clean:
	rm -f $(TARGET_POLL) $(TARGET_EPOLL) socket-poll.o socket-epoll.o socket.o packet.o database.o cJSON.o *.db

.PHONY: all run run-epoll clean
