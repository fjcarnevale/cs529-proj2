default: speak.o network.o
	gcc -g speak.o network.o -lpthread -lasound -o speak

speak: speak.c
	gcc -g -c speak.c -lpthread -lasound

network: network.c
	gcc -g -c network.c 

clean:
	rm *.o speak
