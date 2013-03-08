#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <alsa/asoundlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include "network.h"

int SAMPLE_TIME = 20; // sample size in milliseconds
int SAMPLE_RATE = 8000;
int SAMPLES_PER_PACKET = 1;
int SAMPLE_SIZE;
int DROP_RATE = 0;
int SPEECH_DETECT = 1;
int ITL,ITU;

snd_pcm_t *handle_out;
snd_pcm_t *handle_in;

double energy(unsigned char* sample){
	int i;
	double sum=0;

	sample = &(sample[SAMPLE_SIZE/2]);
	for(i=-(SAMPLE_SIZE/2);i<(SAMPLE_SIZE/2);i++){
		sum += abs(sample[i]-127);
	}

	return sum;
}

void* listen_thread(){	
	int buf_len = sizeof(char)*SAMPLE_SIZE*SAMPLES_PER_PACKET;
	char* buf = (char*)malloc(buf_len);
	char* silence[SAMPLE_SIZE];
	int received,played;
	int maxfd;
	int next_timer = SAMPLE_TIME*1.5*1000;
	int sock = get_socket();
	fd_set rset;
	struct timeval alarm;

	
	if(snd_pcm_open(&handle_out, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0){
		printf("snd_pcm_open -- failed to open playback device\n");
		exit(1);
	}

	if(snd_pcm_set_params(
			handle_out,
			SND_PCM_FORMAT_U8,	
			SND_PCM_ACCESS_RW_INTERLEAVED,
			1, // channels
			SAMPLE_RATE, // sample rate
			1, // allow resampling
			500000 // .5 seconds
		) < 0 ){
		printf("snd_pcm_set_params -- failed to set parameters on playing device\n");
		exit(1);
	}

	FD_ZERO(&rset);
	bzero(silence,SAMPLE_SIZE);

	while(1){
		FD_SET(sock, &rset);
		maxfd = sock+1;

		alarm.tv_sec = 2;
		alarm.tv_usec = next_timer;

		if(select(maxfd,&rset,NULL,NULL,&alarm) == -1){
			printf("select failed\n");
			exit(1);
		}

		if(FD_ISSET(sock, &rset)){
			received = receive_data(buf, buf_len);
			int x = snd_pcm_writei(handle_out,buf,received);
			next_timer = 1000 * received / 8;
			if(x<0){
				printf("Recovering: %s\n",snd_strerror(x));
				snd_pcm_recover(handle_out,x,1);
			}
			buf = (char*)malloc(buf_len); 	
		}else{
			int x = snd_pcm_writei(handle_out,silence,SAMPLE_SIZE);
			next_timer = SAMPLE_TIME*1.5*1000;
		}

	}

	
}

void initialize_audio(){
	if(snd_pcm_open(&handle_in, "default", SND_PCM_STREAM_CAPTURE, 0) < 0){
		printf("snd_pcm_open -- failed to open recording device\n");
		exit(1);
	}

	if(snd_pcm_set_params(
			handle_in,
			SND_PCM_FORMAT_U8,	
			SND_PCM_ACCESS_RW_INTERLEAVED,
			1, // channels
			SAMPLE_RATE, // sample rate
			1, // allow resampling
			500000 // .5 seconds
		) < 0 ){
		printf("snd_pcm_set_params -- failed to set parameters on recording device\n");
		exit(1);
	}


}

void initialize_thresholds(){
	
	unsigned char buffer[SAMPLE_SIZE*10];
	int energy_sum = 0;
	double energy_peak = -999;
	double energy_min = 100000;
	int i,IMX,IMN,I1,I2;
	double energy_avg;

	snd_pcm_readi(handle_in,buffer,SAMPLE_SIZE*10);

	double tmp_energy;

	for(i=0;i<10;i++){
		tmp_energy = energy(&buffer[i*SAMPLE_SIZE]);
		energy_sum += tmp_energy;
		if(tmp_energy > energy_peak)
			energy_peak = tmp_energy;
		else if(tmp_energy < energy_min)
			energy_min = tmp_energy;
	}

	energy_avg = energy_sum / 10.0;

	IMX = energy_peak;
	IMN = energy_avg;
	I1 = .03 * (IMX - IMN) + IMN;
	I2 = 4 * IMN;
	if(I1<I2){
		ITL = I1;
	}else{
		ITL = I2;
	}

	ITU = 5 * ITL;

	printf("ITU: %d\n",ITU);
	printf("ITL: %d\n",ITL);
}



int main(int argc, char** argv){

	char c;
	char *ip,*net_type;
	int port;
	extern int optind, opterr;
	extern char* optarg;

	while (( c = getopt( argc, argv, "m:a:p:s::d::v::" )) != EOF) {
		switch ( c ) {
			case 'm':
				net_type = optarg;
				break;
			case 'a':
				ip = optarg;
				break;
			case 'p':
				port = 	atoi(optarg);
				break;		
			case 's':
				SAMPLE_TIME = atoi(optarg);
				break;
			case 'd':
				DROP_RATE = atoi(optarg);
				break;	
			case 'v':
				SPEECH_DETECT = 0;
				break;

		}
	}

	printf("Drop rate: %d\n",DROP_RATE);
	printf("Sample time: %d msec\n",SAMPLE_TIME);


	SAMPLE_SIZE = SAMPLE_TIME*SAMPLE_RATE / 1000;

	printf("Sample size: %d bytes\n",SAMPLE_SIZE);

	// init network
	initialize_network(net_type,ip,port);
	
	// init audio
	initialize_audio();
	
	// setup thresholds
	initialize_thresholds();

	// start listening thread
	pthread_t listener;
	pthread_create(&listener, NULL, (void*)listen_thread, NULL);
	
	// being recording
	int buf_len = sizeof(char)*SAMPLE_SIZE;
	char* out_buffer = (char*)malloc(buf_len);
	int speech = 0;
	double sample_energy;
	srandom(time(NULL));
	while(1){
		snd_pcm_readi(handle_in,out_buffer,SAMPLE_SIZE);

		if(SPEECH_DETECT==1){
			sample_energy = energy(out_buffer);
		
			if(sample_energy > ITU || (sample_energy > ITL && speech == 1)){
				speech = 1;
				if(random()%100 > DROP_RATE)
					send_data(out_buffer, SAMPLE_SIZE);
				out_buffer = (char*)malloc(buf_len);
			}else if(sample_energy < ITL && speech == 1){
				if(random()%100 > DROP_RATE){
					int x = send_data(out_buffer, SAMPLE_SIZE);
				}
				out_buffer = (char*)malloc(buf_len);
				speech = 0;
			}
		}else{
			int x = send_data(out_buffer, SAMPLE_SIZE);
			out_buffer = (char*)malloc(buf_len);	
		}
	
	}
	
	pthread_join(listener,NULL);
}

