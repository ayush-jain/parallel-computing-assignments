#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define SIZE 1000
#define interval 10

pthread_mutex_t tlock;
pthread_cond_t qempty;
pthread_cond_t qfull;

int queue[SIZE];
int head, tail;
int n;
int insertions, extractions;

int calculate_queue_size(void) {
	int current_size = 0;
	if (head==tail && head !=-1) {
		current_size = 1;		
		// queue full
	} else if (head==tail && head==-1) {
		current_size = 0;		
		// queue empty
	} else if (tail<head) {
		current_size = (tail) + 1 + (SIZE - head);   
	} else if (tail>head) {
		current_size = (tail-head) + 1;
	}
	return current_size;
}

void *consumer(void *cdata) {
	int extracted;
	int inx = *((int *)cdata);
	//printf("%d....cinxxxx\n", inx);
	while (1) {
		extracted = 0;
		pthread_mutex_lock(&tlock);
		int current_size = calculate_queue_size();
		while(current_size <= 1){
			pthread_cond_wait(&qfull, &tlock);
			current_size = calculate_queue_size();
		}
		//printf("Extracted %d by consumer %d at head- %d\n", queue[head], inx, head);
		queue[head] = -1;
		//current_size = calculate_queue_size();
		//printf("current_size is %d....\n\n", current_size-1);
		head = (head+1)%SIZE;
		current_size--;
		extracted = 1;	
		extractions++;	
		pthread_cond_signal(&qempty);	
		pthread_mutex_unlock(&tlock);	
	}
}

void *producer(void *pdata) {
	int inserted;
	int i = 0;
	int inx = *((int *)pdata);
	//printf("%d....pinxxxx\n", inx);
	while (1) {
		int val = inx + (i*n);
		inserted = 0;
		pthread_mutex_lock(&tlock);
		int current_size = calculate_queue_size();
		while(current_size == SIZE){
			pthread_cond_wait(&qempty, &tlock);
			current_size = calculate_queue_size();		
		}
		queue[(tail+1)%SIZE] = val;
		//printf("Inserted %d by producer %d at tail-  %d\n", queue[(tail+1)%SIZE], inx, (tail+1)%SIZE);
		//current_size = calculate_queue_size();
		//printf("current_size is %d....\n\n", current_size+1);
		tail = (tail+1)%SIZE;
		current_size++;
		if(current_size == 1){
			head = tail;
		}
		inserted = 1;
		i++;
		insertions++;
		pthread_cond_signal(&qfull);
		pthread_mutex_unlock(&tlock);
	}
}

int main(int argc, char *argv[]){

	int i;
	pthread_attr_t  attr;

	double start_time, end_time;
	struct timeval tz;
	struct timezone tx;

	pthread_attr_init (&attr);
	pthread_attr_setscope (&attr,PTHREAD_SCOPE_SYSTEM);

	n = atoi(argv[1]);

	head = -1;
	tail = -1;
	insertions = 0;
	extractions = 0;

	pthread_t c_threads[n];
	pthread_t p_threads[n];

	int pinx[n];
	int cinx[n];
	pthread_cond_init(&qfull, NULL);
	pthread_cond_init(&qempty, NULL);
	pthread_mutex_init(&tlock, NULL);

	printf("Processing.....\n");
	//gettimeofday(&tz, &tx);
	//start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	for(i=0;i<n;i++){
		pinx[i] = i+1;
		cinx[i] = i+1;
		//printf("creating consumer and producer.....%d\n", i);
		pthread_create(&c_threads[i], &attr, consumer,(void *) &cinx[i]);
		pthread_create(&p_threads[i], &attr, producer,(void *) &pinx[i]);	
	}

	sleep(interval);
 	
 	for(i=0;i<n;i++){
		pthread_cancel(c_threads[i]);
		pthread_cancel(p_threads[i]);	
	}

	//gettimeofday(&tz, &tx);
	//end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;


	//printf("Elapsed time = %lf seconds\n", end_time - start_time);
	printf("No of insertions %d\n", insertions);
	printf("No of extractions %d\n", extractions);
	printf("Total %d\n", insertions+extractions);
	printf("Interval %d\n", interval);
	printf("Throughput %lf\n", (insertions+extractions)/(1.0*interval));
	
	return 0;
}