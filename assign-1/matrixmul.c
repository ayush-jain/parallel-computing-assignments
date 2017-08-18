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
	int i,j,k;

	struct timeval tz;
	struct timezone tx;
	
	float *arr1[size];
	float *arr2[size];
	float *res[size];
	for (i=0;i<size;i++){
		arr1[i] = (float *)malloc(size*sizeof(float));
		arr2[i] = (float *)malloc(size*sizeof(float));
		res[i] = (float *)malloc(size*sizeof(float));
	}

	for(i = 0; i < size; ++i)
	{
		for(j = 0; j < size; ++j){
			arr1[i][j] = rand() % range + 1;
			arr2[i][j] = rand() % range + 1;	
			res[i][j] = 0;
		}
	}

	gettimeofday(&tz, &tx);
	start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	for (i=0;i<size;i++){
		for(j=0;j<size;j++){
			for(k=0;k<size;k++){
				res[i][k] += arr1[i][j]*arr2[j][k]; 
			}
		}
	}

	gettimeofday(&tz, &tx);
	end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	elapsed_time = end_time - start_time;
	printf("Elapsed time = %lf seconds\n", elapsed_time);
	printf("Size %d\n", size);
	unsigned long ops = ((size)*2*size*size);
	printf("FLOPS/s %lf\n", ops/elapsed_time);	
	
	return 0;
}
