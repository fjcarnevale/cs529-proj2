default: main.o network.o
	gcc main.o network.o -lpthread -o speak

speak:
	gcc -c main.c -lpthread

network:
	gcc -c network.c 

clean:
	rm *.o speak network
