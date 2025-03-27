CC = cc
CPPFLAGS = $(shell pkg-config --cflags gtk+-3.0)
CFLAGS = -Wall -Wextra -std=c99 -pthread -D_XOPEN_SOURCE=500
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)

OBJECTS = src/main.o src/system.o src/auth.o src/shared_notification.o src/notification.o src/database.o src/helpers.o

atm: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o atm $(OBJECTS) -lcrypto -lsqlite3 $(LDFLAGS)

src/%.o: src/%.c src/header.h src/shared_notification.h src/notification.h src/database.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) atm
