/*
 * DoIP.c
 *
 *  Created on: 25 Feb 2026
 *      Author: johns
 */

#include "doip.h"
#include "debug.h"
#include <string.h>
#include "UDS.h"


#define DOIP_PT_ROUTING_ACT_REQ  0x0005
#define DOIP_PT_ROUTING_ACT_RES  0x0006
#define DOIP_PT_DIAG_MSG         0x8001

#define DOIP_PT_ALIVE_CHECK_REQ  0x0007
#define DOIP_PT_ALIVE_CHECK_RES  0x0008

/* ISO 13400-2: Diagnostic message acknowledgement types */
#define DOIP_PT_DIAG_MSG_POS_ACK 0x8002   /* Positive ACK — sent before UDS response  */
#define DOIP_PT_DIAG_MSG_NEG_ACK 0x8003   /* Negative ACK — sent when message rejected */

/* ISO 13400-2 Table 25: Routing activation response codes */
#define DOIP_ROUTING_SUCCESS     0x10     /* Routing successfully activated */

/* ISO 13400-2 Table 27: Diagnostic message NACK codes */
#define DOIP_DIAG_NACK_INVALID_SRC_ADDR  0x02  /* Unknown source address       */
#define DOIP_DIAG_NACK_MSG_TOO_SHORT     0x04  /* Message too short            */
#define DOIP_DIAG_NACK_UNKNOWN_NTWK_TGT  0x03  /* Unknown network target addr  */

static bool_t g_routingActive = FALSE;
static uint16_t g_testerAddr = 0;
static uint16_t g_ecuAddr = 0x0E80;   // Example ECU logical address


static uint8_t s_doipResp[4 +DOIP_MAX_PAYLOAD];
static uint8_t s_udsResp[DOIP_MAX_PAYLOAD];   // same size as the receive buffer

// ------------------------
// big-endian read
// ------------------------
static uint16_t be16(const uint8_t *p)
{
   return ((uint16_t)p[0] << 8) | p[1];
}

static uint32_t be32(const uint8_t *p)
{
   return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
          ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}

// --------------------------------------
// recvExact: TCP stream -> exact N bytes
// --------------------------------------
static error_t recvExact(Socket *sock, uint8_t *buf, size_t totalLen)
{
   error_t error;
   size_t received = 0;
   size_t n = 0;

   while(received < totalLen)
   {
      error = socketReceive(sock, buf + received, totalLen - received, &n, 0);

      if(error == ERROR_TIMEOUT)
         continue; // no data yet, keep waiting

      if(error)
         return error;

      if(n == 0)
         return ERROR_END_OF_STREAM; // peer closed

      received += n;
   }

   return NO_ERROR;
}

// ---------------------------------------------------
// DoIP_ReadFrame: reads header then payload fully
// ---------------------------------------------------
static error_t DoIP_ReadFrame(Socket *sock, DoIPHeader *hdr,
                             uint8_t *payloadBuf, size_t payloadBufSize)
{
   error_t error;
   uint8_t rawHdr[DOIP_HEADER_SIZE];

   // 1) read header (8 bytes)
   error = recvExact(sock, rawHdr, DOIP_HEADER_SIZE);
   if(error)
      return error;

   // 2) parse header
   hdr->ver = rawHdr[0];
   hdr->inv = rawHdr[1];
   hdr->payloadType   = be16(&rawHdr[2]);
   hdr->payloadLength = be32(&rawHdr[4]);

   // 3) validate version bytes
   if(hdr->ver != 0x02 || hdr->inv != 0xFD)
   {
      TRACE_INFO("DoIP: invalid version ver=%02X inv=%02X\r\n", hdr->ver, hdr->inv);
      return ERROR_INVALID_PROTOCOL;
   }

   // 4) payload length guard
   if(hdr->payloadLength > payloadBufSize)
   {
      TRACE_INFO("DoIP: payload too large: %lu (max %u)\r\n",
                 (unsigned long)hdr->payloadLength,
                 (unsigned)payloadBufSize);
      return ERROR_MESSAGE_TOO_LONG;
   }

   // 5) read payload
   if(hdr->payloadLength > 0)
   {
      error = recvExact(sock, payloadBuf, hdr->payloadLength);
      if(error)
         return error;
   }

   return NO_ERROR;
}



