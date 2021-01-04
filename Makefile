p2p: p2p.c List.c
	gcc -o p2p p2p.c functions.c List.c -lpthread
