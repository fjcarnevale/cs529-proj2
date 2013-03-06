default: speak.o network.o
	gcc speak.o network.o -lpthread -o speak

speak:
	gcc -c speak.c -lpthread

network:
	gcc -c network.c 

clean:
	rm *.o speak network
