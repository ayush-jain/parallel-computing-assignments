#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int main(int argc, char** argv) {

  float data_buffer[1000000];
  int message_size, i;
  int ping_pong_limit;

  // data structures for timing
  double start_time, end_time;
  struct timeval tz;
  struct timezone tx;


  // Initialize the MPI environment
  MPI_Init(NULL, NULL);
  // Find out rank, size
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_rank == 0) {
    printf("Enter ping pong message size: ");
    fflush(stdout);
    scanf("%d", &message_size);
    if (message_size > 1000000) {
      fprintf(stderr, "Message size must be less than or equal to 1000000\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    printf("Enter ping pong limit: ");
    fflush(stdout);
    scanf("%d", &ping_pong_limit);
    if (ping_pong_limit > 10000) {
      fprintf(stderr, "Ping pong limit must be less than or equal to 10000\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (i = 0; i < message_size; i++)
      data_buffer[i] = i;
  }

  // broadcast message size to all processors
  MPI_Bcast((void *) &message_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // broadcast ping pong limit to all processors 
  MPI_Bcast((void *) &ping_pong_limit, 1, MPI_INT, 0, MPI_COMM_WORLD);


  // We are assuming 2 processes for this task
  if (world_size != 2) {
    fprintf(stderr, "World size must be two for %s\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  if (world_rank == 0) {
        gettimeofday(&tz, &tx);
        start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
  }


  int ping_pong_count = 0;
  int partner_rank = (world_rank + 1) % 2;
  while (ping_pong_count < ping_pong_limit) {
    if (world_rank == ping_pong_count % 2) {
      // Increment the ping pong count before you send it
      MPI_Send(&data_buffer, message_size, MPI_FLOAT, partner_rank, 0, MPI_COMM_WORLD);
      // printf("%d sent and incremented ping_pong_count %d to %d\n",
      //       world_rank, ping_pong_count, partner_rank);
    } else {
      MPI_Recv(&data_buffer, message_size, MPI_FLOAT, partner_rank, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      // printf("%d received ping_pong_count %d from %d\n",
      //        world_rank, ping_pong_count, partner_rank);
    }
      ping_pong_count++;
  }

  if (world_rank == 0) {
        gettimeofday(&tz, &tx);
        end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
        printf("Elapsed time = %lf seconds\n", end_time - start_time);
        printf("Elapsed time = %lf ms\n", ((end_time - start_time)*1000)/((2.0*(double)ping_pong_limit)));
		double time = ((end_time - start_time))/((2.0*(double)ping_pong_limit));        
		printf("Elapsed time = %lf micros\n", time*1000000);
		printf("Bandwidth in bytes/second %lf\n", (message_size*4.0)/time);
		printf("Per word transfer rate %lf in nanoseconds\n", time*1000000000/(message_size));
  }

  MPI_Finalize();
}
