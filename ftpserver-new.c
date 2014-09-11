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
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PACK_LEN 1500
#define DATA_LEN 1400

#include "packetHeader.h"
//for track seq

#define MAXSEQNO 748983


seqACK *nackArray=NULL;
int fileSize;

typedef struct dataSt{
        int nakNo;
        struct dataSt *next;
        struct dataSt *back;
        }dataSt;

dataSt *head = NULL;
dataSt *cur = NULL;
dataSt *nackTraversalPoint = NULL;

int newSeqNo;
int oldSeqNo = 1;
dataSt *a[MAXSEQNO]={NULL};


nackFile *filesizefromclient;

char *map;  /* mmapped array of int's */


//For track sequence

//Function for track and update

void updateNackList(int sequenceNo);

//void *trackseq(void *args)
void trackseq(){
    int i;
    dataSt *temp=NULL;
    usleep(1);
    printf("Track Sequence called with oldSeqNo : %d\t newSeqNo : %d\n",oldSeqNo,newSeqNo);
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
                nackTraversalPoint=head;
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
        //printf("Temp nak no :%d\t add of link list nodes: %d \n",temp->nakNo, temp);
        temp= temp->next;
    }
 
 }

 void returnNackArrayList(dataSt *head, seqACK *getNackArry){
        dataSt *temp=NULL;
        temp=head;
        int i=0;
        memset(getNackArry->seqNo,0,350);
        while(i<349 && temp!=NULL){
            getNackArry->seqNo[i]=temp->nakNo;
            temp=temp->next;
            i++;
        }
        getNackArry->length=i;
       // printf("assigning nackTraversalPoint address: %d\n",temp);
        //nackTraversalPoint=temp;



 }
//Function for track and sequence


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     FILE* testfd;
     testfd = fopen("newrfile.c","w+");
     int result, maxSequenceNumber=0, tempSequence,maxSequenceCalulated;
     int writefd;
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
    int runsequenceTracker=350;

    //assign memory to nack array
    //seqACK *nackArray
    nackArray= (seqACK*)malloc(sizeof(seqACK));

    //int flagTest =1;
   
    writefd = open("fileReceived.c", O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (writefd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    //for filesize;
    filesizefromclient =(nackFile*)malloc(sizeof(nackFile));
    //to set the mmap size and do it only once 
    int writeToFileFlag=0;
    int yes=1;
    int dataFromClient, addressToAdd=0;
    //int testflag=1;
     while (1) {
         n = recvfrom(sockfd,buffer,PACK_LEN,0,(struct sockaddr *)&from,&fromlen);

         //check buffer data lenth for n==4
         //Two cases:
         //1. fileSzie==-1 [query for nack]
         //2. fileSize>0 [info about file size]
         if(n==4){

              filesizefromclient=(nackFile*)buffer;
              dataFromClient=filesizefromclient->data;

              //fileSize =2097152;
              //check two condition
              if(dataFromClient==-1){
                    //get the packet
                    printf("calling track sequence\n");
                    oldSeqNo = newSeqNo;
                    newSeqNo =maxSequenceNumber;
                    if(oldSeqNo==newSeqNo){
                      if(newSeqNo<maxSequenceCalulated){
                            newSeqNo+=1000;
                            if(newSeqNo>maxSequenceCalulated)
                                newSeqNo=maxSequenceCalulated;

                      }
                              
                    }
                    trackseq();

     
                    returnNackArrayList(head, nackArray);
                    //print the nack list
                    int j=0;
                    for(j=0;j<nackArray->length;j++){
                        printf("%d\t",nackArray->seqNo[j]);
                    }
                   
                    //    break;
                   // printf("Max sequence yet is: %d\n", maxSequenceNumber );
                    if(head==NULL && maxSequenceNumber==(maxSequenceCalulated)){
                        n= sendto(sockfd,"r",1, 0,(struct sockaddr *) &from,fromlen);
                        if (n < 0)
                            error("sendto"); 
                        printf("Tearing Down .....MACHA");

                       // memcpy(writefd,map,fileSize);
                            //unmap file
                        if (munmap(map, fileSize) == -1) {
                            perror("Error un-mmapping the file");
                            /* Decide here whether to close(fd) and exit() or not. Depends... */
                        }

                        close(writefd);
                         break;
                         //continue;
                    }  else {

                        // #######if nackArray->len ==0 do not send the packet
                            if(nackArray->length==0){
                                continue;
                            }
                            printf("Sizeof nack array : %d", sizeof(nackArray));
                            printf("Sizeof nack structure : %d", sizeof(seqACK));
                             n= sendto(sockfd,(char*)nackArray,sizeof(seqACK), 0,(struct sockaddr *) &from,fromlen);
                            //sendto(sockfd,(char*)nackArray,sizeof(nackArray), 0,(struct sockaddr *) &from,fromlen);
                            if (n < 0)
                                error("sendto"); 
                            printf("Nack Packet sent");
                            //printLinklist( head);
                            runsequenceTracker+=350;
                            continue;
                    }

                
              } else{
                     /* Stretch the file size to the size of the (mmapped) array of ints
                    */
                //do this only once as you may get more packets
                if(yes){
                    fileSize=dataFromClient;
                     maxSequenceCalulated=(fileSize/1400)+1;

                    result = lseek(writefd, fileSize-1, SEEK_SET);
                    if (result == -1) {
                        close(writefd);
                        perror("Error calling lseek() to 'stretch' the file");
                        exit(EXIT_FAILURE);
                    }

                    result = write(writefd, "", 1);
                    if (result != 1) {
                        close(writefd);
                        perror("Error writing last byte of the file");
                        exit(EXIT_FAILURE);
                    }

                    map = mmap(0, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, writefd, 0);
                    if (map == MAP_FAILED) {
                        close(writefd);
                        perror("Error mmapping the file");
                        exit(EXIT_FAILURE);
                    }
        
                    //testflag=0;
                    yes=0;
                    //
                    writeToFileFlag=1;
                    continue;
                  }  
              }
           
         }
           else{
                    packet=(customPacket*)buffer;
                    tempSequence=packet->sequenceNo;
                    if(maxSequenceNumber<tempSequence)
                        maxSequenceNumber=tempSequence;
                    printf("max Sequence is : %d\n",maxSequenceNumber);

                    if(writeToFileFlag){
                        printf("Packet length is : %d\n",packet->len);

                        fseek(testfd, addressToAdd, SEEK_SET);
                        addressToAdd=addressToAdd+(packet->sequenceNo*(packet->len-1));
                        fwrite(packet->data,packet->len,1,testfd);
                        fflush(testfd);
                        //memcpy(map+addressToAdd,(char*)packet->data,packet->len);
                        //addressToAdd=addressToAdd+(packet->sequenceNo*(packet->len-1));
                       // printf("address added up to : %d\n",addressToAdd);
                       // printf("data in packet is: %s\n",packet->data );
                        updateNackList(packet->sequenceNo);
                    }

                    

                    //write to mmap or file whatever
                    //if we got the file size => you can map the data to mmap.

         }
         
         if (n < 0)
            error("recvfrom");
     }
    //unmap file
    if (munmap(map, fileSize) == -1) {
        perror("Error un-mmapping the file");
    /* Decide here whether to close(fd) and exit() or not. Depends... */
    }

  close(writefd);
  exit(0); 
}
