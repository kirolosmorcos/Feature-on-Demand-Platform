/*
 * DoIP.h
 *
 *  Created on: 25 Feb 2026
 *      Author: johns
 */

#ifndef DOIP_H_
#define DOIP_H_


#include "core/net.h"
#include "core/socket.h"

// DoIP default TCP port
#define DOIP_TCP_PORT        13400

// DoIP header is always 8 bytes
#define DOIP_HEADER_SIZE     8

// Max payload
#define DOIP_MAX_PAYLOAD     8200  // increased payload size to support 8kB MinaS and JohnS 3/5/2026

typedef struct
{
   uint8_t  ver;				//1 byte
   uint8_t  inv;				//1 byte
   uint16_t payloadType;		//2 byte
   uint32_t payloadLength;		//4 byte
} DoIPHeader;

// Start the ECU-side TCP server task
void DoIP_TcpServerTask(void *param);



#endif /* DOIP_H_ */
