/*
 * DoIP.c - Unified version with flag functionality and reset support
 * FIXED VERSION: Proper diagnostic handling, flag setting, and socket management
 *
 *  Created on: 8 Apr 2026
 *      Author: Mina
 *      Fixed: 22 Apr 2026
 */


#include "doip.h"
#include "debug.h"
#include <string.h>
#include "UDS.h"
#include "feature_management.h"

extern Feature_manager gFeatureManager;

#define DOIP_PT_ROUTING_ACT_REQ  0x0005
#define DOIP_PT_ROUTING_ACT_RES  0x0006
#define DOIP_PT_DIAG_MSG         0x8001

#define DOIP_PT_ALIVE_CHECK_REQ  0x0007
#define DOIP_PT_ALIVE_CHECK_RES  0x0008

/* ISO 13400-2: Diagnostic message acknowledgement types */
#define DOIP_PT_DIAG_MSG_POS_ACK 0x8002   /* Positive ACK – sent before UDS response  */
#define DOIP_PT_DIAG_MSG_NEG_ACK 0x8003   /* Negative ACK – sent when message rejected */

/* ISO 13400-2 Table 25: Routing activation response codes */
#define DOIP_ROUTING_SUCCESS     0x10     /* Routing successfully activated */

/* ISO 13400-2 Table 27: Diagnostic message NACK codes */
#define DOIP_DIAG_NACK_INVALID_SRC_ADDR  0x02  /* Unknown source address       */
#define DOIP_DIAG_NACK_MSG_TOO_SHORT     0x04  /* Message too short            */
#define DOIP_DIAG_NACK_UNKNOWN_NTWK_TGT  0x03  /* Unknown network target addr  */

/* Custom Reset Protocol Payload Types */
#define RESET_PT_RESET_REQ       0x2000
#define RESET_PT_RESET_RES       0x2001

// dummy variable
static uint8_t g_carRunningState = 0; // 0 = not running/Park, 1 = running/not in Park

static bool_t g_routingActive = FALSE;
static uint16_t g_testerAddr = 0;
static uint16_t g_ecuAddr = 0x0E80;   // Example ECU logical address

/*FOR HANDLING THE ENABLE AND DISABLE */

#define DOIP_PT_FEATURE_ENABLE_REQ      0x4100
#define DOIP_PT_FEATURE_ENABLE_RES      0x6100
#define DOIP_PT_FEATURE_DISABLE_REQ     0x4200
#define DOIP_PT_FEATURE_DISABLE_RES     0x6200



// for signature verification
#define LICENSE_TOKEN_BASE_SIZE        (19U)
#define LICENSE_SIGNATURE_B64_SIZE     (400U)
#define LICENSE_TASK_ID_SIZE           (2U)

#define LICENSE_TOKEN_PAYLOAD_SIZE (LICENSE_TOKEN_BASE_SIZE + LICENSE_SIGNATURE_B64_SIZE + LICENSE_TASK_ID_SIZE)


// for Feature enable
#define FEATURE_STATUS_SUCCESS          0x00U
#define FEATURE_STATUS_INVALID_LEN      0x01U
#define FEATURE_STATUS_FAILED           0x02U

// for diagnostic
#define DOIP_PT_DIAGNOSTIC_REQ   		0x4300
#define DOIP_PT_DIAGNOSTIC_RES   		0x6300

#define DIAG_STATUS_SUCCESS          0x00U
#define DIAG_STATUS_INVALID_LEN      0x01U
#define DIAG_STATUS_FAILED           0x02U
#define DIAG_STATUS_INVALID_ID       0x03U


static uint8_t s_doipResp[4 + DOIP_MAX_PAYLOAD]; // 16/3/2026
static uint8_t s_udsResp[DOIP_MAX_PAYLOAD];   // same size as the receive buffer

// Current socket for sending flag responses (needed for async responses)
static Socket *g_currentClientSocket = NULL;



// ------------------------
// Helpers (big-endian read)
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

