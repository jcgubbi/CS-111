#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include "SortedList.h"

int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex);
struct option args[] = {
  {"iterations",1,NULL,'a'},
  {"threads",1,NULL,'b'},
  {"yield",1,NULL,'c'},
  {"sync",1,NULL,'d'},
  {0,0,0,0}
};

int yield = 0;
pthread_t tid[12];
long long counter = 0;
int iterations = 1;
int threads = 1;
int optSwitch;
int opt_yield = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long long spinlock = 0;
char sync = 'n';
SortedList_t* list; 
SortedListElement_t* arrayList; 
int sizeKey = 4;

void delete(int loc){
	SortedListElement_t *ptr = SortedList_lookup(list, arrayList[loc].key);
	if(ptr == NULL){
		fprintf(stderr, "List is corrupted -- SortedList_lookup\n");
	}
	if(SortedList_delete(ptr) != 0){
		fprintf(stderr, "List is corrupted -- SortedList_lookup\n");
	}
}
void* thread_funct(void * arg){
	int ref = *((int * ) arg);
	int fin = ref+iterations;
	for(int i = ref; i < fin; i++){
		if(sync == 'm'){
			pthread_mutex_lock(&mutex);
			SortedList_insert(list, &arrayList[i]);
			pthread_mutex_unlock(&mutex);
		}
		else if(sync == 's'){
			while(__sync_lock_test_and_set(&spinlock,1)){continue;}
			SortedList_insert(list, &arrayList[i]);
			__sync_lock_release(&spinlock);
		}
		else{
			SortedList_insert(list, &arrayList[i]);
		}
	}
	int len = 0;
	for(int i = ref; i < fin; i++){
		if(sync == 'm'){
			pthread_mutex_lock(&mutex);
			len = SortedList_length(list);
			pthread_mutex_unlock(&mutex);
		}
		else if(sync == 's'){
			while(__sync_lock_test_and_set(&spinlock,1)){continue;}
			len = SortedList_length(list);
			__sync_lock_release(&spinlock);
		}
		else{
			len = SortedList_length(list);
		}
	}
	if(len < 0){
		fprintf(stderr,"List is corrupted -- SortedList_length\n");
		exit(2);
	}
	for(int i = ref; i < fin; i++){
		if(sync == 'm'){
			pthread_mutex_lock(&mutex);
			delete(i);
			pthread_mutex_unlock(&mutex);
		}
		else if(sync == 's'){
			while(__sync_lock_test_and_set(&spinlock,1)){continue;}
			delete(i);
			__sync_lock_release(&spinlock);
		}
		else{
			delete(i);
		}
	}
	return NULL;
}
void handler(int num){
	if(num == SIGSEGV){
		fprintf(stderr, "Segmentation fault caught -- corrupted list.\n");
		exit(2);
	}
}
int main(int argc, char **argv){
	signal(SIGSEGV, handler);
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
				int len = strlen(optarg);
				for(int i = 0; i < len; i++){
					if(optarg[i] == 'i'){
						opt_yield |= INSERT_YIELD;
					}
					else if(optarg[i] == 'd'){
						opt_yield |= DELETE_YIELD;
					}
					else if(optarg[i] == 'l'){
						opt_yield |= LOOKUP_YIELD;
					}
					else{
						fprintf(stderr,"Incorrect sync options. Must use [idl]");
						exit(1);
					}
				}
				break;
			case 'd':
				sync = optarg[0];
				if(sync != 'm' && sync != 's'){
					fprintf(stderr,"Incorrect sync options. Must use m or s");
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
	//start monitoring
	struct timespec my_start_time;
	clock_gettime(CLOCK_MONOTONIC, &my_start_time);
	list = malloc(sizeof(SortedList_t));
	SortedList_t* h = list;
	list->next = h;
	list->prev = h;
	list->key = NULL;
	arrayList = malloc(sizeof(SortedListElement_t) * threads * iterations);
	for(int i = 0; i < threads*iterations; i++){
		char *key = malloc(sizeof(char) *(sizeKey+1));
		for (int j = 0; j < sizeKey; j++){
			key[j] = ((char) rand() % 26) +'a';
		}
		key[sizeKey] = '\0';
		arrayList[i].key = key;
	}
	//initialize array
	int* threadStart = malloc(threads*sizeof(int));
	for(int i = 0; i < threads; i++){
		threadStart[i] = iterations*i;
	}
	for(int i = 0; i < threads; i++){
		pthread_create(tid+i, NULL,thread_funct, (void *)&threadStart[i]);
	}
	for(int i = 0; i < threads; i++){
		pthread_join(tid[i],NULL);
	}
	struct timespec my_end_time;
	clock_gettime(CLOCK_MONOTONIC, &my_end_time);
	long long timeRunning = my_end_time.tv_sec*1000000000+my_end_time.tv_nsec - my_start_time.tv_sec*1000000000 - my_start_time.tv_nsec;
	long ops = 3*threads*iterations;
	char prefix[50] = "";
	strcat(prefix,"list-");
	if(opt_yield & INSERT_YIELD){
		strcat(prefix, "i");
	}
	if(opt_yield & DELETE_YIELD){
		strcat(prefix,"d");
	}
	if(opt_yield & LOOKUP_YIELD){
		strcat(prefix,"l");
	}
	if(!opt_yield){
		strcat(prefix,"none");
	}
	strcat(prefix, "-");
	if(sync == 'm'){
		strcat(prefix, "m");
	}
	else if(sync == 's'){
		strcat(prefix, "s");
	}
	else{
		strcat(prefix, "none");
	}
	printf("%s,%d,%d,1,%ld,%lld,%lld\n", prefix, threads, iterations, ops, timeRunning, timeRunning/ops);

}