#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

int main(){
	int size = 10000000;
	int range = 1000;
	float sum = 0;

	double start_time, end_time;
	double elapsed_time, bandwidth;	
	int i;

	struct timeval tz;
	struct timezone tx;
	
	float *arr;
	arr = (float *)malloc(size*4);
	for(i = 0; i < size; ++i)
	{
		//printf("hi..%d\n", i);
  		arr[i] = rand() % range + 1;
	}

	//printf("hi..%d\n", i);

	gettimeofday(&tz, &tx);
	start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	for (i=0;i<size;++i){
		sum = sum + arr[i];
	}

	//printf("hi..%d\n", i);

	gettimeofday(&tz, &tx);
	end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	elapsed_time = end_time - start_time;
	printf("Elapsed time = %lf seconds\n", elapsed_time);
	printf("Bytes Accessed %lu\n", sizeof(float)*size);
	printf("Bandwidth %lf\n", (sizeof(float)*size)/(elapsed_time*pow(2, 20)));	

	return 0;
}
