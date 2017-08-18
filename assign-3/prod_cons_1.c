#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>

#define REQ_WORK 0
#define WORK_MSG 1
#define ACK 2

int calculate_queue_size(int *head, int *tail, int BUF_SIZE) {
	int current_size = 0;
	if ((*head)==(*tail) && (*head) !=-1) {
		current_size = 1;		
		// queue full
	} else if ((*head)==(*tail) && (*head)==-1) {
		current_size = 0;		
		// queue empty
	} else if ((*tail)<(*head)) {
		current_size = (*tail) + 1 + (BUF_SIZE - (*head));   
	} else if ((*tail)>(*head)) {
		current_size = ((*tail)-(*head)) + 1;
	}
	return current_size;
}

void insert(int *queue, int num, int *bufsize, int *head, int *tail, int BUF_SIZE){
	//printf("Insertion of number %d happening at bufsize...%d\n", num, *bufsize);
	if(*bufsize == BUF_SIZE){
		return;
	}
	queue[((*tail)+1)%BUF_SIZE] = num;
	(*tail) = ((*tail)+1)%BUF_SIZE;
	(*bufsize) = (*bufsize) + 1;
	if((*bufsize) == 1){
		*head = *tail;
	}
}

int delete(int *queue, int *bufsize, int *head, int *tail, int BUF_SIZE){
	//printf("Deletion Happening at bufsize %d..\n", *bufsize);
	int res;
	if((*bufsize) == 0){
		return -1;
	}
	res = queue[*head];
	queue[*head] = -1;
	*head = ((*head)+1)%BUF_SIZE;
	(*bufsize) = (*bufsize) - 1;
	if(*bufsize == 0){
		*head = -1;
		*tail = -1;
	}
	return res;
}

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

	int pcount;
	int ccount;
	int num;
	
	MPI_Request req;
	MPI_Status status;


  	err = MPI_Init(NULL, NULL);

    err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    err = MPI_Comm_size(MPI_COMM_WORLD, &npes);

    pcount = npes/2;
    ccount = npes-(pcount+1);

    if(rank == 0){

    	int BUF_SIZE = 2*pcount;
    	int *queue = (int *)malloc(BUF_SIZE*sizeof(int));
    	int bufsize = 0;
		int head = -1; 
		int tail = -1;

		gettimeofday(&tz, &tx);
        start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
  		

    	while(1){

            // code that handles elapsed time check and sending of dummy signals at the end
    		gettimeofday(&tz, &tx);
        	end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
        	double elapsed_time = end_time-start_time;
        	if((elapsed_time) >= ((1.0 *(execution_time)))){
        		int dummy = -99;
        		//printf("Time over.............\n");
        		//fflush(stdout);
    			for(i=1;i<npes;i++){
    				if(i<=pcount){
    					//printf("Sending **dummy** val to %d by %d\n", i, rank);
    					//fflush(stdout);
    					MPI_Send(&dummy, 1,MPI_INT, i, ACK, MPI_COMM_WORLD);
    				} else {
    					MPI_Send(&dummy, 1,MPI_INT, i, WORK_MSG, MPI_COMM_WORLD);
    					//printf("Sending **dummy** random number %d to %d by %d\n", dummy, i, rank);	
    					//fflush(stdout);
    				}
    			}
        		break;
        	}



            //start of broker code
    		bufsize = calculate_queue_size(&head, &tail, BUF_SIZE);
    		//printf("Buffer size is %d\n", bufsize);
    		//fflush(stdout);
    		if(bufsize >0  && bufsize < BUF_SIZE){
    			err = MPI_Recv(&num, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    			if(status.MPI_TAG == WORK_MSG){
    				insert(queue, num, &bufsize, &head, &tail, BUF_SIZE);
    				int src = status.MPI_SOURCE;
    				ack = -1;
    				//printf("Sending ack to %d by %d\n", src, rank);
    				//fflush(stdout);
    				MPI_Send(&ack, 1,MPI_INT, src, ACK, MPI_COMM_WORLD);	
    			} else if(status.MPI_TAG == REQ_WORK){
    				int elem = delete(queue, &bufsize, &head, &tail, BUF_SIZE);
    				int src = status.MPI_SOURCE;
    				MPI_Send(&elem, 1,MPI_INT, src, WORK_MSG, MPI_COMM_WORLD);
    				//printf("Sending random number %d to %d by %d\n", elem, src, rank);	
    				//fflush(stdout);
    			}	
    		}
    		else if(bufsize == 0){
    			err = MPI_Recv(&num, 1, MPI_INT, MPI_ANY_SOURCE, WORK_MSG, MPI_COMM_WORLD, &status);
    			insert(queue, num, &bufsize, &head, &tail, BUF_SIZE);
				int src = status.MPI_SOURCE;
				ack = -1;
				//printf("BBBB--Sending ack to %d by %d\n", src, rank);
				//fflush(stdout);
				MPI_Send(&ack, 1,MPI_INT, src, ACK, MPI_COMM_WORLD);		  			
    		} else if(bufsize == BUF_SIZE) {
    			err = MPI_Recv(&num, 1, MPI_INT, MPI_ANY_SOURCE, REQ_WORK, MPI_COMM_WORLD, &status);
    			int elem = delete(queue, &bufsize, &head, &tail, BUF_SIZE);
    			int src = status.MPI_SOURCE;
    			MPI_Send(&elem, 1,MPI_INT, src, WORK_MSG, MPI_COMM_WORLD);
    			//printf("NNNN--Sending random number %d to %d by %d\n", elem, src, rank);
    			//fflush(stdout);
    		}
    	}

    	free(queue);    	
    } else if (rank>0 && rank <=pcount){
    	
        //start of producers' code
    	while(1){

    		
    		num = rand();
    		//printf("Random number generated by process %d is %d\n", rank, num);
    		//fflush(stdout);
    		err = MPI_Send(&num, 1,MPI_INT, 0, WORK_MSG, MPI_COMM_WORLD);
			MPI_Recv(&ack, 1, MPI_INT, 0, ACK, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			//printf("Ack recieved by process %d....\n", rank);
    		//fflush(stdout);
    		if(ack == -99){
    			//printf("time over for producer %d\n\n", rank);
    			//fflush(stdout);
    			break;
    		}
    	}

    } else if (rank > pcount && rank <npes) {
    	   
        //start of consumers' code
        consumed_count = 0;
    	while(1){
			    		
    		num = -1;
    		err = MPI_Send(&num, 1,MPI_INT, 0, REQ_WORK, MPI_COMM_WORLD);
    		//printf("Requesting work by process %d\n", rank);
    		//fflush(stdout);
    			
            int flag;     
    		while(1)
            {   
                flag = 0;
                err = MPI_Irecv(&num, 1, MPI_INT, 0, WORK_MSG, MPI_COMM_WORLD, &req);
			    //sleep((1.0)/100000);
                //printf("Hey.....\n");
			    //fflush(stdout);
			    err = MPI_Test(&req, &flag, &status);
                if(flag != 0){
                    break;
                }
                MPI_Cancel(&req);
                MPI_Request_free(&req);
            }
    		if(num == -99){
    			//printf("time over for consumer %d\n\n", rank);
				//fflush(stdout);
				break;
    		}
    		//printf("\n\nReceiving successful yoooooo ***********%d..%d\n", num, rank);
    		//fflush(stdout);	
    	
			consumed_count++;
			//printf("*****Consumed Count is now %d.. for process %d\n", consumed_count, rank);	
			//fflush(stdout);
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

