#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex);
struct option args[] = {
  {"rdonly", 1, NULL, 'r'},
  {"wronly", 1, NULL, 'w'},
  {"command", 1, NULL, 'c'},
  {"verbose", 0, NULL, 'v'},
  {0,0,0,0}
};
int main(int argc, char *argv[]){
	char i;
	int j;
	int k = 0;
	int fd;
	int forkresult;
	int first;
	int ifd;
	int ofd;
	int efd;
	int fdarr[100];
	char *command[1024];
	int highestfd = 0;
	int verbose = 0;
	int exitcode = 0;
	while( (i = getopt_long(argc, argv, "", args, NULL)) != -1){
		switch(i){
			case 'r': 
				fd = open(optarg,O_RDONLY);
				if(fd < 0){
					fprintf(stderr,"Cannot open %s read only", optarg);
					exitcode = 1;
				}
				fdarr[highestfd++] = fd;
				if(verbose){
					fprintf(stdout,"--rdonly %s\n",optarg);
				}
				break;
			case 'w':
				fd = open(optarg,O_WRONLY);
                                if(fd < 0){
                                        fprintf(stderr,"Cannot open %s write only", optarg);
                                        exitcode = 1;
                                }
				fdarr[highestfd++] = fd;
				if(verbose){
                                        fprintf(stdout,"--wronly %s\n",optarg);
                                }
				break;
			case 'c':
				first = optind;
				while(optind < argc && (argv[optind][0] != '-' || argv[optind][1] != '-')){
					optind++;
				}
				if(verbose){
                                        for(j = first-2; j<optind; j++){
						fprintf(stdout,"%s ",argv[j]);
					}
					fprintf(stdout,"\n");
                                }
				ifd = atoi(argv[first-1]);
				ofd = atoi(argv[first]);
				efd = atoi(argv[first+1]);
                                if(ifd >= highestfd || ofd >= highestfd || efd >= highestfd){
					fprintf(stderr,"Bad file descriptors");
					exitcode = 1;
				}
				forkresult = fork();
                                if(forkresult == 0){
					close(0);
					dup(fdarr[ifd]);
					close(fdarr[ifd]);
					close(1);
					dup(fdarr[ofd]);
					close(fdarr[ofd]);
					close(2);
					dup(fdarr[efd]);
					close(fdarr[efd]);
					
					for(j = first+2; j < optind; j++){
						command[k++] = argv[j];
					}
					command[k] = NULL;
					forkresult = execvp(command[0],command);
					if(forkresult < 0){
						exitcode = 1;
						fprintf(stderr,"Execvp error");
					}
				}
				break;
			case 'v': 
				verbose = 1;
				break;
			default:
				exitcode = 1;
				break;
		}
	}
	exit(exitcode);
}
