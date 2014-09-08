/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define PACK_LEN 1500
#define DATA_LEN 1400

#include "packetHeader.h"
//for track seq

#define MAXSEQNO 748983

typedef struct dataSt{
        int nakNo;
        struct dataSt *next;
        struct dataSt *back;
        }dataSt;

dataSt *head = NULL;
dataSt *cur = NULL;

int newSeqNo;
int oldSeqNo = 1;
dataSt *a[MAXSEQNO]={NULL};
//For track sequence

//Function for track and update

void updateNackList(int sequenceNo);

//void *trackseq(void *args)
void trackseq(){
    int i;
    dataSt *temp=NULL;
    usleep(1);
    for(i = oldSeqNo; i <= newSeqNo; i++){

        if(a[i] ==NULL){
            //printf("Haed value: %d\n\r\t",head);
            if(head == NULL){
                //printf("Updating head: %d\n",i);
                head = (dataSt *)malloc(sizeof(dataSt));
               // printf("head : %u\n",head);
                a[i] = head;
                //printf("a[i] : %u\n",a[i]);
                head->nakNo = i;
                //printf("head stored : %d\n",head->nakNo);
                head->next = (dataSt *)cur;
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
    //printf("updating sequence %d\n ",sequenceNo);
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

                } else{
                    temp1=temp->back;
                    temp1->next=temp->next;
                    temp2=temp->next;
                    temp2->back=temp->back;
                    free(temp);
                }
            }
            //update the link list

            a[sequenceNo]=(dataSt *)1;

    } else{
        printf("Setting sequenceNo  : %d to 1\n",sequenceNo );
        a[sequenceNo]=(dataSt *)1;
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
//Function for track and sequence


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     customPacket *packet;
     int sockfd, newsockfd, portno;
     socklen_t fromlen;
     unsigned char buffer[PACK_LEN];
     struct sockaddr_in serv_addr, from;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");

    fromlen = sizeof(struct sockaddr_in);
    //Allocate the memory for packet
    packet=(customPacket* )malloc(sizeof(customPacket));
   // FILE * fpg=fopen("copyGB.bin", "w");
    newSeqNo=1;
    int runsequenceTracker=100;
     while (1) {
         n = recvfrom(sockfd,buffer,PACK_LEN,0,(struct sockaddr *)&from,&fromlen);
       
         packet=(customPacket*)buffer;
         //printf("Packet sequenceNumber : %d\n packet Length: %d\n",packet->sequenceNo,packet->len);
         //logic to store the sequence number.
         //printf("%s\n", packet->data);
         //Store and update the sequence.
         updateNackList(packet->sequenceNo);

         if(packet->sequenceNo >runsequenceTracker){
            printf("calling track sequence\n");
            oldSeqNo = newSeqNo;
            newSeqNo =runsequenceTracker+1;
            trackseq();
            printLinklist( head);
            runsequenceTracker+=100;
            continue;
            
         }

        // fwrite(packet->data, 1400,1,fpg);
        //fflush(fpg);
         //
         if (n < 0)
            error("recvfrom");
/*          n = sendto(sockfd,"Got your message ",17, 0,(struct sockaddr *) &from,fromlen);
          if (n < 0)
             error("sendto"); */   

     }

  exit(0); 
}
