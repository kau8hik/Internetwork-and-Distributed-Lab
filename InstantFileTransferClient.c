#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "packetHeader.h"

#define IP_ADDR "10.1.1.3"
#define SEND_PORT 32000
#define RECV_PORT 32001
#define PACK_LEN 1400
#define HEAD_LEN 8

char *addr; //holds the pointer of the file returned by mmap
struct sockaddr_in servaddr; //the client sending socket structure
int sockfd = 0; //the client sending socket file descriptor
size_t fileLength; //fileLength of the file 

void handle_error(const char *msg){ 
    perror(msg); 
	exit(EXIT_FAILURE); 
}

//Function to send a packet 
void sendPacket(int socket,customPacket* pack, size_t len, struct sockaddr_in to, socklen_t tolen){
	sendto(socket,(char*)pack,len,0,(struct sockaddr *)&to,tolen);
}

//Function to handle the nacks and resend the missed packets
void *recieveNACK(void *arg){
    socklen_t fromlen;
    unsigned char buffer[PACK_LEN+HEAD_LEN];
    struct sockaddr_in serv_addr, from;
    int n;
    int recieveSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
       	handle_error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(RECV_PORT);
    if (bind(recieveSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    	handle_error("ERROR on binding");
	fromlen = sizeof(struct sockaddr_in);
	seqACK *acks;
	customPacket *packet;
	packet=(customPacket* )malloc(sizeof(customPacket));
    while (1) {
        n = recvfrom(recieveSocket,buffer,PACK_LEN,0,(struct sockaddr *)&from,&fromlen);
		acks=(seqACK*)buffer;
		customPacket *packet;
		packet=(customPacket* )malloc(sizeof(customPacket));
		int j = 0;
		for(j=0; j<acks->length; j++){
	
	        packet->sequenceNo = acks->seqNo[j];
			int pack_len = 0;
   			if(acks->seqNo[j]*1400>fileLength){
				pack_len = fileLength - acks->seqNo[j]*1400;
				packet->len = pack_len;
			}
			packet->len = 1400;      
		
   	    	memcpy(packet->data,addr+1400*acks->seqNo[j],packet->len);
			sendPacket(sockfd,packet,PACK_LEN+HEAD_LEN,servaddr,sizeof(servaddr)); 
       	}
		   
	    usleep(0.1);
		printf("%s",buffer);
        if (n < 0)
           handle_error("recvfrom");
		sendPacket(sockfd,packet,PACK_LEN+HEAD_LEN,servaddr,sizeof(servaddr)); 
	}

}
			
int main(int argc, char *argv[])
{   
    //declare packet structure!
    int fd;
    struct stat sb;
    off_t offset, pa_offset;
    ssize_t s;
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDR);
    servaddr.sin_port = htons(SEND_PORT);

    if (argc < 1 || argc > 2) {
        fprintf(stderr, "%s filename \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        handle_error("open");
    if (fstat(fd, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");
    offset = 0;
	//printf("%d \n",sb.st_size):
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
               /* offset for mmap() must be page aligned */
    if (offset >= sb.st_size) {
		//printf("%d %d\n",offset,sb.st_size);
        fprintf(stderr, "offset is past end of file\n");
        exit(EXIT_FAILURE);
    }
	
    fileLength = sb.st_size;
    addr = mmap(NULL, fileLength + offset - pa_offset, PROT_READ, MAP_PRIVATE, fd, pa_offset);
    if (addr == MAP_FAILED)
        handle_error("mmap");

	customPacket *packet;
	packet =(customPacket *)malloc(sizeof(customPacket));
    int i = 0;
    int seq = 1;
	int initial = 0;
	//pthread_t recvThread;
	FILE *fpc =fopen("cli","w");
	FILE *ffread = fopen("data.bin","rb");
	//pthread_create(&recvThread,NULL,recieveNACK,NULL);
    while(i<fileLength){
		initial = i;
        packet->sequenceNo=seq;
        fprintf(fpc,"%d\n",packet->sequenceNo); 
		if(i<fileLength-PACK_LEN){
	   		i = i + PACK_LEN;
        	packet->len=PACK_LEN;
   	    	memcpy(packet->data,addr+initial,PACK_LEN);
			sendPacket(sockfd,packet,PACK_LEN+HEAD_LEN,servaddr,sizeof(servaddr)); 
       	}
	   	else{
			i = fileLength;
			packet->len=fileLength - initial;
   	    	memcpy(packet->data,addr+initial,packet->len);
			packet->data[fileLength-initial]=0;
			sendPacket(sockfd,packet,fileLength-initial+HEAD_LEN,servaddr,sizeof(servaddr)); 
	   	}
		seq++;
	    //usleep(3);
    }
	//(void) pthread_join(recvThread, NULL);
	exit(EXIT_SUCCESS);
