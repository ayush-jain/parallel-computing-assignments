#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define SIZE 1000
#define interval 10

pthread_rwlock_t qlock;

int queue[SIZE];
int head;
int n;
int insertions, extractions;

void *consumer(void *cdata) {
	int extracted;
	int inx = *((int *)cdata);
	//printf("%d....cinxxxx\n", inx);
	while (1) {
		extracted = 0;
		pthread_rwlock_rdlock(&qlock);
		if(head != -1){
			pthread_rwlock_unlock(&qlock);
			while(extracted == 0){
				pthread_rwlock_wrlock(&qlock);
				if(head != -1){
					//printf("Extracted %d by consumer %d at head %d\n", queue[head], inx, head);
					queue[head--] = -1;
					extracted = 1;	
					extractions++;	
				} else {
					pthread_rwlock_unlock(&qlock);
				}
			}
			pthread_rwlock_unlock(&qlock);			
		} else {
			pthread_rwlock_unlock(&qlock);
		}
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
		pthread_rwlock_rdlock(&qlock);
		if (head != SIZE-1){ 
			pthread_rwlock_unlock(&qlock);
			while(inserted == 0){
				pthread_rwlock_wrlock(&qlock);		
				if(head != SIZE-1){
					queue[head+1] = val;
					//printf("Inserted %d by producer %d at head%d\n", queue[head+1], inx, head+1);
					head++;
					inserted = 1;
					i++;
					insertions++;
				} else {
					pthread_rwlock_unlock(&qlock);		
				}
			}
			pthread_rwlock_unlock(&qlock);			
		} else {
			pthread_rwlock_unlock(&qlock);
		}
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
	insertions = 0;
	extractions = 0;

	pthread_t c_threads[n];
	pthread_t p_threads[n];

	int pinx[n];
	int cinx[n];
	pthread_rwlock_init(&qlock, NULL);
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