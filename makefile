default: speak.o network.o
	gcc speak.o network.o -lpthread -lasound -o speak

speak:
	gcc -c speak.c -lpthread -lasound

network:
	gcc -c network.c 

clean:
	rm *.o speak network
