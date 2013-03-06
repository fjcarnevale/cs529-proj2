#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef enum{
	TCP_CLIENT,TCP_SERVER,UDP
}NETWORK_TYPE;

NETWORK_TYPE current_type;

int sock,server_sock;
struct sockaddr_in server_addr,client_addr;

void setup_server(int port){
	if((server_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0){
		printf("Failed to open socket\n");
		exit(1);
	}

	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
     		printf("Failed to bind to local address\n");
     		exit(1);
   	}

	listen(server_sock,5);

	printf("Waiting for connection...\n");

	int client_len = sizeof(client_addr);
	sock = accept(server_sock,(struct sockaddr*)&client_addr,&client_len);
	if(sock < 0){
		printf("Failed to accept connection\n");
		exit(1);
	}

	printf("Connected\n");
}

void connect_to_server(char* ip, int port){
	if((sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0){
		printf("Failed to open socket\n");
		exit(1);
	}

	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	if(connect(sock,(struct sockaddr*)&server_addr, sizeof(server_addr))<0){
		printf("Failed to connect to server\n");
		exit(1);
	}

	printf("Connected\n");
}

void setup_udp(char* ip, int port){

}


int setup_network(NETWORK_TYPE type, char* ip, int port){
	current_type = type;

	if(current_type == TCP_CLIENT){
		connect_to_server(ip, port);
	}else if(current_type == TCP_SERVER){
		setup_server(port);
	}else{
		setup_udp(ip, port);
	}
}


int initialize_network(int argc, char** argv){
	char* ip = NULL;
	int port = -1;

	char c;
	extern int optind, opterr;
	extern char* optarg;
	NETWORK_TYPE net_type = -1;

	while (( c = getopt( argc, argv, "m:a:p:" )) != EOF) {
		switch ( c ) {
			case 'm':
				if(!strcmp(optarg,"tcpclient"))
					net_type = TCP_CLIENT;
				else if(!strcmp(optarg,"tcpserver"))
					net_type = TCP_SERVER;
				else if(!strcmp(optarg,"udp"))
					net_type = UDP;
				else
					net_type = -1;
				break;
			case 'a':
				if(optarg != NULL)
					ip = optarg;
				break;
			case 'p':
				if(optarg != NULL)
					port = 	atoi(optarg);
				break;		

		}
	}
	
	if(net_type == -1 || ip == NULL || port == -1){
		printf("incorrect arguments\n");
		exit(1);
	}

	setup_network(net_type,ip,port);
}

ssize_t send_data(char* data, int data_len){
	return send(sock, data, data_len, 0);
}
ssize_t receive_data(char* buf, int buf_len){
	return read(sock, buf, buf_len);
}





