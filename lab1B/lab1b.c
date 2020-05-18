#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex);
struct option args[] = {
  {"append",0,NULL,'a'},
  {"cloexec",0,NULL,'b'},
  {"creat",0,NULL,'c'},
  {"directory",0,NULL,'d'},
  {"dsync",0,NULL,'e'},
  {"excl",0,NULL,'f'},
  {"nofollow",0,NULL,'g'},
  {"nonblock",0,NULL,'h'},
  {"rsync",0,NULL,'i'},
  {"sync",0,NULL,'j'},
  {"trunc",0,NULL,'k'},
  {"rdonly", 1, NULL, 'l'},
  {"wronly", 1, NULL, 'm'},
  {"rdwr",1,NULL,'n'},
  {"pipe",0,NULL,'o'},
  {"command", 1, NULL, 'p'},
  {"wait",0,NULL,'q'},
  {"verbose", 0, NULL, 'r'},
  {"chdir",1,NULL,'s'},
  {"close",1,NULL,'t'},
  {"profile",0,NULL,'u'},
  {"abort",0,NULL,'v'},
  {"catch",1,NULL,'w'},
  {"ignore",1,NULL,'x'},
  {"default",1,NULL,'y'},
  {"pause",0,NULL,'z'},
  {0,0,0,0}
};
void handler(int sig){
	fprintf(stderr,"%i caught",sig);
	exit(sig);
}
struct kid{
	int pid;
	char com[1024];
};
int main(int argc, char *argv[]){
	setvbuf(stdout, NULL, _IONBF, 0);
	char i;
	int j;
	int *abortP = NULL;
	int k = 0;
	int fd;
	int forkresult;
	int first;
	int ifd;
	int ofd;
	int oflag = 0;
	int efd;
	int pip[2];
	int fdarr[100];
	struct kid karr[100];
	char *command[1024];
	char com[1024];
	int highestfd = 0;
	int highestCh = 0;
	int verbose = 0;
	int status;
	int highestsignal = 0;
	int exitcode = 0;
	while( (i = getopt_long(argc, argv, "", args, NULL)) != -1){
		switch(i){
			case 'a':
				oflag = oflag | O_APPEND; 
				break;
			case 'b':
				oflag = oflag | O_CLOEXEC;
				break;
			case 'c':
				oflag = oflag | O_CREAT;
				break;
			case 'd':
				oflag = oflag | O_DIRECTORY;
				break;
                        case 'e':
				oflag = oflag | O_DSYNC;
                                break;
                        case 'f':
				oflag = oflag | O_EXCL;
                                break;
                        case 'g':
				oflag = oflag | O_NOFOLLOW;
                                break;
                        case 'h':
				oflag = oflag | O_NONBLOCK;
                                break;
                        case 'i':
				oflag = oflag | O_RSYNC;
                                break;
                        case 'j':
				oflag = oflag | O_SYNC;
                                break;
                        case 'k':
				oflag = oflag | O_TRUNC;
                                break;
			case 'l': 
				fd = open(optarg,O_RDONLY | oflag, 0666);
				oflag = 0;
				if(fd < 0){
					fprintf(stderr,"Cannot open %s read only\n", optarg);
					exitcode = 1;
				}
				fdarr[highestfd++] = fd;
				if(verbose){
					fprintf(stdout,"--rdonly %s\n",optarg);
				}
				break;
			case 'm':
				fd = open(optarg,O_WRONLY | oflag, 0666);
                                oflag = 0;
				if(fd < 0){
                                        fprintf(stderr,"Cannot open %s write only\n", optarg);
                                        exitcode = 1;
                                }
				fdarr[highestfd++] = fd;
				if(verbose){
                                        fprintf(stdout,"--wronly %s\n",optarg);
                                }
				break;
			case 'n':
				fd = open(optarg,O_RDWR | oflag, 0666);
                                oflag = 0;
				if(fd < 0){
                                        fprintf(stderr,"Cannot open %s read write\n", optarg);
                                        exitcode = 1;
                                }
                                fdarr[highestfd++] = fd;
                                if(verbose){
                                        fprintf(stdout,"--rdwr %s\n",optarg);
                                }
                                break;

				break;
			case 'o':
				if( pipe(pip) < 0){
					fprintf(stderr,"Pipe failed\n");
					exitcode = 1;
				}
				fdarr[highestfd++] = pip[0];
				fdarr[highestfd++] = pip[1];
				if(verbose){
					fprintf(stdout,"--pipe\n");
				}
				break;
			case 'p':
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
				if(optind == first + 2){
					fprintf(stderr,"No command specified\n");
					exitcode = 1;
				}
				ifd = atoi(argv[first-1]);
				ofd = atoi(argv[first]);
				efd = atoi(argv[first+1]);
                                if(ifd >= highestfd || ofd >= highestfd || efd >= highestfd){
					fprintf(stderr,"Bad file descriptors\n");
					exitcode = 1;
				}
				if(fdarr[ifd] == -1 || fdarr[ofd] == -1 || fdarr[efd] == -1){
					fprintf(stderr,"File descriptor closed\n");
					exitcode = 1;
				}
				forkresult = fork();
                                if(forkresult == 0){
					close(0);
					dup(fdarr[ifd]);
					close(1);
					dup(fdarr[ofd]);
					close(2);
					dup(fdarr[efd]);
					close(fdarr[ifd]);
					close(fdarr[ofd]);
					close(fdarr[efd]);
					for(int i = 0; i < highestfd; i++){
                                                close(fdarr[i]);
                                        }
					for(j = first+2; j < optind; j++){
						command[k++] = argv[j];
					}
					command[k] = NULL;
					forkresult = execvp(command[0],command);
					if(forkresult < 0){
						exitcode = 1;
						fprintf(stderr,"Cannot run command %s\n",strerror(errno));
					}
					close(0);
					close(1);
					close(2);
				}
				else{
					for(j = first+2; j < optind; j++){
                                                strcat(com,argv[j]);
						strcat(com," ");
                                        }
					karr[highestCh].pid = forkresult;
					strcpy(karr[highestCh++].com, com);
					strcpy(com,"");
				}
				break;
			case 'q':
				if(verbose){
					fprintf(stdout, "--wait\n");
				}
				for(int i = 0; i < highestCh; i++){
					if( waitpid(karr[i].pid,&status,0) >= 0){
                                                if(WIFEXITED(status)){
                                                        printf("exit %d %s\n",WEXITSTATUS(status),karr[i].com);
							if(WEXITSTATUS(status)>exitcode){
								exitcode = WEXITSTATUS(status);
							}
                                                }
						if(WIFSIGNALED(status)){
							printf("signal %d %s\n",WTERMSIG(status),karr[i].com);
							if(WTERMSIG(status)>highestsignal){
                                                                highestsignal = WTERMSIG(status);
                                                        }
						}
                                         }
				}
				highestCh = 0;
				break;
			case 'r': 
				verbose = 1;
				break;
		        case 's':
				forkresult = chdir(optarg);
				if(forkresult < 0){
					fprintf(stderr, "Change directory error\n");
					exitcode = 1;
				}
				if(verbose){
					fprintf(stdout, "--chdir %s\n",optarg);
				}
                                break;
                        case 't':
				ifd = atoi(optarg);
				if(ifd >= highestfd){
					fprintf(stderr,"Trying to close unused fd\n");
				}
				if(fdarr[ifd] == -1){
					fprintf(stderr,"Already closed fd\n");
				}
				ofd = close(fdarr[ifd]);
				if(ofd < 0){
					fprintf(stderr,"Error closing fd\n");
				}
				if(verbose){
					fprintf(stdout,"--close %s\n",optarg);
				}
				fdarr[ifd] = -1;
                                break;
                        case 'u':
                                break;
                        case 'v':
				if(verbose){
					printf("--abort\n");
				}
				*abortP = 0;
                                break;
                        case 'w':
				if(verbose){
					fprintf(stdout,"--catch %s\n",optarg);
				}
				ifd = atoi(optarg);
				signal(ifd,handler);
                                break;
                        case 'x':
				if(verbose){
					fprintf(stdout,"--ignore %s\n",optarg);
				}
				ifd = atoi(optarg);
				signal(ifd, SIG_IGN);
                                break;
                        case 'y':
				ifd = atoi(optarg);
				if(verbose){
					fprintf(stdout,"--default %s\n",optarg);
				}
				signal(ifd, SIG_DFL);
                                break;
                        case 'z':
				if(verbose){
					fprintf(stdout,"--pause\n");
				}
				pause();
                                break;
			default:
				exitcode = 1;
				break;
		}
	}
	if(highestsignal>0){
		signal(highestsignal,SIG_DFL);
		raise(highestsignal);
	}
	exit(exitcode);
}
