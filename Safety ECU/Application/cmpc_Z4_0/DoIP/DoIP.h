/*
 * DoIP.h
 *
 *  Created on: 8 Apr 2026
 *      Author: Mina
 */

#ifndef DOIP_H_
#define DOIP_H_

#include "core/net.h"
#include "core/socket.h"
#include "MPC5748G.h"

#include "mbedtls/sha256.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
// DoIP default TCP port
#define DOIP_TCP_PORT        15000

// DoIP header is always 8 bytes
#define DOIP_HEADER_SIZE     8

// Max payload
#define DOIP_MAX_PAYLOAD     4096

// ========================================
// CUSTOM FLAG-SETTING PROTOCOL DEFINITIONS
// ========================================

// Flag request/response payload types (custom, non-ISO)
#define FLAG_PT_GET_FLAG_REQ     0x1000  // HMI requests current flag value
#define FLAG_PT_GET_FLAG_RES     0x1001  // ECU responds with current flag value
#define FLAG_PT_SET_FLAG_REQ     0x1002  // HMI requests to set a flag value
#define FLAG_PT_SET_FLAG_RES     0x1003  // ECU responds with set result

// Flag setting response codes
#define FLAG_SET_SUCCESS         0x00    // Flag set successfully
#define FLAG_SET_CAR_RUNNING     0x01    // Failed: Car is running/transmission not in Park
#define FLAG_SET_INVALID_VALUE   0x02    // Failed: Invalid flag value
#define FLAG_SET_MEMORY_ERROR    0x03    // Failed: Memory write error

// Address for storing the programming session flag (no-init section)
#define NOINIT_FLAG_ADDRESS      0x400B0000
// Predefined flag value indicating bootloader mode
#define BOOTLOADER_FLAG_VALUE    0xFFFFAAAA

// Maximum number of retries for flash write
#define FLAG_WRITE_MAX_RETRIES   3

typedef struct
{
   uint8_t  ver;             // 1 byte
   uint8_t  inv;             // 1 byte
   uint16_t payloadType;     // 2 bytes
   uint32_t payloadLength;   // 4 bytes
} DoIPHeader;

// Function declarations
void DoIP_TcpServerTask(void *param);

void DoIP_Handle_GetFlagRequest(void);
void DoIP_Handle_SetFlagRequest(const uint8_t *payload, uint32_t payloadLen);
void SoftwareReset(void);


 void test_key();

#endif /* DOIP_H_ */
