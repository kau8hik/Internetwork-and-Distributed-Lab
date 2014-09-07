#include <stdio.h>
#include <stdlib.h>

#define MAXSEQNO 748983

typedef struct{
        int nakNo;
        dataSt *next;
        dataSt *back;
        }dataSt;
dataSt *head = NULL;
dataSt *cur;

int newSeqNo;
int oldSeqNo = 0;
int a[MAXSEQNO] = 0;

void trackseq(int seqNo){
     int i = 0;
     newSeqNo = seqNo;
     dataSt *temp;
     while(1){
              if(seqNo < MAXSEQNO){
                       a[seqNo] = 1;
                       }
              for(i = oldSeqNo; i < newSeqNo; i++){
                    if(a[i] == 0){
                            if(head == NULL){
                                    head = (dataSt *)malloc(sizeof(dataSt));
                                    a[i] = head;
                                    head->nakNo = i;
                                    head->next = NULL;
                                    head->back = NULL;
                                    cur = head;
                                    }
                            }
                            else{
                                 temp = (dataSt *)malloc(sizeof(dataSt));
                                 a[i] = temp;
                                 cur->next = temp;
                                 cur->nakNo = i;
                                 temp->prev = cur;
                                 temp->next = NULL;
                                 cur = temp;
                                 }
                                 oldSeqNo = newSeqNo;
                    }
                                 
