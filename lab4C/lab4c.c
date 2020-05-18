#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <poll.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <sys/socket.h>
#include <netdb.h>

int period = 1;
char scale = 'F';
int loggingFd = 0;
mraa_aio_context tempSensor;
int id = -1;
int port = -1;
char *host;

struct option args[] = {
	{"period", required_argument, NULL, 'a'},
	{"scale", required_argument, NULL, 'b'},
	{"log", required_argument, NULL, 'c'},
	{"host", required_argument, NULL, 'd'},
	{"id", required_argument, NULL, 'e'},
	{0,0,0,0}
};
float convert_temp(int reading){
	float R = 1023.0/((float)reading)-1.0;
	R= 100000.0*R;
	float cel = 1.0/(log(R/100000.0)/4275 + 1/298.15) - 273.15;
	if(scale == 'F'){
		return cel * 9/5 + 32;
	}
	return cel;
}
int connectHost(char *hostname, int portnum){
	struct sockaddr_in serv_addr; 
	int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if(sockfd < 0){
		fprintf(stderr, "Unable to connect to socket\n");
		exit(2);
	}
	struct hostent *server = gethostbyname(hostname);  
	if(server == NULL){
		fprintf(stderr, "Unable to get host by name\n");
		exit(2);
	}
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length); 
	serv_addr.sin_port = htons(portnum);
	int ret = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if(ret < 0){
		fprintf(stderr, "Couldn't connect to the server\n");
	}
    return sockfd; 
}
int main(int argc, char **argv){
	int switchVar;
	while((switchVar = getopt_long(argc, argv, "", args, NULL)) != -1){
		switch(switchVar){
			case 'a':
				period = atoi(optarg);
				break;
			case 'b':
				if(optarg[0] != 'F' && optarg[0] != 'C'){
					fprintf(stderr, "Incorrect argument passed, only 'F' or 'C' supported.\n");
					exit(1);
				}
				scale = optarg[0];
				break;
			case 'c':
				loggingFd = open(optarg, O_WRONLY|O_CREAT|O_TRUNC,0666);
				if(loggingFd < 0){
					fprintf(stderr, "Error opening log file.\n");
					exit(1);
				}
				break;
			case 'd':
				host = optarg;
				break;
			case 'e':
				if(strlen(optarg) != 9){
					fprintf(stderr, "ID length must be 9\n");
					exit(1);
				}
				id = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Incorrect argument passed, only --period --scale --log supported.\n");
				exit(1);
				break;
		}
	}
	port = atoi(argv[optind]);

	if(id == -1 || port < 1024 || host == NULL){
		fprintf(stderr, "Missing required argument id, port or host\n");
		exit(1);
	}
	int socketNum = connectHost(host, port);
	char toWrite[15];
	sprintf(toWrite, "ID=%d\n",id);
	write(socketNum, toWrite, strlen(toWrite));
	if(loggingFd != 0){
		write(loggingFd, toWrite, strlen(toWrite));
	}
	tempSensor = mraa_aio_init(1);
	if(tempSensor == NULL){
		fprintf(stderr, "Cannot initialize temp sensor.\n");
		exit(1);
	}
	struct timeval current;
	struct timeval previous;
	gettimeofday(&previous,NULL);
	gettimeofday(&current,NULL);
	struct pollfd pollv;
	pollv.fd = socketNum;
	pollv.events = POLLIN;
	char command[64];
	int commandlen = 0;
	int printing = 1;
	while(1){
		gettimeofday(&current,NULL);
		if(printing && current.tv_sec >= previous.tv_sec + period){
			struct tm *local;
			local = localtime(&current.tv_sec);
			char toWrite[32];
			sprintf(toWrite, "%02d:%02d:%02d %.1f\n", local->tm_hour, local->tm_min, local->tm_sec, convert_temp(mraa_aio_read(tempSensor)));
			write(socketNum, toWrite, strlen(toWrite));
			if(loggingFd != 0){
				write(loggingFd, toWrite, strlen(toWrite)); 
			}
			gettimeofday(&previous,NULL);
		}
		int ret = poll(&pollv, 1, 0);
	    if (ret > 0){
			char processing;
			read(socketNum,&processing,1);
			if(processing != '\n'){
				command[commandlen++] = processing;
			}
			else{
				command[commandlen] = '\0';
				if(strcmp(command, "STOP") == 0){
					printing = 0;
				}
				else if(strcmp(command, "START") == 0){
					printing = 1;
				}
				else if(strcmp(command, "OFF") == 0){
					struct tm *local;
					local = localtime(&current.tv_sec);
					char towrite[32];
					sprintf(towrite, "%02d:%02d:%02d SHUTDOWN\n",local->tm_hour, local->tm_min, local->tm_sec);
					write(socketNum, towrite, strlen(towrite));
					if(loggingFd != 0){
						write(loggingFd, "OFF\n", 4);
						write(loggingFd, towrite, strlen(towrite));
					}
					exit(0);
				}
				else if(strcmp(command, "SCALE=F") == 0){
					scale = 'F';
				}
				else if(strcmp(command, "SCALE=C") == 0){
					scale = 'C';
				}
				else if(strncmp(command, "PERIOD=", 7) == 0){
					period = atoi(command+7);
				}
				else if(strncmp(command, "LOG", 3) == 0){
					
				}
				else{
					fprintf(stderr, "Incorrect command received\n");
					exit(1);
				}
				if(loggingFd != 0){
					write(loggingFd, command, strlen(command));
					write(loggingFd, "\n", 1);
				}
				commandlen = 0;
			}
		}
	}
}
