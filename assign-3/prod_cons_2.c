#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>


#define WORK_MSG 1
#define ACK 2




int main(int argc, char ** argv)
{
  	int err;  
	int npes; 
	int rank;
	int tag;
	int ack;

	double start_time, end_time;
  	struct timeval tz;
  	struct timezone tx;

	int execution_time = 120; 

	int result = 0;
	int consumed_count = 0;

	int i,j,k,n,count;

	int num, flag, flag_1;
	int inx = -1;
	
	MPI_Request req, req_1;
	MPI_Status status, status_1;


  	err = MPI_Init(NULL, NULL);

    err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    err = MPI_Comm_size(MPI_COMM_WORLD, &npes);
    
	
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
    				
    				//printf("*******************Sending **dummy** val to %d by %d\n", i, rank);
    				//fflush(stdout);	
    				MPI_Send(&dummy, 1,MPI_INT, i, WORK_MSG, MPI_COMM_WORLD);
    				//printf("Sending **dummy** random number %d to %d by %d\n", dummy, i, rank);	
    				//fflush(stdout);
    				
                    //printf("*******************Sending **dummy** val to %d by %d\n", i, rank);
                    //fflush(stdout); 
                    MPI_Send(&dummy, 1,MPI_INT, i, ACK, MPI_COMM_WORLD);
                    //printf("Sending **dummy** random number %d to %d by %d\n", dummy, i, rank); 
                    //fflush(stdout);
    			}
        		break;
    		}	
		}
		

        //start of producer/consumer code
    	num = rand();
    	inx = -1;
    	while(inx == -1 || inx == rank){
    		inx = rand() % npes;
    	}
    	MPI_Send(&num, 1,MPI_INT, inx, WORK_MSG, MPI_COMM_WORLD);
    	//printf("Sending %d to process %d by process %d\n", num, inx, rank);
    	//fflush(stdout);
    	
		flag = 0;
		i=0;
		while(1){
            flag = 0;    
            status.MPI_TAG = -1;
		    err = MPI_Irecv(&ack, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
		    //printf("Hey....\n");
            //fflush(stdout);
            err = MPI_Test(&req, &flag, &status);
            if(flag != 0){
				if(ack == -99 || num == -99){
                	//printf("time over for process %d\n", rank);
                	//fflush(stdout);
                	break;
            	}
				//printf("yo...........\n");
                if(status.MPI_TAG == ACK){
                    //printf("ACKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKReceiving ack %d by process %d\n", ack, rank);
                    //fflush(stdout);
                    if(status.MPI_SOURCE != -1){
                        //printf("Ack Source is %d for process %d\n", status.MPI_SOURCE, rank);                   
                        //fflush(stdout);
                    }
					break;   
                } else if (status.MPI_TAG == WORK_MSG){
                    //printf("WORRRRRRRRRRRRRRRRRRRRRRRRRRRSource is %d for process %d and number is %d\n", status.MPI_SOURCE, rank, num);
                    //fflush(stdout);
                    num = -1;
                    MPI_Send(&num, 1,MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);
                    consumed_count++;           
                    //printf("Consumed count for process %d is %d\n", rank, consumed_count);
                    //fflush(stdout);
					continue;                
				}   
            }
            MPI_Cancel(&req);
            MPI_Request_free(&req);
        }
		//printf("%d........for process %d...\n", status.MPI_TAG, rank);
		//fflush(stdout);

		if(ack == -99 || num == -99){
			//printf("hhhhhhhyo...........\n");
			break;
		}		
    }
  	
    MPI_Reduce(&consumed_count, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank == 0){
    	printf("\nConsumed items from all consumers are %d\n",result);
    	fflush(stdout);	
    }
    err = MPI_Finalize();
    return 0;
}
