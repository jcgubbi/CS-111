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
  {"input", 1, NULL, 'i'},
  {"output", 1, NULL, 'o'},
  {"segfault", 0, NULL, 's'},
  {"catch", 0, NULL, 'c'},
  {"dump-core", 0, NULL, 'd'},
  {0,0,0,0}
};
void handler(){
	fprintf(stderr,"Segfault caught and handled - %s\n",strerror(errno));
	exit(4);
}
int main(int argc, char *argv[]){
	char* infile = NULL;
	char* outfile = NULL;
	int segfault = 0;
	int catch = 0;
	int dump_core = 0;
	int filein = 0;
	int fileout = 1;
	int readCount = 0;
	int writeCount = 0;
	char i;
	while( (i = getopt_long(argc, argv, "", args, NULL)) != -1){
		switch(i){
			case 'i': 
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 's':
				segfault = 1;
				break;
			case 'c': 
				catch = 1; 
				break;
			case 'd':
				dump_core = 1;
				break;
			default:
				exit(1);
				break;
		}
	}
	if(dump_core && catch && segfault){
                int* a = NULL;
                *a = 0;
        }
	if(segfault && !catch){
		int* a = NULL;
		*a = 0;
	}
	if(segfault && catch && !dump_core){
		int *a = NULL;
		signal(SIGSEGV,handler);
		*a = 0;
	}
	if(infile != NULL){
		char cwd[1024];
		getcwd(cwd,sizeof(cwd));
		strcat (cwd,"/");
		strcat (cwd, infile);
		filein = open(cwd, O_RDONLY);
		if (filein < 0){
			fprintf(stderr,"Error: %s. Unable to open input file %s\n",strerror(errno),cwd);
			exit(2);
		}
		//NEED TO DUP
		close(0);
		dup(filein);
		close(filein);
	}
	if(outfile != NULL){
		char cwd[1024];
                getcwd(cwd,sizeof(cwd));
                strcat (cwd,"/");
                strcat (cwd, outfile);
                fileout = open(cwd, O_WRONLY | O_APPEND);
                if (fileout < 0){
			fileout = creat(outfile,S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
			if(fileout < 0){
				fprintf(stderr, "Error: %s. Unable to open output file %s\n",strerror(errno),cwd);
				exit(3);
			}
		}
		else{
			if(ftruncate(fileout, 0) != 0){
				fprintf(stderr, "Error: %s. Unable to open output file %s\n",strerror(errno),cwd);
				exit(3);
			}
		}
		//NEED TO DUP
		close(1);
		dup(fileout);
		close(fileout);
	}
	char* buff;
	buff = (char*) malloc(sizeof(char));
	readCount = read(0, buff, 1);
	while(1){
		writeCount = write(1, buff, 1);
		readCount = read(0,buff,1);
		if(readCount <=0 || writeCount <=0){
			break;
		}
	}	
	free(buff);
	return 0;
}