void SoftwareReset(void) {
    /*
     * We use the Mode Control (MCTL) register to transition to RESET mode (0x00).
     * This requires a two-step key sequence: 0x5AF0 then 0xA50F.
     */

    /* Target Mode 0 = RESET */
    MC_ME.MCTL.R = 0x00005AF0; // Mode 0 + Key part 1
    MC_ME.MCTL.R = 0x0000A50F; // Mode 0 + Key part 2

    /* The processor will reset immediately after the second write.
       We add an infinite loop just in case, though it will never be reached. */
    while(1);
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

   TRACE_INFO("UDS DBG: reqLen=%u bytes: ", (unsigned)requestFrame.dataSize);
   for(uint32_t i = 0; i < requestFrame.dataSize; i++)
   {
      TRACE_INFO("%02X ", requestFrame.dataBuffer[i]);
   }
   TRACE_INFO("\r\n");

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

// ========================================
// FLAG HANDLING FUNCTIONS
// ========================================

static uint8_t DoIP_WriteFlagToMemory(uint32_t flagValue)
{
   uint8_t retry = 0;

   TRACE_INFO("DoIP: Writing flag 0x%08X to address 0x%08X\r\n",
              (unsigned)flagValue, (unsigned)NOINIT_FLAG_ADDRESS);

   for(retry = 0; retry < FLAG_WRITE_MAX_RETRIES; retry++)
   {
      // Volatile pointer to write to no-init section
      volatile uint32_t *flagAddr = (volatile uint32_t *)NOINIT_FLAG_ADDRESS;
      *flagAddr = flagValue;

      // Verify write
      if(*flagAddr == flagValue)
      {
         TRACE_INFO("DoIP: Flag write successful on attempt %u\r\n", retry + 1);
         return 0;
      }

      TRACE_INFO("DoIP: Flag write verification failed, retrying (attempt %u)\r\n", retry + 1);
      osDelayTask(10); // Small delay before retry
   }

   TRACE_ERROR("DoIP: Flag write failed after %u attempts\r\n", FLAG_WRITE_MAX_RETRIES);
   return 1;
}


static uint32_t DoIP_ReadFlagFromMemory(void)
{
   volatile uint32_t *flagAddr = (volatile uint32_t *)NOINIT_FLAG_ADDRESS;
   uint32_t currentFlag = *flagAddr;

   TRACE_INFO("DoIP: Read flag value 0x%08X from address 0x%08X\r\n",
              (unsigned)currentFlag, (unsigned)NOINIT_FLAG_ADDRESS);

   return currentFlag;
}


static uint8_t DoIP_CheckVehicleState(void)
{
   // In real implementation, this would read from actual CAN signals
   // For now, we use the dummy variable g_carRunningState

   if(g_carRunningState != 0)
   {
      TRACE_INFO("DoIP: Vehicle is running or not in Park - flag setting denied\r\n");
      return 1; // Not safe
   }

   TRACE_INFO("DoIP: Vehicle state check passed - safe to set flag\r\n");
   return 0; // Safe
}


void DoIP_Handle_GetFlagRequest(void)
{
   uint8_t response[8]; // Flag_value (4 bytes) + status (1 byte) + padding
   uint32_t currentFlag = DoIP_ReadFlagFromMemory();

   // Response format: [Flag_value(4 bytes)][Status(1 byte)]
   response[0] = (currentFlag >> 24) & 0xFF;
   response[1] = (currentFlag >> 16) & 0xFF;
   response[2] = (currentFlag >> 8) & 0xFF;
   response[3] = currentFlag & 0xFF;
   response[4] = 0x00; // Success status

   TRACE_INFO("DoIP: Sending GET_FLAG response: 0x%08X\r\n", (unsigned)currentFlag);

   if(g_currentClientSocket != NULL)
   {
      (void)DoIP_SendFrame(g_currentClientSocket, FLAG_PT_GET_FLAG_RES,
                          response, 5);
   }
}

/**
 * Expected payload format:
 * [Flag_value(4 bytes)] = 0xFFFFAAAA for bootloader mode
 */
void DoIP_Handle_SetFlagRequest(const uint8_t *payload, uint32_t payloadLen)
{
   uint8_t response[2]; // [Status(1 byte)][Reserved(1 byte)]
   uint32_t flagValue = 0;
   uint8_t status = FLAG_SET_SUCCESS;

   // Parse flag value from payload
   if(payloadLen < 4)
   {
      TRACE_ERROR("DoIP: SET_FLAG payload too short (%u bytes)\r\n", (unsigned)payloadLen);
      status = FLAG_SET_INVALID_VALUE;
   }
   else
   {
      flagValue = be32(&payload[0]);

      TRACE_INFO("DoIP: SET_FLAG request with value 0x%08X\r\n", (unsigned)flagValue);

      // Check vehicle state
      if(DoIP_CheckVehicleState() != 0)
      {
         status = FLAG_SET_CAR_RUNNING;
      }
      // Validate flag value (only accept bootloader flag value)
      else if(flagValue != BOOTLOADER_FLAG_VALUE)
      {
         TRACE_ERROR("DoIP: Invalid flag value 0x%08X (expected 0x%08X)\r\n",
                     (unsigned)flagValue, (unsigned)BOOTLOADER_FLAG_VALUE);
         status = FLAG_SET_INVALID_VALUE;
      }
      // Attempt to write flag
      else if(DoIP_WriteFlagToMemory(flagValue) != 0)
      {
         status = FLAG_SET_MEMORY_ERROR;
      }
   }

   // Build response
   response[0] = status;
   response[1] = 0x00; // Reserved

   TRACE_INFO("DoIP: Sending SET_FLAG response with status 0x%02X\r\n", status);

   if(g_currentClientSocket != NULL)
   {
      (void)DoIP_SendFrame(g_currentClientSocket, FLAG_PT_SET_FLAG_RES,
                          response, 2);
   }

   // If flag was set successfully, schedule reset after a brief delay
   if(status == FLAG_SET_SUCCESS)
   {
      TRACE_INFO("DoIP: Flag set successfully\r\n");
   }
}

// ========================================
// RESET HANDLING FUNCTIONS
// ========================================

/**
 * @brief Handle Reset Request (0x2000)
 *
 * Expected payload format:
 * [Reset_type(1 byte)] = 0x01 for Hard Reset, 0x02/0x03 for Soft Reset
 */
void DoIP_Handle_ResetRequest(const uint8_t *payload, uint32_t payloadLen)
{
   uint8_t response[2]; // [Status(1 byte)][Reserved(1 byte)]
   uint8_t resetType = 0;
   uint8_t status = 0x00; // Success

   TRACE_INFO("DoIP: Received RESET request (payload len=%lu)\r\n",
              (unsigned long)payloadLen);

   // Parse reset type from payload
   if(payloadLen < 1)
   {
      TRACE_ERROR("DoIP: RESET payload too short (%u bytes)\r\n", (unsigned)payloadLen);
      status = 0x01; // Invalid value
   }
   else
   {
      resetType = payload[0];

      TRACE_INFO("DoIP: RESET request with type 0x%02X\r\n", resetType);

      // Validate reset type
      if(resetType != 0x01 && resetType != 0x02 && resetType != 0x03 && resetType != 0x04)
      {
         TRACE_ERROR("DoIP: Invalid reset type 0x%02X\r\n", resetType);
         status = 0x01; // Invalid value
      }
      else
      {
         TRACE_INFO("DoIP: Reset type 0x%02X is valid, calling reset handler\r\n", resetType);
         status = 0x00; // Success
      }
   }

   // Build response
   response[0] = status;
   response[1] = 0x00; // Reserved

   TRACE_INFO("DoIP: Sending RESET response with status 0x%02X\r\n", status);

   if(g_currentClientSocket != NULL)
   {
      (void)DoIP_SendFrame(g_currentClientSocket, RESET_PT_RESET_RES,
                          response, 2);
   }

   // If reset was accepted, call the reset request handler
   if(status == 0x00 && payloadLen >= 1)
   {
      TRACE_INFO("DoIP: Invoking reset handler for type 0x%02X\r\n", resetType);
      SoftwareReset();
   }
}


//ADDED 25/4/2026 modified in 11/5/2026
static bool_t DoIP_ParseFeatureLicenseToken(const uint8_t *buffer,
                                            uint32_t size,
                                            LicenseToken *token)
{
   uint16_t i;

   if((buffer == NULL) || (token == NULL))
      return FALSE;

   if(size != LICENSE_TOKEN_PAYLOAD_SIZE)
      return FALSE;

   token->license_id =
      ((uint32_t)buffer[0] << 24U) |
      ((uint32_t)buffer[1] << 16U) |
      ((uint32_t)buffer[2] << 8U)  |
      ((uint32_t)buffer[3]);

   token->ecu_id =
      ((uint32_t)buffer[4] << 24U) |
      ((uint32_t)buffer[5] << 16U) |
      ((uint32_t)buffer[6] << 8U)  |
      ((uint32_t)buffer[7]);

   token->feature_id =
      ((uint16_t)buffer[8] << 8U) |
      ((uint16_t)buffer[9]);

   token->valid_from =
      ((uint32_t)buffer[10] << 24U) |
      ((uint32_t)buffer[11] << 16U) |
      ((uint32_t)buffer[12] << 8U)  |
      ((uint32_t)buffer[13]);

   token->valid_until =
      ((uint32_t)buffer[14] << 24U) |
      ((uint32_t)buffer[15] << 16U) |
      ((uint32_t)buffer[16] << 8U)  |
      ((uint32_t)buffer[17]);

   token->issuer_id = buffer[18];

   for(i = 0U; i < LICENSE_SIGNATURE_B64_SIZE; i++)
   {
      token->signature_base64[i] = (char)buffer[19U + i];
   }

   token->signature_base64[LICENSE_SIGNATURE_B64_SIZE - 1U] = '\0';

   /* task_id is AFTER signature */
   token->task_id =
      ((uint16_t)buffer[419] << 8U) |
      ((uint16_t)buffer[420]);

   TRACE_INFO(" signature strlen  = %u\r\n",
              (unsigned)strlen(token->signature_base64));

   TRACE_INFO("DoIP FoD Token Parsed:\r\n");
   TRACE_INFO(" license_id       = 0x%08X\r\n", (unsigned)token->license_id);
   TRACE_INFO(" ecu_id           = 0x%08X\r\n", (unsigned)token->ecu_id);
   TRACE_INFO(" feature_id       = 0x%04X\r\n", (unsigned)token->feature_id);
   TRACE_INFO(" task_id          = 0x%04X\r\n", (unsigned)token->task_id);
   TRACE_INFO(" valid_from       = 0x%08X\r\n", (unsigned)token->valid_from);
   TRACE_INFO(" valid_until      = 0x%08X\r\n", (unsigned)token->valid_until);
   TRACE_INFO(" issuer_id        = 0x%02X\r\n", (unsigned)token->issuer_id);
   TRACE_INFO(" signature_base64 = %s\r\n", token->signature_base64);

   return TRUE;
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

         // Store socket for flag response handling
         g_currentClientSocket = clientSock;

         UDS_ResetProgrammingState();

         socketSetTimeout(clientSock, 60000);

         while(1)
         {
            error = DoIP_ReadFrame(clientSock, &hdr, payload, sizeof(payload));

            if(error == NO_ERROR)
            {
               TRACE_INFO("DoIP frame: type=0x%04X len=%lu\r\n",
                          hdr.payloadType, (unsigned long)hdr.payloadLength);

               // ===================================================================
               // DIAGNOSTIC MESSAGE HANDLING (0x8001) - MINIMAL
               // ===================================================================
               if(hdr.payloadType == DOIP_PT_DIAG_MSG)
               {
                  TRACE_INFO("DoIP: Diagnostic message received (0x8001)\r\n");

                  // Set bootloader flag
                  TRACE_INFO("DoIP: Writing bootloader flag\r\n");
                  if(DoIP_WriteFlagToMemory(BOOTLOADER_FLAG_VALUE) == 0)
                  {
                     TRACE_INFO("DoIP: Bootloader flag set successfully\r\n");
                  }
                  else
                  {
                     TRACE_ERROR("DoIP: Failed to set bootloader flag\r\n");
                  }

                  // Reset immediately
                  TRACE_INFO("DoIP: Executing reset\r\n");
                 SoftwareReset();

                  // Should not reach here
                  continue;
               }

               // ===================================================================
               // ALIVE CHECK HANDLING (0x0007)
               // ===================================================================
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

               // ===================================================================
               // ROUTING ACTIVATION HANDLING (0x0005)
               // ===================================================================
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

                  /*
                   * ISO 13400-2 Table 25 – Routing Activation Response payload
                   * must be 9 bytes minimum:
                   *   byte[0..1] : tester logical address  (echoed back)
                   *   byte[2..3] : ECU logical address
                   *   byte[4]    : response code – 0x10 = routing successfully activated
                   *   byte[5..8] : reserved_iso – must be 0x00000000
                   */
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

               /* =================================================================== */
               /* FEATURE ENABLE REQUEST HANDLING - TEMPORARY DIRECT DOIP PATH         */
               /* Payload type: 0x4100                                                */
               /* Payload: LicenseToken only                                          */
               /* =================================================================== */
               if(hdr.payloadType == DOIP_PT_FEATURE_ENABLE_REQ)
               {
                  LicenseToken token;
                  uint8_t response[4];

                  TRACE_INFO("DoIP FoD: FEATURE ENABLE request received\r\n");

                  if(!DoIP_ParseFeatureLicenseToken(payload, hdr.payloadLength, &token))
                  {
                     response[0] = FEATURE_STATUS_INVALID_LEN;
                     response[1] = 0x00U;
                     response[2] = 0x00U;
                     response[3] = 0x00U;

                     TRACE_ERROR("DoIP FoD: invalid enable token length=%lu\r\n",
                                 (unsigned long)hdr.payloadLength);
                  }
                  else if(!enable_By_request(&gFeatureManager, token.task_id, &token))
                  {
                     response[0] = FEATURE_STATUS_FAILED;
                     response[1] = (uint8_t)(token.task_id >> 8U);
                     response[2] = (uint8_t)(token.task_id & 0xFFU);
                     response[3] = 0x00U;

                     TRACE_ERROR("DoIP FoD: enable failed feature_id=%u\r\n",
                                 (unsigned)token.task_id);
                  }
                  else
                  {
                     response[0] = FEATURE_STATUS_SUCCESS;
                     response[1] = (uint8_t)(token.task_id >> 8U);
                     response[2] = (uint8_t)(token.task_id & 0xFFU);
                     response[3] = 0x00U;

                     TRACE_INFO("DoIP FoD: feature enabled feature_id=%u\r\n",
                                (unsigned)token.task_id);
                  }


                  (void)DoIP_SendFrame(clientSock,
                                       DOIP_PT_FEATURE_ENABLE_RES,
                                       response,
                                       sizeof(response));

                  if(response[0] == FEATURE_STATUS_SUCCESS)
                  {
                     TRACE_INFO("***************************************DoIP FoD: waiting before reset so feature state/LED can update**********************************************************************\r\n");

                    osDelayTask(3000);   // or 1000 ms for testing

                     SoftwareReset();
                  }
                  continue;
               }

               /* =================================================================== */
               /* FEATURE DISABLE REQUEST HANDLING - TEMPORARY DIRECT DOIP PATH        */
               /* Payload type: 0x4200                                               */
               /* Payload: LicenseToken only                                          */
               /* =================================================================== */
               if(hdr.payloadType == DOIP_PT_FEATURE_DISABLE_REQ)
               {
                  LicenseToken token;
                  uint8_t response[4];

                  TRACE_INFO("DoIP FoD: FEATURE DISABLE request received\r\n");

                  if(!DoIP_ParseFeatureLicenseToken(payload, hdr.payloadLength, &token))
                  {
                     response[0] = FEATURE_STATUS_INVALID_LEN;
                     response[1] = 0x00U;
                     response[2] = 0x00U;
                     response[3] = 0x00U;

                     TRACE_ERROR("DoIP FoD: invalid disable token length=%lu\r\n",
                                 (unsigned long)hdr.payloadLength);
                  }
                  else if(!disable_feature_By_request(&gFeatureManager, token.task_id, &token))
                  {
                     response[0] = FEATURE_STATUS_FAILED;
                     response[1] = (uint8_t)(token.task_id >> 8U);
                     response[2] = (uint8_t)(token.task_id & 0xFFU);
                     response[3] = 0x00U;

                     TRACE_ERROR("DoIP FoD: disable failed feature_id=%u\r\n",
                                 (unsigned)token.task_id);
                  }
                  else
                  {
                     response[0] = FEATURE_STATUS_SUCCESS;
                     response[1] = (uint8_t)(token.task_id >> 8U);
                     response[2] = (uint8_t)(token.task_id & 0xFFU);
                     response[3] = 0x00U;

                     TRACE_INFO("DoIP FoD: feature disabled feature_id=%u\r\n",
                                (unsigned)token.task_id);
                  }

                  (void)DoIP_SendFrame(clientSock,
                                       DOIP_PT_FEATURE_DISABLE_RES,
                                       response,
                                       sizeof(response));

                  if(response[0] == FEATURE_STATUS_SUCCESS)
                  {
                     TRACE_INFO("*********************************************DoIP FoD: waiting before reset after disable************************************************************************************\r\n");

                     osDelayTask(3000);

                     SoftwareReset();
                  }
                  continue;
               }

               /* =================================================================== */
               /* DIAGNOSTIC DATA REQUEST HANDLING - TEMPORARY DIRECT DOIP PATH       */
               /* Payload type: 0x4300                                                */
               /* Payload: CAR_ID                                          			  */
               /* =================================================================== */


               if (hdr.payloadType == DOIP_PT_DIAGNOSTIC_REQ)
               {
                   DiagnosticData data;
                   DiagnosticToken token;
                   DiagnosticResponse resp;

                   TRACE_INFO("DoIP DIAG: request received\r\n");

                   /* Validate payload size */
                   if (hdr.payloadLength != sizeof(DiagnosticToken))
                   {
                       memset(&resp, 0, sizeof(resp));
                       resp.status = DIAG_STATUS_FAILED;

                       DoIP_SendFrame(clientSock,
                                      DOIP_PT_DIAGNOSTIC_RES,
                                      (uint8_t*)&resp,
                                      sizeof(resp));
                       continue;
                   }

                   /* Extract request safely */
                   memcpy(&token, payload, sizeof(DiagnosticToken));

                   token.car_id[sizeof(token.car_id) - 1] = '\0';  // safety

                   TRACE_INFO("DoIP DIAG: car_id=%s\r\n", token.car_id);

                   if (Diagnostic_Read(token.car_id, &data))
                   {
                       resp.status = DIAG_STATUS_SUCCESS;

                       /* Full copy (no truncation) */
                       memset(resp.car_id, 0, sizeof(resp.car_id));
                       memcpy(resp.car_id, data.car_id, sizeof(resp.car_id));

                       /* Full 16-bit values (no loss) */
                       resp.temperature = data.temperature;
                       resp.pressure    = data.pressure;
                   }
                   else
                   {
                       resp.status = DIAG_STATUS_INVALID_ID;

                       memset(resp.car_id, 0, sizeof(resp.car_id));
                       resp.temperature = 0;
                       resp.pressure = 0;
                   }

                   DoIP_SendFrame(clientSock,
                                  DOIP_PT_DIAGNOSTIC_RES,
                                  (uint8_t*)&resp,
                                  sizeof(resp));

                   continue;
               }



               // Reset request handling (0x2000)
               if(hdr.payloadType == RESET_PT_RESET_REQ)
               {
                  DoIP_Handle_ResetRequest(payload, hdr.payloadLength);
                  continue;
               }

               // Other payload types: ignore for now
               TRACE_INFO("DoIP: Ignoring payload type 0x%04X\r\n", hdr.payloadType);
               continue;
            }

            // Error handling
            if(error == ERROR_END_OF_STREAM)
            {
               TRACE_INFO("DoIP: peer closed connection\r\n");
               break;
            }
            else if(error == ERROR_TIMEOUT)
            {
               // Timeout is normal - just continue the loop
               continue;
            }
            else
            {
               TRACE_INFO("DoIP: read error (%d)\r\n", error);
               break;
            }
         }

         socketClose(clientSock);
         g_currentClientSocket = NULL;
         TRACE_INFO("DoIP: client disconnected\r\n");
      }

      osDelayTask(10);
   }
}
