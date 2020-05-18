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

int period = 1;
char scale = 'F';
int loggingFd = 0;
mraa_aio_context tempSensor;
mraa_gpio_context buttonSensor;


struct option args[] = {
	{"period", required_argument, NULL, 'a'},
	{"scale", required_argument, NULL, 'b'},
	{"log", required_argument, NULL, 'c'},
	{0,0,0,0}
};
void shutoff(){
	struct timeval current;
	gettimeofday(&current,NULL);
	struct tm *local;
	local = localtime(&current.tv_sec);
	char toWrite[32];
	sprintf(toWrite, "%02d:%02d:%02d SHUTDOWN\n", local->tm_hour, local->tm_min, local->tm_sec);
	printf("%s", toWrite);
	if(loggingFd != 0){
		write(loggingFd, toWrite, strlen(toWrite));
	}
	exit(0);
}
float convert_temp(int reading){
	float R = 1023.0/((float)reading)-1.0;
	R= 100000.0*R;
	float cel = 1.0/(log(R/100000.0)/4275 + 1/298.15) - 273.15;
	if(scale == 'F'){
		return cel * 9/5 + 32;
	}
	return cel;
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
			default:
				fprintf(stderr, "Incorrect argument passed, only --period --scale --log supported.\n");
				exit(1);
				break;
		}
	}
	tempSensor = mraa_aio_init(1);
	buttonSensor = mraa_gpio_init(60);
	if(tempSensor == NULL){
		fprintf(stderr, "Cannot initialize temp sensor.\n");
		exit(1);
	}
	if(buttonSensor == NULL){
		fprintf(stderr, "Cannot initalize button.\n");
		exit(1);
	}
	if( mraa_gpio_dir(buttonSensor, MRAA_GPIO_IN) != MRAA_SUCCESS){
		fprintf(stderr, "Cannot make button input.\n");
		exit(1);
	}
	if( mraa_gpio_isr(buttonSensor, MRAA_GPIO_EDGE_RISING, &shutoff, NULL) != MRAA_SUCCESS){
		fprintf(stderr, "Cannot shutdown on button press.\n");
		exit(1);
	}
	struct timeval current;
	struct timeval previous;
	gettimeofday(&previous,NULL);
	gettimeofday(&current,NULL);
	struct pollfd pollv;
	pollv.fd = 0;
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
			printf("%s", toWrite);
			if(loggingFd != 0){
				write(loggingFd, toWrite, strlen(toWrite)); 
			}
			gettimeofday(&previous,NULL);
		}
		int ret = poll(&pollv, 1, 0);
	       	if (ret > 0){
			char processing;
			read(0,&processing,1);
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
					printf("OFF\n");
					printf("%s", towrite);
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
