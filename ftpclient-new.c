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

#define IP_ADDR "127.0.0.1"
#define SEND_PORT 32000
#define RECV_PORT 32001
#define PACK_LEN 1400
#define HEAD_LEN 8
/*typedef struct 
{
    int data;
} nackFile;
*/
int killParent = 0;
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
void sendPacketChar(int socket,nackFile* pack, size_t len, struct sockaddr_in to, socklen_t tolen){
    sendto(socket,(char*)pack,len,0,(struct sockaddr *)&to,tolen);
}

//Function to handle the nacks and resend the missed packets
void *recieveNACK(void *arg){
    socklen_t fromlen;
    unsigned char buffer[PACK_LEN+HEAD_LEN];
    //struct sockaddr_in serv_addr, from;
    struct sockaddr_in from;
    int n;
    fromlen = sizeof(struct sockaddr_in);
    seqACK *acks;
    int *nacklistcopied;
    //--kapil
    acks =(seqACK*)malloc(sizeof(seqACK));
    customPacket *packet;
    packet=(customPacket* )malloc(sizeof(customPacket));
    printf("Waiting for NAC packet\n");
    while (1) {
        //n = recvfrom(recieveSocket,buffer,PACK_LEN,0,(struct sockaddr *)&from,&fromlen);
        n = recvfrom(sockfd,buffer,PACK_LEN,0,NULL,NULL);//from,&fromlen);
        if(n==1){  
            killParent = 1;
            printf("tear down");
            break;
        }
        else {
            acks=(seqACK*)buffer;
            //--kapil
            //customPacket *packet;
            // packet=(customPacket* )malloc(sizeof(customPacket));
            //nacklistcopied=(int *)malloc(sizeof(acks->length));

            //memcpy(nacklistcopied,acks->seqNo,acks->length);
            int j = 0;
            printf("Printing list\n");
            for(j=0;j<acks->length;j++)
                printf(" %d\t ",acks->seqNo[j]);
            j=0;

            for(j=0; j<acks->length; j++){
                if (acks->seqNo[j]==0)continue;
                printf("%d------>\n",acks->seqNo[j]);
                packet->sequenceNo = acks->seqNo[j];
                int pack_len = 0;
                if(acks->seqNo[j]*1400>fileLength){
                    pack_len = fileLength - acks->seqNo[j]*1400;
                    packet->len = pack_len;
                }
                packet->len = 1400;      
        
                memcpy(packet->data,addr+1400*(acks->seqNo[j]-1),packet->len);
                sendPacket(sockfd,packet,PACK_LEN+HEAD_LEN,servaddr,sizeof(servaddr)); 
            }
            printf("%s",buffer);
            if (n < 0)  handle_error("recvfrom");
            sendPacket(sockfd,packet,PACK_LEN+HEAD_LEN,servaddr,sizeof(servaddr)); 
        }
    }
    return NULL;
}        
int main(int argc, char *argv[]){   
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
    for(i=0;i<10;i++){
        printf("sending file len\n");
        nackFile *nf;
        nf = (nackFile *)malloc(sizeof(nackFile));
        nf->data = fileLength;
        sendPacketChar(sockfd,(nackFile *)nf,4,servaddr,sizeof(servaddr)); 
        free(nf);
    }
    i = 0;
    pthread_t recvThread;
    FILE *fpc =fopen("cli","w");
    pthread_create(&recvThread,NULL,recieveNACK,NULL);
    while(i<fileLength){
        initial = i;
        packet->sequenceNo=seq;
        fprintf(fpc,"%d\n",packet->sequenceNo); 
        if(i<fileLength-PACK_LEN){
            i = i + PACK_LEN;
            if(i%10000==0){
                nackFile *nf1;
                nf1 = (nackFile *)malloc(sizeof(nackFile));
                nf1->data = -1;
                //sendPacketChar(sockfd,(nackFile *)nf1,4,servaddr,sizeof(servaddr)); 
                free(nf1);
            }
            if(i==1400) continue;
            packet->len=PACK_LEN;
            //printf("Sending Sequence number : %d\n",packet->sequenceNo);
            //printf("%s\n",packet->data);
            memcpy(packet->data,addr+initial,PACK_LEN);
            printf("Sending Sequence number : %d\n",packet->sequenceNo);
            sendPacket(sockfd,packet,PACK_LEN+HEAD_LEN,servaddr,sizeof(servaddr)); 
        }
        else{
            i = fileLength;
            packet->len=fileLength - initial;
           // printf("Sending Sequence number : %d\n",packet->sequenceNo);
             //printf("%s\n",packet->data);
            memcpy(packet->data,addr+initial,packet->len);
            packet->data[fileLength-initial]=0;
            sendPacket(sockfd,packet,fileLength-initial+HEAD_LEN,servaddr,sizeof(servaddr)); 
        }
        seq++;
        //usleep(3);
    }
    while(1){ 
        //printf("sent the final");
        nackFile *nf1;
        nf1 = (nackFile *)malloc(sizeof(nackFile));
        nf1->data = -1;
        sendPacketChar(sockfd,(nackFile *)nf1,4,servaddr,sizeof(servaddr)); 
        free(nf1);
        printf("%d",killParent);
        sleep(1);
        if(killParent)
            break;
    }
    (void) pthread_join(recvThread, NULL);
    exit(EXIT_SUCCESS);
}
