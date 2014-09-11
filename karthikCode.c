#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;

int a[100] = {0};
int i = 0;

typedef struct data{
	int num;
	struct data *next;
}data;

void print_list(data* h){
	data* temp1 = h;
	while(temp1 != NULL){
		printf("%d ", temp1 -> num);
		temp1 = temp1->next;
	}
	printf("\n");
}

data *head = NULL;
data *first=NULL;
pthread_t readthread;
int condtionFlag=0;
int old=2, new;
void *updateSeq(void *args){


	printf("UPDATE SEQUENCE STARTED\n");
	usleep(10);
	data *temp = NULL;
	data *tempnext = NULL;
	data *temp1 = NULL;
	data *cur = NULL;
	temp = (data*)malloc(sizeof(data));
	int k = 0;
	while(1){
		//check for the condition 
		if(condtionFlag){
			printf("send NACK Packet list\n");
			condtionFlag=0;
			printf("old Seq: %d new seq : %d\n",old,new);
	        for(i = old; i < new; i++){
		   	    if(head == NULL && a[i] == i){
					printf("INITIALIZING NACK LIST: %d\n",i);
					head = (data*)malloc(sizeof(data));
					head -> num = i;
					head ->next = NULL;
					cur = head;
					first=head;
				}
				else{
					if(a[i] == i){
						printf("APPENDING TO NACK LIST : %d\t", i);
						temp=(data*)malloc(sizeof(data));
						cur -> next = temp;
						temp -> num = i;
						temp -> next = NULL;
						cur = temp;
					}
				}
				old =new;

			//printf("\n");
			print_list(first);
			}
		}
		//if true send message

	
		temp = head;
			while(temp != NULL){
				if(a[temp->num]==1){
					tempnext = temp -> next;
					temp -> num = tempnext -> num;
					temp -> next = tempnext -> next;
					free(tempnext);
					
				}
				temp = temp -> next;
				//printf("Printing changed link list: %d\n", k);
				
			}

		k += 1;
		usleep(10);
	}

	return 0;
}

int main(){
	int j = 0;
	for(i = 0; i < 100; i++){
		if(i % 2 == 0) a[i] = i;
		else a[i] = 1;
	}
	printf("Printing initial array\n");
	for(i = 0; i < 100; i++){
		printf("[%d] ", a[i]);
	}
	printf("\n");
	//pthread_create(&readthread, 0, updateSeq, 0);
		for(i = 0; i < 100; i+=4){
			a[i] = 1;
		}
	
    pthread_create(&readthread, 0, updateSeq, 0);	
	//pthread_join(readthread,0);
	//pthread_kill(readthread,0);
	usleep(100);
	condtionFlag=1;
	new=50;
    	//signal thread to come up
		//pthread_cond_signal(*cond);

		printf("Printing changed array 1\n");
		for(i = 0; i < 100; i++){
			printf("[%d]", a[i]);
		}
		for(i = 0; i < 100; i+=8){
			a[i] = 1;
		}
		printf("Printing changed array 2\n");
		for(i = 0; i < 100; i++){
			printf("[%d]", a[i]);
		}
		for(i = 0; i < 100; i+=16){
			a[i] = 1;
		}
		printf("Printing changed array 3\n");
		for(i = 0; i < 100; i++){
			printf("[%d]", a[i]);
		}
	
	condtionFlag=1;
	new=100;
	pthread_join(readthread,0);

	printf("READ JOINED\n");
	return 0;	
}
