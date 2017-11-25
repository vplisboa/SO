make: shell.o
	@gcc -w -o shell shell.o

shell.o: shell.c
	@gcc -w -c shell.c

clean:
	@rm  *.o