// Convert a raw UDS request into response using your existing UDS module
// returns response length, 0 if no response
uint32_t UDS_ProcessBuffer(const uint8_t *udsReq, uint32_t udsReqLen,
                           uint8_t *udsResp, uint32_t udsRespMax)
{
   if(!udsReq || udsReqLen == 0 || !udsResp || udsRespMax == 0)
      return 0;

   // Clear global frames safely
   requestFrame.ready = 0;
   requestFrame.dataSize = 0;
   memset(requestFrame.dataBuffer, 0, sizeof(requestFrame.dataBuffer));

   responseFrame.ready = 0;
   responseFrame.dataSize = 0;
   memset(responseFrame.dataBuffer, 0, sizeof(responseFrame.dataBuffer));

   // Copy request into requestFrame
   if(udsReqLen > sizeof(requestFrame.dataBuffer))
      udsReqLen = sizeof(requestFrame.dataBuffer);

   memcpy(requestFrame.dataBuffer, udsReq, udsReqLen);
   requestFrame.dataSize = udsReqLen;
   requestFrame.ready = READY;

   // Run UDS state machine
   UDS_Receive();

   // If no response ready -> return 0
   if(responseFrame.ready != READY || responseFrame.dataSize == 0)
      return 0;

   // Copy response out
   uint32_t outLen = responseFrame.dataSize;
   if(outLen > udsRespMax)
      outLen = udsRespMax;

   memcpy(udsResp, responseFrame.dataBuffer, outLen);
   return outLen;
}


/**
 * @brief Send a DoIP frame (Header + Payload) over an established TCP socket
 * @param[in] sock        CycloneTCP socket handle (connected)
 * @param[in] payloadType DoIP payload type (e.g. 0x0006, 0x8001)
 * @param[in] payload     Pointer to payload buffer (may be NULL if payloadLen == 0)
 * @param[in] payloadLen  Payload size in bytes
 * @return Error code
 */
static error_t DoIP_SendFrame(Socket *sock, uint16_t payloadType,
                              const uint8_t *payload, uint32_t payloadLen)
{
   uint8_t hdr[DOIP_HEADER_SIZE];
   size_t sent;
   error_t error;

   hdr[0] = 0x02;
   hdr[1] = 0xFD;
   hdr[2] = (payloadType >> 8) & 0xFF;
   hdr[3] = payloadType & 0xFF;
   hdr[4] = (payloadLen >> 24) & 0xFF;
   hdr[5] = (payloadLen >> 16) & 0xFF;
   hdr[6] = (payloadLen >> 8) & 0xFF;
   hdr[7] = (payloadLen) & 0xFF;

   // Send header
   error = socketSend(sock, hdr, DOIP_HEADER_SIZE, &sent, 0);
   if(error)
      return error;

   // Send payload (if any)
   if(payloadLen > 0 && payload != NULL)
   {
      error = socketSend(sock, payload, payloadLen, &sent, 0);
      if(error)
         return error;
   }

   return NO_ERROR;
}

