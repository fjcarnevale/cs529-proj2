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

void setup_udp_server(int port){
	if((sock = socket(AF_INET,SOCK_DGRAM,0))<0){
		printf("Failed to open socket\n");
		exit(1);
	}

	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
     		printf("Failed to bind to local address\n");
     		exit(1);
   	}

	char tmp[1];
	int len = sizeof(struct sockaddr_in);
	//printf("Waiting for dummy packet\n");
	recvfrom(sock,tmp,1,0,(struct sockaddr*)&client_addr,&len);
	//printf("Got dummy from client\n");
}

void udp_connect(char* ip, int port){
	if((sock = socket(AF_INET,SOCK_DGRAM,0))<0){
		printf("Failed to open socket\n");
		exit(1);
	}

	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	if (connect(sock,(struct sockaddr *) &server_addr,sizeof(server_addr)) < 0) {
		printf("Failed to connect UDP socket to server\n");
		close(sock);
		exit(1);
	}

	char* tmp = (char*)malloc(sizeof(char));
	int x = sendto(sock,tmp,1,0,(struct sockaddr*)&server_addr,sizeof(server_addr));

	//printf("Sent dummy len: %d\n",x);
}


int setup_network(char* ip, int port){

	if(net_type == TCP_CLIENT){
		tcp_connect(ip, port);
	}else if(net_type == TCP_SERVER){
		setup_tcp_server(port);
	}else if(net_type == UDP_SERVER){
		setup_udp_server(port);
	}else{
		udp_connect(ip,port);
	}
}


int initialize_network(char* network_type, char* ip, int port){
	if(!strcmp(network_type,"tcpclient"))
		net_type = TCP_CLIENT;
	else if(!strcmp(network_type,"tcpserver"))
		net_type = TCP_SERVER;
	else if(!strcmp(network_type,"udpclient"))
		net_type = UDP_CLIENT;
	else
		net_type = UDP_SERVER;

	setup_network(ip,port);
}

ssize_t send_data(char* data, int data_len){
	if(net_type == TCP_CLIENT || net_type == TCP_SERVER)
		return send(sock, data, data_len, 0);
	else
		return sendto(sock,data,data_len,0,(struct sockaddr*)&client_addr,sizeof(client_addr));
}
ssize_t receive_data(char* buf, int buf_len){
	if(net_type == TCP_CLIENT || net_type == TCP_SERVER){
		return recv(sock, buf, buf_len, 0);
	}else{
		int len = sizeof(struct sockaddr_in);
		return recvfrom(sock,buf,buf_len,0,(struct sockaddr*)&client_addr.sin_addr,&len);
	}
}

int get_socket(){
	return sock;
}




