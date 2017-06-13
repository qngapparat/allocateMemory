all:
	gcc membench.c -o membench.out -pthread
	gcc malloc.c -o malloc.out
