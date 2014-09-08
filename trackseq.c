#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/time.h>

#define MAXSEQNO 748983

typedef struct{
        int nakNo;
        struct dataSt *next;
        struct dataSt *back;
        }dataSt;

dataSt *head = NULL;
dataSt *cur = NULL;

int newSeqNo;
int oldSeqNo = 1;
dataSt *a[MAXSEQNO]={NULL};

void *trackseq(void *args){
    int i;
    dataSt *temp=NULL;
    usleep(100);
    for(i = oldSeqNo; i <= newSeqNo; i++){

        if(a[i] ==NULL){
            //printf("Haed value: %d\n\r\t",head);
            if(head == NULL){
                //printf("Updating head: %d\n",i);
                head = (dataSt *)malloc(sizeof(dataSt));
                a[i] = head;
                head->nakNo = i;
                //printf("head stored : %d\n",head->nakNo);
                head->next = cur;
                head->back = NULL;
                cur = head;
            }
        
        else{
            //printf("adding after head %d\n",i );
            temp = (dataSt *)malloc(sizeof(dataSt));
            a[i] = temp;
            cur->next = temp;
            temp->nakNo = i;
            temp->back = cur;
            temp->next = NULL;
            cur = temp;
        }
        oldSeqNo = newSeqNo;
    }
  }
 }   

 void printLinklist(dataSt *head){
    dataSt *temp=NULL;
    temp=head;
    //printf("%d\n", temp->nakNo);
    while(temp!=NULL){
        printf("%d \n",temp->nakNo);
        temp= temp->next;
    }
 
 }

int main(int argc, char* argv[]){

    pthread_t seqtrack;
    int i=1;
    for(i;i<=10;i+=2){
        a[i] = 1;
    }
    for(i=1;i<=10;i++){
        printf("%d\n",a[i]);
    }
    printf("Done copying\n");
    newSeqNo=10;
    printf("Executing thread");
    pthread_create(&seqtrack,0,trackseq,0);
    pthread_join(seqtrack,0);
    printLinklist(head);

    return 0;
}
 
