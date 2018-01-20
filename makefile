CC = gcc
CFLAGS = -Wall
LIBS = -lm

CONSOLE_EXEC = jms_console
CONSOLE_OBJECTS = console_main.o console_functions.o
CONSOLE_SOURCES = console_main.c console_functions.c
CONSOLE_HEADERS = header.h console_functions.h

COORD_EXEC = jms_coord
COORD_OBJECTS = coord_main.o coord_functions.o coord_commands.o
COORD_SOURCES = coord_main.c coord_functions.c coord_commands.c
COORD_HEADERS = coord_header.h coord_functions.h coord_commands.h

POOL_EXEC = pool
POOL_OBJECTS = pool_main.o pool_functions.o pool_commands.o
POOL_SOURCES = pool_main.c pool_functions.c pool_commands.c
POOL_HEADERS = pool_header.h pool_functions.h pool_commands.h

ALL_EXEC = $(CONSOLE_EXEC) $(COORD_EXEC) $(POOL_EXEC)
ALL_OBJECTS = $(CONSOLE_OBJECTS) $(COORD_OBJECTS) $(POOL_OBJECTS)
ALL_SOURCES = $(CONSOLE_SOURCES) $(COORD_SOURCES) $(POOL_SOURCES)
ALL_HEADERS = $(CONSOLE_HEADERS) $(COORD_HEADERS) $(POOL_HEADERS)

executables: $(CONSOLE_OBJECTS) $(COORD_OBJECTS) $(POOL_OBJECTS)
	$(CC) $(CFLAGS) -o $(CONSOLE_EXEC) $(CONSOLE_OBJECTS)
	$(CC) $(CFLAGS) -o $(COORD_EXEC) $(COORD_OBJECTS) $(LIBS)
	$(CC) $(CFLAGS) -o $(POOL_EXEC) $(POOL_OBJECTS)

console_main.o: console_main.c
	$(CC) $(CFLAGS) -c console_main.c

console_functions.o: console_functions.c
	$(CC) $(CFLAGS) -c console_functions.c

coord_main.o: coord_main.c
	$(CC) $(CFLAGS) -c coord_main.c

coord_functions.o: coord_functions.c
	$(CC) $(CFLAGS) -c coord_functions.c

coord_commands.o: coord_commands.c
	$(CC) $(CFLAGS) -c coord_commands.c

pool_main.o: pool_main.c
	$(CC) $(CFLAGS) -c pool_main.c

pool_functions.o: pool_functions.c
	$(CC) $(CFLAGS) -c pool_functions.c

pool_commands.o: pool_commands.c
	$(CC) $(CFLAGS) -c pool_commands.c

.PHONY: clean

clean:
	rm -f $(ALL_EXEC) $(ALL_OBJECTS)

count:
	wc $(ALL_SOURCES) $(ALL_HEADERS)