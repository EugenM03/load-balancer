CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
HASHTABLE =hashtable
LIST =linked_list
.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(HASHTABLE).o $(LIST).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(LIST).o: $(LIST).c $(LIST).h
	$(CC) $(CFLAGS) $^ -c

$(HASHTABLE).o: $(HASHTABLE).c $(HASHTABLE).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch
