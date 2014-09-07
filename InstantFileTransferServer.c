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

     while (1) {
         n = recvfrom(sockfd,buffer,PACK_LEN,0,(struct sockaddr *)&from,&fromlen);
       
         packet=(customPacket*)buffer;
         printf("Packet sequenceNumber : %d\n packet Length: %d\n",packet->sequenceNo,packet->len);
         //logic to store the sequence number.
         printf("%s\n", packet->data);
         //
         if (n < 0)
            error("recvfrom");
          n = sendto(sockfd,"Got your message ",17, 0,(struct sockaddr *) &from,fromlen);
          if (n < 0)
             error("sendto");    

     }

  exit(0); 
}




