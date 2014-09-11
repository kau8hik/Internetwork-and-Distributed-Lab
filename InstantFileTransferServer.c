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
#include "packetHeader.h"

#define PACK_LEN 1500
#define DATA_LEN 1400

int fileSize = 0;


int *a;

typedef struct {
	int num;
	struct nackList *next;
}nackList;

nackList *head = NULL;
nackList *tail = NULL;

nackFile *filesizefromclient;
int sockfd, portno;
socklen_t fromlen;
struct sockaddr_in serv_addr, from;
int writefd;

char *map;  /* mmapped array of int's */

void error(const char *msg){
    perror(msg);
    exit(1);
}

void* sendNack(void *arg){
    while(1){    
	for(i = old; i < new; i++){
	    if(head == NULL && a[i] == 0){
	    	head = (data*)malloc(sizeof(data));
	    	head -> num = i;
	    	head -> next = NULL;
	    	tail = head;
	    	first = head;
            }
            else{
	    	if(a[i] == 0){
		    temp = (data*)malloc(sizeof(data));
		    tail -> next = temp;
		    temp -> num = i;
		    temp -> next = NULL;
		    tail = temp;
	     	}
	    } 
     	}
    	temp = head;
	seqACK *nackPacket = (seqACK *)malloc(sizeof(seqACK));    
    	nackPacket->length = 0;
    	while(temp != NULL){
  	    if(temp -> next == NULL && a[temp->num] == 1){
	    	free(head);
	    	head = NULL:
	    	temp = NULL;
            } 
   	    else if(a[temp->num]==1){
	    	tempnext = temp -> next;
	    	temp -> num = tempnext -> num;
     	    	temp -> next = tempnext -> next;
	    	free(tempnext);
	    }
	    else{
	    	if( nackPacket->length < 350 ){
  	            nackPacket->length++;
		    nackPacket->seqNo[nackPacketStatus] = temp->num;
		    nackPacketStatus++;
	    	}
  	        if( nackPacket->length >= 350 ){    
      	        //send i to sender
                    n= sendto(sockfd,(char*)nackPacket,sizeof(seqACK), 0,(struct sockaddr *) &from,fromlen);
	            nackPacket->length = 0;
	            nackPacket->seqNo={0};
    	    	}
	        temp = temp -> next;
	    }
	    n= sendto(sockfd,(char*)nackPacket,sizeof(seqACK), 0,(struct sockaddr *) &from,fromlen);
	    free(nackPacket);
     	}
    }
}

int main(int argc, char *argv[]){
    int maxSequenceNumber=0;
    unsigned char buffer[PACK_LEN];
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
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    fromlen = sizeof(struct sockaddr_in);
    customPacket *packet;
    packet=(customPacket* )malloc(sizeof(customPacket));
    writefd = open("fileReceived.c", O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (writefd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    pthread_t handleNackThread; 
    int err = pthread_create(&handleNackThread, NULL, &sendNack, NULL);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    while (1) {
  	n = recvfrom(sockfd,buffer,PACK_LEN,0,(struct sockaddr *)&from,&fromlen);
        if(n == 4 && fileSize == 0){
    	    filesizefromclient = (nackFile*)malloc(sizeof(nackFile));
	    filesizefromclient = (nackFile*)buffer;
            fileSize = filesizefromclient->data;
	    free(filesizefromclient);
            map = mmap(0, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, writefd, 0);
            if (map == MAP_FAILED) {
                close(writefd);
                perror("Error mmapping the file");
                exit(EXIT_FAILURE);
            }
	} 
	else if(n > 4){
            memcpy(packet, (customPacket*)buffer, n);
            if(maxSequenceNumber < packet->sequenceNo)
            	maxSequenceNumber = packet->sequenceNo;;
            int addressToAdd = 0;
	    addressToAdd = (packet -> sequenceNo - 1)*1400;
	    memcpy(map+addressToAdd,(char*)packet->data,packet->len);
    	    a[packet -> sequenceNo] = 1;
	}
    }
    if (munmap(map, fileSize) == -1) {
        perror("Error un-mmapping the file");
    }
    close(writefd);
    exit(0); 
}
