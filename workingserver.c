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

typedef struct nackList{
        int num;
        struct nackList *next;
}nackList;

nackList *head = NULL;
nackList *tail = NULL;

int sockfd, portno;
socklen_t fromlen;
struct sockaddr_in serv_addr, from;
FILE *writefd;

int maxSeqNo = 0;
int maxSeqNoSeen = 0;
int teardown = 0;
char *map;  /* mmapped array of int's */

void error(const char *msg){
    perror(msg);
    exit(1);
}

void* sendNack(void *arg){
    int old = 1;
    int new = 1;
    nackList *temp;
    nackList *tempnext;
    while(1){
        old = new;
        new = maxSeqNoSeen;
        int i;

        for(i = old; i < new; i++){
            if(a[0]==0){
                a[0]=1;
                continue;
             }
            if(head == NULL && a[i] == 0){
                printf("creating Head %d\n",i);
                head = (nackList*)malloc(sizeof(nackList));
                head -> num = i;
                head -> next = NULL;
                tail = head;
            }
            else{
                if(a[i] == 0){
                    printf("adding missing sequence to list: %d\n ",i );
                    temp = (nackList*)malloc(sizeof(nackList));
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
                printf("Deleting head\n");
                free(head);
                head = NULL;
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
                    nackPacket->seqNo[nackPacket -> length] = temp->num;
                    nackPacket -> length++;
                }
                if( nackPacket->length >= 350 ){
                //send i to sender
                   //kapil
                        //printf("send Nack list\n");
                    sendto(sockfd,(char*)nackPacket,sizeof(seqACK), 0,(struct sockaddr *) &from,fromlen);
                    nackPacket->length = 0;
                    memset(nackPacket->seqNo,0,350*sizeof(int));
                }
                temp = temp -> next;
            }
        }
        sendto(sockfd,(char*)nackPacket,sizeof(seqACK), 0,(struct sockaddr *) &from,fromlen);
        free(nackPacket);

        if (head == NULL && old == maxSeqNo && old !=0 ){
          //  teardown = 1;
            printf("setting teardown %d\n",teardown);
            pthread_exit();
        }
        sleep(1);
    }
}

int main(int argc, char *argv[]){
    unsigned char buffer[PACK_LEN];
    int chunck_size=0;
    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    int sock_buf_size=1598029824;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&sock_buf_size, sizeof(sock_buf_size));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&sock_buf_size, sizeof(sock_buf_size));
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
    writefd = fopen("fileReceived.bin", "w");
    if (writefd == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    pthread_t handleNackThread;
    int err = pthread_create(&handleNackThread, NULL, &sendNack, NULL);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    int initialChunckSzie=457143;
    int x=59919;
    while (1) {
        if(teardown)
            break;
        n = recvfrom(sockfd,buffer,PACK_LEN,0,(struct sockaddr *)&from,&fromlen);
        if(n == 4 && fileSize == 0){
            nackFile *filesizefromclient;
            filesizefromclient = (nackFile*)malloc(sizeof(nackFile));
            memcpy(filesizefromclient,(nackFile*)buffer,sizeof(nackList));
            fileSize = filesizefromclient->data;
            chunck_size= filesizefromclient->chunckSize;
            chunck_size=83886080;
            free(filesizefromclient);
            if(fileSize % 1400 == 0 )
                maxSeqNo = fileSize / 1400;
            else
                maxSeqNo = (fileSize / 1400) + 1;
            a = (int *)calloc(maxSeqNo,sizeof(int));
        }
        else if(n > 4){
            memcpy(packet, (customPacket*)buffer, n);
            if(maxSeqNoSeen < packet->sequenceNo)
                maxSeqNoSeen = packet->sequenceNo;;
            int addressToAdd = 0;
            addressToAdd = (packet -> sequenceNo - 1)*1400;
            printf("%d\n",packet->sequenceNo);
            //memcpy(map+addressToAdd,(char*)packet->data,packet->len);
            fseek(writefd,addressToAdd,SEEK_SET);
            if(a[packet->sequenceNo]==0){
                fseek(writefd,addressToAdd,SEEK_SET);
                fwrite(packet->data,packet->len,1,writefd);
                fflush(writefd);
                a[packet -> sequenceNo] = 1;
           }
        //sent the server that send next chunck
        //<chunck no>chunckSize*80/1400



         if((packet->sequenceNo) >x){
            printf("Send Next Chunck\n");
            sendto(sockfd,"send next chunck",17, 0,(struct sockaddr *) &from,fromlen);
            initialChunckSzie+=initialChunckSzie;
            x+=17000;
         }
        }
    }
    (void) pthread_join(handleNackThread, NULL);
    int j;
    for(j = 0; j< 10; j++)
        sendto(sockfd,"r",1, 0,(struct sockaddr *) &from,fromlen);
    //if (munmap(map, fileSize) == -1)
    //    perror("Error un-mmapping the file");
    fclose(writefd);
    exit(0);
}
