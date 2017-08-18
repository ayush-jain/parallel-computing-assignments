#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

int main(){
	int size = 750;
	int range = 1000;

	double start_time, end_time;
	double elapsed_time, bandwidth;	
	int i,j;

	struct timeval tz;
	struct timezone tx;
	
	float *arr[size];
	for (i=0;i<size;i++){
		arr[i] = (float *)malloc(size*sizeof(float));
	}

	float *vec;
	vec = (float *)malloc(size *sizeof(float));

	float *res;
	res = (float*)malloc(size*sizeof(float));
	
	for(i = 0; i < size; ++i)
	{
		vec[i] = rand() % range + 1;
		res[i] = 0;
		for(j = 0; j < size; ++j){
			arr[i][j] = rand() % range + 1;	
		}
	}

	gettimeofday(&tz, &tx);
	start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	for (j=0;j<size;j++){
		for(i=0;i<size;i++){
			res[i] += arr[i][j]*vec[j];
		}
	}

	gettimeofday(&tz, &tx);
	end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	elapsed_time = end_time - start_time;
	printf("Elapsed time = %lf seconds\n", elapsed_time);
	printf("Size %d\n", size);
	printf("FLOPS/s %lf\n", ((size)*2*size)/elapsed_time);	
	
	return 0;
}
