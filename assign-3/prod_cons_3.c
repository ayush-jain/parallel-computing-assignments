#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>


#define WORK_MSG 1
#define ACK 2
#define CSIG 3

//additional CSIG tag which is sent by producer to corresponding consumer to resume/end the consumption process based on ACK received/not.

int main(int argc, char ** argv)
{
  	int err;  
	int npes; 
	int rank;
	int tag;
	int ack;
	int resume;

	double start_time, end_time;
  	struct timeval tz;
  	struct timezone tx;

	int execution_time = 120; 

	int result = 0;
	int consumed_count = 0;

	int i,j,k,n,count;

	int num, flag;
	int inx = -1;
	
	MPI_Request req;
	MPI_Status status;


  	err = MPI_Init(NULL, NULL);

    err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    err = MPI_Comm_size(MPI_COMM_WORLD, &npes);
    
    //printf("Initializing %d...\n", rank);
    //fflush(stdout);
	
    if(rank == 0){
		gettimeofday(&tz, &tx);
        start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
  	}	
    
    while(1){
		
		if(rank == 0){

			// code that handles elapsed time check and sending of dummy signals at the end
			gettimeofday(&tz, &tx);
    		end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
    		double elapsed_time = end_time-start_time;
    		if((elapsed_time) >= ((1.0 *(execution_time)))){
        		int dummy = -99;
        		//printf("******************Time over.............******************************\n");
        		//fflush(stdout);
    			for(i=1;i<npes;i++){
    				
    				MPI_Send(&dummy, 1,MPI_INT, i, ACK, MPI_COMM_WORLD);
    				//printf("*******************Sending **dummy** val to %d by %d\n", i, rank);
    				//fflush(stdout);	
    				MPI_Send(&dummy, 1,MPI_INT, i, WORK_MSG, MPI_COMM_WORLD);
    				//printf("Sending **dummy** random number %d to %d by %d\n", dummy, i, rank);	
    				//fflush(stdout);
    				MPI_Send(&dummy, 1,MPI_INT, i, CSIG, MPI_COMM_WORLD);
    				//printf("Sending **dummy** random number %d to %d by %d\n", dummy, i, rank);	
    				//fflush(stdout);
    			}
        		break;
    		}	
		}
		
		//------------------------------------------------------------------

		//even including 0 is producer and next odd rank is corresponding consumer
		if(rank%2 ==0){
			num = rand();
    		inx = -1;
    		while(inx <= 0 || inx%2 == 0 || inx == (rank+1)){
    			inx = rand() % npes;
    		}
    		MPI_Send(&num, 1,MPI_INT, inx, WORK_MSG, MPI_COMM_WORLD);
    		//printf("Sending %d to process %d by process %d\n", num, inx, rank);
    		//fflush(stdout);

			resume = 1;
			MPI_Send(&resume, 1,MPI_INT, rank+1, CSIG, MPI_COMM_WORLD);
    		//printf("Sending resume to consumer process %d by process %d\n", rank+1, rank);
    		//fflush(stdout);			

			err = MPI_Irecv(&ack, 1, MPI_INT, MPI_ANY_SOURCE, ACK, MPI_COMM_WORLD, &req);
			err = MPI_Wait(&req, &status);
			
			if(ack == -99){
				//printf("time over for process %d\n", rank);
				//fflush(stdout);
				break;
			}
			if(status.MPI_SOURCE == inx){
				resume = 0;
				MPI_Send(&resume, 1,MPI_INT, rank+1, CSIG, MPI_COMM_WORLD);
    			//printf("Sending stop to consumer process %d by process %d\n", rank+1, rank);
    			//fflush(stdout);
			}
				
		} else {
			
			err = MPI_Irecv(&resume, 1, MPI_INT, MPI_ANY_SOURCE, CSIG, MPI_COMM_WORLD, &req);
			err = MPI_Wait(&req, &status);
			////printf("%d........for process %d...\n", status.MPI_TAG, rank);
			////fflush(stdout);
			if(resume == -99){
				//printf("time over for process %d\n", rank);
				//fflush(stdout);
				break;
			}

	 		if (status.MPI_SOURCE == (rank-1) && status.MPI_TAG == CSIG){
				while(1){
					status.MPI_TAG = -1;
					err = MPI_Irecv(&resume, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
					err = MPI_Wait(&req, &status);
					//printf("Status is %d....%d\n", status.MPI_TAG, resume);
					//fflush(stdout);
					if(resume == -99){
						//printf("time over for process %d\n", rank);
						//fflush(stdout);
						break;
					}
					if(status.MPI_TAG == WORK_MSG){
						//printf("Source is %d for process %d and number is %d\n", status.MPI_SOURCE, rank, resume);
						//fflush(stdout);
						num = -1;
						MPI_Send(&num, 1,MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);
						consumed_count++;			
						//printf("Consumed count for process %d is %d\n", rank, consumed_count);
						//fflush(stdout);	
					} else if (status.MPI_TAG == CSIG){
						//printf("Breaking consumption from %d\n", status.MPI_SOURCE);
						//fflush(stdout);
						break;
					}						
				}
			
				if(resume == -99){
					//printf("time over for process %d\n", rank);
					//fflush(stdout);
					break;
				}
			}								
		}

		if(resume == -99 || ack == -99){
			//printf("time over for process %d\n", rank);
			//fflush(stdout);
			break;
		}
		
			
	}
	
	//printf("Reaching the end....................%d\n", rank);
    MPI_Reduce(&consumed_count, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank == 0){
    	printf("\nConsumed items from all consumers are %d\n",result);
    	fflush(stdout);	
    }
    err = MPI_Finalize();
    return 0;
}
