#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_THREADS	512

void *compute_pi (void *);

int total_hits, total_misses, hits[MAX_THREADS],
	sample_points, sample_points_per_thread, num_threads;


int main()
{
	int i;
	pthread_t p_threads[MAX_THREADS];
	pthread_attr_t  attr;
	double computed_pi;

	double start_time, end_time;
	struct timeval tz;
	struct timezone tx;

        pthread_attr_init (&attr);
        pthread_attr_setscope (&attr,PTHREAD_SCOPE_SYSTEM);

	printf("Enter number of sample points (darts) (use a number over a million): ");
	scanf("%d", &sample_points);
	printf("Enter number of threads (use a number between 1 and 512): ");
        scanf("%d", &num_threads);

	total_hits = 0;

	sample_points_per_thread = sample_points / num_threads;

	gettimeofday(&tz, &tx);
	start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	for (i=0; i< num_threads; i++)
	{
		hits[i] = i;
                // create threads here
		pthread_create(&p_threads[i], &attr, compute_pi,
			(void *) &hits[i]);
	}
	for (i=0; i< num_threads; i++)
        {
		// join threads and add up all the darts in the circle
		pthread_join(p_threads[i], NULL);
		total_hits += hits[i];
	}
	gettimeofday(&tz, &tx);
	end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	// pi is 4 times the fraction of darts
	computed_pi = 4.0*(double) total_hits /
		((double)(sample_points));

	
	printf("%d ", num_threads);
	printf("Computed PI = %lf\n", computed_pi);
	printf("Elapsed time = %lf seconds\n", end_time - start_time);
}

void *compute_pi (void *s)
{
	int thread_no, i, *hit_pointer;
	double rand_no_x, rand_no_y;
	int hits;

	hit_pointer = (int *) s;
	thread_no = *hit_pointer; // use this to randomize seed of random generator

	/* remember to randomize the seed here */

	srand48(thread_no);
	for (i = 0; i < sample_points_per_thread; i++)
	{
		rand_no_x = (double)(rand_r(&thread_no))/(double)(RAND_MAX);
		rand_no_y = (double)(rand_r(&thread_no))/(double)(RAND_MAX);

		if (((rand_no_x - 0.5) * (rand_no_x - 0.5) +
			(rand_no_y - 0.5) * (rand_no_y - 0.5)) < 0.25)
			(*hit_pointer) ++;
	}
}
