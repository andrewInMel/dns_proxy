phase1: phase1.c
	gcc -Wall -o phase1 phase1.c

clean:
	rm -rf phase1 dns_svr *.log *.o
