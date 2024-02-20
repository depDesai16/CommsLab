PORT?=8888
SERVER?=localhost

.PHONY: all clean test 

all: pipes server client threads

pipes: pingpipe.c
	gcc -o pipes pingpipe.c

server: pingser.o
	gcc -o server pingser.o 

pingser.o: pingser.c 
	gcc -c pingser.c

client: pingcli.o
	gcc -o client pingcli.o

pingcli.o: pingcli.c
	gcc -c pingcli.c

threads: pingpc.c saferq.c saferq.h
	gcc -o threads pingpc.c saferq.c -lpthread

test: pipes server client threads
	@echo "Testing ping-pong pipes..."
	@echo ""
	./pipes
	@echo "Testing ping-pong client-server..."
	@echo ""
	./server $(PORT) &
	./client $(SERVER) $(PORT) 
	@echo "Testing ping-pong threaded..."
	@echo ""
	./threads


clean:
	rm *.o server client pipes threads
