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
    usleep(1);
    for(i = oldSeqNo; i <= newSeqNo; i++){

        if(a[i] ==NULL){
            //printf("Haed value: %d\n\r\t",head);
            if(head == NULL){
                //printf("Updating head: %d\n",i);
                head = (dataSt *)malloc(sizeof(dataSt));
                printf("head : %u\n",head);
                a[i] = head;
                printf("a[i] : %u\n",a[i]);
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

//update the list
void updateNackList(int sequenceNo){
    dataSt *temp, *temp1, *temp2;
    //check for the a[sequence]

    int ttype = (a[sequenceNo]!=(dataSt *)1);
    int ptype= (a[sequenceNo]!=NULL);
    if(a[sequenceNo]!=NULL && a[sequenceNo]!=(dataSt *)1){
            //address is stored!
            temp=a[sequenceNo];
            if(temp==head){
                head=head->next;
                free(temp);
            } else{
                if(temp->next==NULL){
                    //last packet
                    temp1=temp->back;
                    temp1->next=NULL;
                    free(temp);

                }
                temp1=temp->back;
                temp1->next=temp->next;
                temp2=temp->next;
                temp2->back=temp->back;
                free(temp);
            }
            //update the link list

            a[sequenceNo]=1;

    } else{
        a[sequenceNo]=1;
    }
        

}



 void printLinklist(dataSt *head){
    dataSt *temp=NULL;
    temp=head;
    //printf("%d\n", temp->nakNo);
    while(temp!=NULL){
        printf("Temp nak no :%d\t add of link list nodes: %d \n",temp->nakNo, temp);
        temp= temp->next;
    }
 
 }
//main function
int main(int argc, char* argv[]){

    pthread_t seqtrack;
    int i=1;
    for(i;i<=10;i+=2){
        a[i] = (dataSt*)1;
    }
    for(i=1;i<=10;i++){
        printf("%d\n",a[i]);
    }
    printf("Done copying\n");
    newSeqNo=10;
    printf("Executing thread");
    pthread_create(&seqtrack,0,trackseq,0);

  
    for(i=1;i<=10;i++){
        printf("a[i] : %u\n",a[i]);
    }

    pthread_join(seqtrack,0);

   // printLinklist(head);
    //printf("%u\n",head);
      /*for(i=2;i<=8;i+=2){
        a[i]=(dataSt*)malloc(sizeof(dataSt));
    }*/

    for(i=1;i<=4;i++){
        printf("before : %d\n",a[i] );
        updateNackList(i);
        printf("After: %d\n",a[i] );
    }
    printLinklist(head);
    return 0;
}
