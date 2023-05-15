CC=gcc
CFLAGS= -std=gnu99 -Wall -Wextra -pedantic -pthread
BIN=logging-daemon
SOURCE=logging-daemon.c
PARAMS=/tmp/output1.log /tmp/output2.log

all:
	cd src && $(CC) $(CFLAGS) -o $(BIN) $(SOURCE)

clean:
	cd src && rm $(BIN)

run:
	cd src && sudo ./$(BIN) $(PARAMS)
