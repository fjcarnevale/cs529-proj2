#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "network.h"

void* listen_thread(int* n);

int main(int argc, char** argv){

	// init network
	network_init(argc, argv);

	// start listening thread
	pthread_t listener;
	int n = 1; // substitute for actual arguments
	pthread_create(&listener, NULL, (void*)listen_thread, &n);

	pthread_join(listener,NULL);
}

void* listen_thread(int* n){
	printf("n = %d\n",*n);
}
