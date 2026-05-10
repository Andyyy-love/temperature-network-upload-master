SHELL := /bin/bash

CC = gcc
TARGET = testclient
INST_PATH = ./INSTALL
STATIC_LIB = $(INST_PATH)/libs/libmyfunction.a

SRCS = packet.c socket.c gettime.c database.c ds18b20.c logger.c
OBJS = $(patsubst %.c,$(INST_PATH)/bin/%.o,$(SRCS))

CFLAGS = -Wall -I$(INST_PATH)/include/
LDFLAGS = -L$(INST_PATH)/libs
LIBS = -lmyfunction -lsqlite3

all: prepare $(STATIC_LIB) $(TARGET)

prepare:
	mkdir -p $(INST_PATH)/bin $(INST_PATH)/etc $(INST_PATH)/include $(INST_PATH)/libs
	cp -f *.h $(INST_PATH)/include/ 2>/dev/null || true
	cp -f ../install/include/sqlite3.h $(INST_PATH)/include/ 2>/dev/null || true
	cp -f ../install/lib/lib* $(INST_PATH)/libs/ 2>/dev/null || true

$(STATIC_LIB): $(OBJS)
	ar -rcs $@ $(OBJS)
	@echo "static library generated: $@"

$(TARGET): $(STATIC_LIB)
	$(CC) $(CFLAGS) main.c -o $(INST_PATH)/bin/$(TARGET) $(LDFLAGS) $(LIBS)
	@echo "client generated: $(INST_PATH)/bin/$(TARGET)"

$(INST_PATH)/bin/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(INST_PATH)
	rm -f *.o *.db

.PHONY: all prepare clean
