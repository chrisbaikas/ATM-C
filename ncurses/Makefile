CC = cc
CFLAGS = -Wall -Wextra -std=c99 -pthread -D_XOPEN_SOURCE=500

OBJECTS = src/main.o src/system.o src/auth.o src/shared_notification.o src/notification.o src/database.o src/helpers.o

atm: $(OBJECTS)
	$(CC) $(CFLAGS) -o atm $(OBJECTS) -lcrypto -lsqlite3 -lncurses

src/%.o: src/%.c src/header.h src/shared_notification.h src/notification.h src/database.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) atm

