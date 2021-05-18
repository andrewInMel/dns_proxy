all: dns_svr

struct.o: struct.c
	gcc -Wall -c struct.c

phase1.o: phase1.c
	gcc -Wall -c phase1.c

dns_svr: dns_svr.c phase1.o struct.o
	gcc -Wall -o dns_svr dns_svr.c phase1.o struct.o

clean:
	rm -rf dns_svr *.log *.o