// ---------------------
// TCP server main task
// ---------------------
void DoIP_TcpServerTask(void *param)
{
   error_t error;
   Socket *listenSock;
   Socket *clientSock;
   IpAddr clientIp;
   uint16_t clientPort;

   static uint8_t payload[DOIP_MAX_PAYLOAD];
   DoIPHeader hdr;

   listenSock = socketOpen(SOCKET_TYPE_STREAM, SOCKET_IP_PROTO_TCP);
   if(!listenSock)
   {
      TRACE_ERROR("DoIP: socketOpen failed\r\n");
      osSuspendAllTasks();
   }

   socketSetTimeout(listenSock, 0);

   error = socketBind(listenSock, &IP_ADDR_ANY, DOIP_TCP_PORT);
   if(error)
   {
      TRACE_ERROR("DoIP: bind failed (%d)\r\n", error);
      socketClose(listenSock);
      osSuspendAllTasks();
   }

   error = socketListen(listenSock, 1);
   if(error)
   {
      TRACE_ERROR("DoIP: listen failed (%d)\r\n", error);
      socketClose(listenSock);
      osSuspendAllTasks();
   }

   TRACE_INFO("DoIP: TCP server listening on %u\r\n", DOIP_TCP_PORT);

   while(1)
   {
      clientSock = socketAccept(listenSock, &clientIp, &clientPort);
      if(clientSock)
      {
         TRACE_INFO("DoIP: client connected %s:%u\r\n",
                    ipAddrToString(&clientIp, NULL), clientPort);

         // Reset session flags per connection
         g_routingActive = FALSE;
         g_testerAddr = 0;
         UDS_ResetProgrammingState();  // <--- added so we can rerun 5/3/2026

         socketSetTimeout(clientSock, 60000);

         while(1)
         {
            error = DoIP_ReadFrame(clientSock, &hdr, payload, sizeof(payload));

            if(error == NO_ERROR)
            {
               TRACE_INFO("DoIP frame: type=0x%04X len=%lu\r\n",
                          hdr.payloadType, (unsigned long)hdr.payloadLength);

               // -----------------------------
               // Alive Check handling
               // -----------------------------
              if(hdr.payloadType == DOIP_PT_ALIVE_CHECK_REQ)
            	   {
            	      // ISO: only answer AliveCheck after routing activation
            	      if(!g_routingActive)
            	      {
            	         TRACE_INFO("DoIP: AliveCheck ignored (routing not active)\r\n");
            	         continue;
            	      }

            	      uint8_t aliveResp[2];
            	      aliveResp[0] = (g_testerAddr >> 8) & 0xFF;
            	      aliveResp[1] = (g_testerAddr) & 0xFF;

            	      TRACE_INFO("DoIP: AliveCheck req -> resp type=0x0008 len=2 SA=0x%04X\r\n",
            	                 g_testerAddr);

            	      (void)DoIP_SendFrame(clientSock, DOIP_PT_ALIVE_CHECK_RES,
            	                           aliveResp, sizeof(aliveResp));
            	      continue;
            	   }

               // -----------------------------
               // Routing Activation hzandling
               // -----------------------------
               if(hdr.payloadType == DOIP_PT_ROUTING_ACT_REQ)
               {
                  if(hdr.payloadLength >= 2)
                  {
                     g_testerAddr = ((uint16_t)payload[0] << 8) | payload[1];
                  }
                  else
                  {
                     g_testerAddr = 0x0E00; // fallback tester address
                  }

                  g_routingActive = TRUE;

                  TRACE_INFO("DoIP: Routing Activation OK (tester=0x%04X ecu=0x%04X)\r\n",
                             g_testerAddr, g_ecuAddr);


                  uint8_t respPayload[9];
                  respPayload[0] = (g_testerAddr >> 8) & 0xFF;
                  respPayload[1] = g_testerAddr & 0xFF;
                  respPayload[2] = (g_ecuAddr >> 8) & 0xFF;
                  respPayload[3] = g_ecuAddr & 0xFF;
                  respPayload[4] = DOIP_ROUTING_SUCCESS; /* 0x10 = routing successfully activated */
                  respPayload[5] = 0x00;                 /* reserved_iso byte 1                  */
                  respPayload[6] = 0x00;                 /* reserved_iso byte 2                  */
                  respPayload[7] = 0x00;                 /* reserved_iso byte 3                  */
                  respPayload[8] = 0x00;                 /* reserved_iso byte 4                  */

                  (void)DoIP_SendFrame(clientSock, DOIP_PT_ROUTING_ACT_RES,
                                       respPayload, sizeof(respPayload));
                  continue;
               }

               // -----------------------------
               // Diagnostic Message handling (0x8001)
               // -----------------------------

               if(hdr.payloadType == DOIP_PT_DIAG_MSG)
               {

                  if(!g_routingActive)
                  {
                     TRACE_INFO("DoIP: DIAG msg rejected (routing not active)\r\n");

                     /* Need at least src address from payload to build NACK */
                     if(hdr.payloadLength >= 2)
                     {
                        uint16_t rejSrc = ((uint16_t)payload[0] << 8) | payload[1];
                        uint8_t nack[5];
                        nack[0] = (g_ecuAddr >> 8) & 0xFF;            /* source: ECU      */
                        nack[1] = g_ecuAddr & 0xFF;
                        nack[2] = (rejSrc >> 8) & 0xFF;               /* target: tester   */
                        nack[3] = rejSrc & 0xFF;
                        nack[4] = DOIP_DIAG_NACK_INVALID_SRC_ADDR;    /* NACK code: 0x02  */
                        (void)DoIP_SendFrame(clientSock, DOIP_PT_DIAG_MSG_NEG_ACK,
                                             nack, sizeof(nack));
                     }
                     continue;
                  }

                  if(hdr.payloadLength < 4)
                  {
                     TRACE_INFO("DoIP: DIAG payload too short\r\n");
                     uint8_t nack[5];
                     nack[0] = (g_ecuAddr >> 8) & 0xFF;
                     nack[1] = g_ecuAddr & 0xFF;
                     nack[2] = (g_testerAddr >> 8) & 0xFF;
                     nack[3] = g_testerAddr & 0xFF;
                     nack[4] = DOIP_DIAG_NACK_MSG_TOO_SHORT;           /* NACK code: 0x04  */
                     (void)DoIP_SendFrame(clientSock, DOIP_PT_DIAG_MSG_NEG_ACK,
                                          nack, sizeof(nack));
                     continue;
                  }

                  uint16_t src = ((uint16_t)payload[0] << 8) | payload[1];
                  uint16_t tgt = ((uint16_t)payload[2] << 8) | payload[3];

                  uint8_t *udsReq = &payload[4];
                  uint32_t udsReqLen = hdr.payloadLength - 4;

                  TRACE_INFO("DoIP DIAG: src=0x%04X tgt=0x%04X UDS_len=%lu\r\n",
                             src, tgt, (unsigned long)udsReqLen);

                  uint8_t ack[5];
                  ack[0] = (tgt >> 8) & 0xFF;   /* source: ECU   (tgt from request) */
                  ack[1] = tgt & 0xFF;
                  ack[2] = (src >> 8) & 0xFF;   /* target: tester (src from request) */
                  ack[3] = src & 0xFF;
                  ack[4] = 0x00;                 /* ACK code 0x00 = ACK               */
                  (void)DoIP_SendFrame(clientSock, DOIP_PT_DIAG_MSG_POS_ACK,
                                       ack, sizeof(ack));

                  uint32_t udsRespLen = UDS_ProcessBuffer(udsReq, udsReqLen, s_udsResp, sizeof(s_udsResp));

                  if(udsRespLen == 0)
                  {
                     TRACE_INFO("DoIP: no UDS response generated\r\n");
                     continue;
                  }

                  // ECU becomes source, tester becomes target (swap)
                  s_doipResp[0] = (tgt >> 8) & 0xFF;
                  s_doipResp[1] = tgt & 0xFF;
                  s_doipResp[2] = (src >> 8) & 0xFF;
                  s_doipResp[3] = src & 0xFF;

                  memcpy(&s_doipResp[4], s_udsResp, udsRespLen);

                  error_t sendErr = DoIP_SendFrame(clientSock, DOIP_PT_DIAG_MSG,
                                                   s_doipResp, 4 + udsRespLen);

                  if(sendErr)
                  {
                     TRACE_INFO("DoIP: failed to send DIAG response (%d)\r\n", sendErr);
                  }
                  else
                  {
                     TRACE_INFO("UDS RESP: ");
                     for(uint32_t i = 0; i < udsRespLen; i++)
                        TRACE_INFO("%02X ", s_udsResp[i]);
                     TRACE_INFO("\r\n");
                  }

                  continue;
               }

               // Other payload types: ignore for now
               continue;
            }

            // Normal disconnect case (client closed socket)
            if(error == ERROR_END_OF_STREAM)
            {
               TRACE_INFO("DoIP: peer closed connection\r\n");
            }
            else
            {
               TRACE_INFO("DoIP: read error (%d)\r\n", error);
            }

            break;
         }

         socketClose(clientSock);
         TRACE_INFO("DoIP: client disconnected\r\n");
      }

      osDelayTask(10);
   }
}
