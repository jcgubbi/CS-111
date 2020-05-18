#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex);
struct option args[] = {
  {"iterations",1,NULL,'a'},
  {"threads",1,NULL,'b'},
  {"yield",0,NULL,'c'},
  {"sync",1,NULL,'d'},
  {0,0,0,0}
};

int yield = 0;
pthread_t tid[12];
long long counter = 0;
int iterations = 1;
int threads = 1;
int optSwitch;
char sync;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long long spinlock = 0;

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if(yield)
		sched_yield();
	*pointer = sum;
}
void* thread_funct(void * n){
	if(n != NULL){
		fprintf(stderr, "Invalid parameter to thread function.\n");
	}
	for(int i = 0; i < iterations; i++){
		if(sync == 'm'){
			pthread_mutex_lock(&mutex);
			add(&counter, 1);
			pthread_mutex_unlock(&mutex);
		}
		else if(sync == 's'){
			while(__sync_lock_test_and_set(&spinlock,1)){continue;}
			add(&counter, 1);
			__sync_lock_release(&spinlock);
		}
		else if(sync == 'c'){
			long long copy = 0;
			do{
				copy = counter;
			} while(__sync_val_compare_and_swap(&counter, copy, copy+1) != copy);
		}
		else{
			add(&counter, 1);
		}
	}
	for(int i = 0; i < iterations; i++){
		if(sync == 'm'){
			pthread_mutex_lock(&mutex);
			add(&counter, -1);
			pthread_mutex_unlock(&mutex);
		}
		else if(sync == 's'){
			while(__sync_lock_test_and_set(&spinlock,1)){continue;}
			add(&counter, -1);
			__sync_lock_release(&spinlock);
		}
		else if(sync == 'c'){
			long long copy = 0;
			do{
				copy = counter;
			} while(__sync_val_compare_and_swap(&counter, copy, copy-1) != copy);
		}
		else{
			add(&counter, -1);
		}
	}
	return NULL;
}

int main(int argc, char **argv){
	while( (optSwitch = getopt_long(argc, argv, "", args, NULL)) != -1){
		switch(optSwitch){
			case 'a':
				iterations = atoi(optarg);
				break;
			case 'b':
				threads = atoi(optarg);
				break;
			case 'c':
				yield = 1;
				break;
			case 'd':
				sync = optarg[0];
				if(sync != 'm' && sync != 's' && sync != 'c'){
					fprintf(stderr,"Incorrect sync options. Must use m, s, or c");
					exit(1);
				}
				break;
			default:
				exit(1);
		}
	}
	if(iterations <= 0 || threads <= 0){
		fprintf(stderr,"Invalid arguments");
		exit(1);
	}
	struct timespec my_start_time;
	clock_gettime(CLOCK_MONOTONIC, &my_start_time);
	for(int i = 0; i < threads; i++){
		pthread_create(tid+i, NULL,thread_funct, NULL);
	}
	for(int i = 0; i < threads; i++){
		pthread_join(tid[i],NULL);
	}
	struct timespec my_end_time;
	clock_gettime(CLOCK_MONOTONIC, &my_end_time);
	long long timeRunning = my_end_time.tv_sec*1000000000+my_end_time.tv_nsec - my_start_time.tv_sec*1000000000 - my_start_time.tv_nsec;
	long ops = 2*threads*iterations;
	if(yield){
		if(sync == 'm'){
			printf("add-yield-m,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, ops, timeRunning, timeRunning/ops,counter);
		}
		else if(sync == 's'){
			printf("add-yield-s,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, ops, timeRunning, timeRunning/ops,counter);
		}
		else if(sync == 'c'){
			printf("add-yield-c,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, ops, timeRunning, timeRunning/ops,counter);
		}
		else{
			printf("add-yield-none,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, ops, timeRunning, timeRunning/ops,counter);
		}
	}
	else{
		if(sync == 'm'){
			printf("add-m,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, ops, timeRunning, timeRunning/ops,counter);
		}
		else if(sync == 's'){
			printf("add-s,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, ops, timeRunning, timeRunning/ops,counter);
		}
		else if(sync == 'c'){
			printf("add-c,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, ops, timeRunning, timeRunning/ops,counter);
		}
		else{
			printf("add-none,%d,%d,%ld,%lld,%lld,%lld\n", threads, iterations, ops, timeRunning, timeRunning/ops,counter);
		}
	}
}