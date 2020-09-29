all:
	gcc -c main.c -o main.o
	gcc -c cJSON.c -o cJSON.o
	gcc -o weatherget main.o cJSON.o -lm
	rm *.o
clean:
	rm weatherget
