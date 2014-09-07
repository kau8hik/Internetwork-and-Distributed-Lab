


#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

#include "packetHeader.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{   
    //declare packet structure!
    customPacket *packet;
    char *addr;
    int fd;
    struct stat sb;
    off_t offset, pa_offset;
    size_t length;
    ssize_t s;
    struct sockaddr_in servaddr,cliaddr;
    int sockfd=socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    servaddr.sin_port=htons(32000);
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "%s file offset [length]\n", argv[0]);
        exit(EXIT_FAILURE);
     }
    //int fdOut;
    //fdOut = open("out.bin",O_WRONLY);
    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        handle_error("open");
    if (fstat(fd, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");
    offset = atoi(argv[2]);
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
               /* offset for mmap() must be page aligned */
    if (offset >= sb.st_size) {
        fprintf(stderr, "offset is past end of file\n");
        exit(EXIT_FAILURE);
     }
    if (argc == 4) {
        length = sb.st_size;
	//length = atoi(argv[3]);
        if (offset + length > sb.st_size)
            length = sb.st_size - offset;
                   /* Can't display bytes past end of file */
        } else {    /* No length arg ==> display to end of file */
            length = sb.st_size - offset;
     }
     addr = mmap(NULL, length + offset - pa_offset, PROT_READ,
            MAP_PRIVATE, fd, pa_offset);
     if (addr == MAP_FAILED)
        handle_error("mmap");

    packet =(customPacket *)malloc(sizeof(customPacket));
     int i = 0;
     int seq = 1;
     int digit = 1;
     int initial = 0;
     while(i<length){
	       initial = i;

           packet->sequenceNo=seq;
           packet->len=1400;
           
    if(i<length-1400){
		i = i + 1400;

        memcpy(packet->data,addr+initial,1400);
        //printf("Sequence : %d\t  len: %d\t %s\n",packet->sequenceNo,packet->len,packet->data);
        printf("Size of packet %d \n",(int)sizeof(customPacket));
		sendto(sockfd,(char*)packet,sizeof(customPacket),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
      
	}
	else {
	
	 }
    seq++;
	usleep(0.1);
     }

     exit(EXIT_SUCCESS);
}
