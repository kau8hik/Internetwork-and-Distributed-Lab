#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>

typedef struct 
{
	int sequenceNo;
	int len;
	char data[1400];
} customPacket;

#endif
