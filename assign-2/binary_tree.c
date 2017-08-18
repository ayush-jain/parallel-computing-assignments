#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define SIZE 255

struct btree;
typedef struct btree *binary_tree;

pthread_rwlock_t tlock[SIZE+1];

int n;
int insertions, extractions;

struct btree{
	int val;
	binary_tree left;
	binary_tree right;
};

struct tree{
	binary_tree root;
};

struct tree *start;

binary_tree create_node(int val){
	binary_tree node = (binary_tree)malloc(sizeof(struct btree));
	node->val = val;
	node->left = NULL;
	node->right = NULL;
	return node;
}

int insertInTree(binary_tree node, int val, int inx){
	
	binary_tree next;
	pthread_rwlock_wrlock(&tlock[node->val]);
	if(val > node->val){
		if(node->right == NULL){
			node->right = create_node(val);
			pthread_rwlock_unlock(&tlock[node->val]);
			//printf("Inserting %d by thread %d \n", val, inx);
			return 1;
		} else {
			next = node->right;
			pthread_rwlock_unlock(&tlock[node->val]);
			return insertInTree(next, val, inx);
		}
	} else {
		if(node->left == NULL){
			node->left = create_node(val);
			pthread_rwlock_unlock(&tlock[node->val]);
			//printf("Inserting %d by thread %d \n", val, inx);
			return 1;
		} else {
			next = node->left;
			pthread_rwlock_unlock(&tlock[node->val]);
			return insertInTree(next, val, inx);
		}
	}
}

void *insert(void *pdata) {
	int inserted;
	int val,i = 0;
	int inx = *((int *)pdata);
	//
	//printf("%d....pinxxxx\n", inx);
	while (1) {
		val = inx + (i*n);
		//printf("val is....%d\n", val);
		if(val > 255){
			break;
		}
		inserted = 0;

		pthread_rwlock_wrlock(&tlock[0]);
		if(start->root == NULL){
			start->root = create_node(val);
			//printf("Inserting first node with value %d by thread %d\n", val, inx);
			inserted = 1;
		}
		binary_tree st = start->root;
		pthread_rwlock_unlock(&tlock[0]);
		while(inserted == 0){
			inserted = insertInTree(st, val, inx);
			//printf("yoxxxx  %d....\n", inserted);	
		}
		//printf("yo....\n");
		i++;
	}
	//printf("insertion for thread %d exceeds 255\n", inx);
}

int lookInTree(binary_tree node, int val, int inx){
	
	binary_tree next;
	pthread_rwlock_rdlock(&tlock[node->val]);
	if(val == node->val){
		//printf("Lookup for val %d by thread %d successful\n", val, inx);
		pthread_rwlock_unlock(&tlock[node->val]);
		return 1;
	}
	else if(val > node->val){
		if(node->right == NULL){
			pthread_rwlock_unlock(&tlock[node->val]);
			return 0;
		} else {
			next = node->right;
			pthread_rwlock_unlock(&tlock[node->val]);
			return lookInTree(next, val, inx);
		}
	} else {
		if(node->left == NULL){
			return 0;
		} else {
			next = node->left;
			pthread_rwlock_unlock(&tlock[node->val]);
			return lookInTree(next, val, inx);
		}
	}
}

void *lookup(void *pdata) {
	int found;
	int val,i = 0;
	int inx = *((int *)pdata);
	//printf("%d....linxxxx\n", inx);
	while (1) {
		val = inx + (i*n);
		//printf("val is....%d\n", val);
		if(val > 255){
			break;
		}
		found = 0;

		pthread_rwlock_rdlock(&tlock[0]);
		if(start->root == NULL){
			return;
		}
		binary_tree st = start->root;
		pthread_rwlock_unlock(&tlock[0]);
		while(found == 0){
			found = lookInTree(st, val, inx);
			//printf("yoxxxx  %d....\n", inserted);	
		}
		//printf("yo....\n");
		i++;
	}
	//printf("lookup for thread %d exceeds 255\n", inx);
}

void *printTree(binary_tree node){
	if(node == NULL){
		return;
	}
	printTree(node->left);
	printf("Node value is %d\n", node->val);
	printTree(node->right);
}


int main(int argc, char *argv[]){

	int i;
	pthread_attr_t  attr;

	double start_time, end_time;
	struct timeval tz;
	struct timezone tx;

	double start_time_1, end_time_1;
	struct timeval tz_1;
	struct timezone tx_1;

	pthread_attr_init (&attr);
	pthread_attr_setscope (&attr,PTHREAD_SCOPE_SYSTEM);

	n = atoi(argv[1]);

	pthread_t p_threads[n];

	int pinx[n];
	
	for(i=0;i<SIZE;i++){
		pthread_rwlock_init(&tlock[i], NULL);	
	}

	start = (struct tree *)malloc(sizeof(struct tree));
	
	gettimeofday(&tz, &tx);
	start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	for(i=0;i<n;i++){
		pinx[i] = i+1;
		//printf("creating insertion threads.....%d\n", i+1);
		pthread_create(&p_threads[i], &attr, insert,(void *) &pinx[i]);	
	}
 	
 	for(i=0;i<n;i++){
		pthread_join(p_threads[i], NULL);	
	}

	gettimeofday(&tz, &tx);
	end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;

	//printTree(start->root);
	
	printf("Elapsed time = %lf seconds\n", end_time - start_time);
	printf("No of insertions %d\n", SIZE);
	
	//printf("No of extractions %d\n", SIZE);
	
	printf("Insertion Throughput in nodes/s.....%lf\n", SIZE/((end_time - start_time)));

	//sleep(5);
	//------------------------------------------------------------------------------------

	pthread_t l_threads[n];

	int linx[n];
	
	for(i=0;i<SIZE;i++){
		pthread_rwlock_init(&tlock[i], NULL);	
	}
	
	gettimeofday(&tz_1, &tx_1);
	start_time_1 = (double)tz_1.tv_sec + (double) tz_1.tv_usec / 1000000.0;

	for(i=0;i<n;i++){
		linx[i] = i+1;
		//printf("creating lookup threads.....%d\n", i+1);
		pthread_create(&l_threads[i], &attr, lookup,(void *) &linx[i]);	
	}
 	
 	for(i=0;i<n;i++){
		pthread_join(l_threads[i], NULL);	
	}

	gettimeofday(&tz_1, &tx_1);
	end_time_1 = (double)tz_1.tv_sec + (double) tz_1.tv_usec / 1000000.0;
	
	printf("Elapsed time = %lf seconds\n", end_time_1 - start_time_1);
	
	printf("No of lookups %d\n", SIZE);
	
	printf("lookups Throughput in nodes/s.....%lf\n", SIZE/((end_time_1 - start_time_1)));
	

	return 0;
}