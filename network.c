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
	TCP_CLIENT,TCP_SERVER,UDP_CLIENT,UDP_SERVER
}NETWORK_TYPE;

NETWORK_TYPE net_type;

int sock,server_sock;
struct sockaddr_in server_addr,client_addr;

void setup_tcp_server(int port){
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

	printf("Waiting for client to connect...\n");

	int client_len = sizeof(client_addr);
	sock = accept(server_sock,(struct sockaddr*)&client_addr,&client_len);
	if(sock < 0){
		printf("Failed to accept connection\n");
		exit(1);
	}

	printf("Connected\n");
}

void tcp_connect(char* ip, int port){
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

	printf("Connected to server\n");
}

void setup_udp_server(char* ip, int port){
	if((sock = socket(AF_INET,SOCK_DGRAM,0))<0){
		printf("Failed to open socket\n");
		exit(1);
	}
}

void udp_connect(char* ip, int port){


}


int setup_network(char* ip, int port){

	if(net_type == TCP_CLIENT){
		tcp_connect(ip, port);
	}else if(net_type == TCP_SERVER){
		setup_tcp_server(port);
	}else if(net_type == UDP_SERVER){
		setup_udp_server(ip, port);
	}else{
		udp_connect(ip,port);
	}
}


int initialize_network(int argc, char** argv){
	char* ip = NULL;
	int port = -1;

	char c;
	extern int optind, opterr;
	extern char* optarg;

	while (( c = getopt( argc, argv, "m:a:p:" )) != EOF) {
		switch ( c ) {
			case 'm':
				if(!strcmp(optarg,"tcpclient"))
					net_type = TCP_CLIENT;
				else if(!strcmp(optarg,"tcpserver"))
					net_type = TCP_SERVER;
				else if(!strcmp(optarg,"udpclient"))
					net_type = UDP_CLIENT;
				else if(!strcmp(optarg,"udpserver"))
					net_type = UDP_SERVER;
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

	setup_network(ip,port);
}

ssize_t send_data(char* data, int data_len){
	if(net_type == TCP_CLIENT || net_type == TCP_SERVER)
		return send(sock, data, data_len, 0);
	else
		return sendto(sock,data,data_len, 0, NULL, 0); // replace NULL with sockaddr* struct
}
ssize_t receive_data(char* buf, int buf_len){
	if(net_type == TCP_CLIENT || net_type == TCP_SERVER)
		return recv(sock, buf, buf_len, 0);
	else
		return recvfrom(sock,buf,buf_len,0, NULL, 0);
}





