#ifndef PACKET_H
#define PACKET_H
#include <stdio.h>
#define SEQ_PACK 350
typedef struct 
{
	int sequenceNo;
	int len;
	char data[1400];
} customPacket;
typedef struct
{
	int length;
	int seqNo[SEQ_PACK];
} seqACK;
#endif
