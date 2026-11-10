#include "UDS.h"
#include <string.h>   // memset, memcpy
#include "debug.h"

#ifdef UDS_BOOTLOADER

#endif

SECURITY_ACCESS_STATE g_secState = SEED_STATE;

UDS_SID SID = DIAGNOSTIC_SESSION_CONTROL;
UDS_SID prev_SID = DIAGNOSTIC_SESSION_CONTROL;

DIAGNOSTIC_SESSION_SUBFUNC currentSession = DEFAULT_SESSION;
__attribute__((section(".noinit"))) volatile uint32_t initVAR;

// UDS Buffers (assumes dataBuffer is a fixed array inside dataFrame)
dataFrame requestFrame  = {0, {0}, 0};
dataFrame responseFrame = {0, {0}, 0};

// TransferData state
static uint8_t  g_seqNumber = 1;
static uint32_t g_remainingData = 0;
static uint32_t g_maxNumberBlockLength = MAX_BLOCK_NUMBER+2u; //  modified added 2  4.3.2026

// Bootloader shared data/callbacks
static BL_Data BL_data = {0};
static BL_Functions *BL_Callbacks = NULL;
static uint8_t BL_Func_missing = 0;

// SecurityAccess state
static uint32_t sec_access_seed = 0x00000000;
static ECU_UNLOCK_STATE ECU_unlock_state = LOCKED;

// ------------------------------------------------------------
// Local helpers
// ------------------------------------------------------------
static void reset_dataframe(dataFrame *frame)
{
    if(!frame) return;
    frame->ready = 0;
    memset(frame->dataBuffer, 0, MAX_BUFF_SIZE);
    frame->dataSize = 0;
}

static void reqDownloadAbort(void)
{
    SID = prev_SID;
    prev_SID = DIAGNOSTIC_SESSION_CONTROL;
    g_remainingData = 0;
}

static void transferDataAbort(void)
{
    SID = REQUEST_TRANSFER_EXIT;
    prev_SID = DIAGNOSTIC_SESSION_CONTROL;
    g_remainingData = 0;
    g_seqNumber = 1;
}

static DID_Found_Status checkIfIDExists(DID ID)
{
    switch (ID)
    {
        case CURR_SESSION_ID: return FOUND;
        default:              return NOT_FOUND;
    }
}

static uint8_t get_data_by_ID(DID ID)
{
    switch (ID)
    {
        case CURR_SESSION_ID: return (uint8_t)currentSession;
        default:              return (uint8_t)NOT_DEFINED_ID;
    }
}

static void write_data_by_ID(DID ID, uint8_t data)
{
    (void)ID;
    (void)data;
    // Not implemented yet
}

// ------------------------------------------------------------
// Public: response builders
// ------------------------------------------------------------
void UDS_Create_pos_response(uint8_t isReady)
{
    // Minimal positive response: [SID+0x40][subfn]
    responseFrame.dataBuffer[SID_POS]     = (uint8_t)(requestFrame.dataBuffer[SID_POS] + 0x40);
    responseFrame.dataBuffer[SUB_BYTE_POS] = requestFrame.dataBuffer[SUB_BYTE_POS];
    responseFrame.dataSize = 2;
    responseFrame.ready = isReady;
}

static void UDS_Create_pos_response_sid_only(uint8_t isReady) // not all function has subFN
{
    responseFrame.dataBuffer[0] = (uint8_t)(requestFrame.dataBuffer[0] + 0x40);
    responseFrame.dataSize = 1;
    responseFrame.ready = isReady;
}

void UDS_Create_neg_response(NRC neg_code, uint8_t isReady)
{
    responseFrame.dataBuffer[0] = 0x7F;
    responseFrame.dataBuffer[1] = requestFrame.dataBuffer[SID_POS];
    responseFrame.dataBuffer[2] = (uint8_t)neg_code;
    responseFrame.dataSize = 3;
    responseFrame.ready = isReady;
}

// ------------------------------------------------------------
// Security algorithm
// ------------------------------------------------------------
uint32_t generate_recommended_key(uint32_t seed, uint8_t security_level)
{
    uint32_t level_masks[] = {
        0x00000000,
        0x5A5A5A5A,
        0xA5A5A5A5,
        0x12345678,
        0x87654321
    };

    uint32_t key = seed;
    uint8_t rotation = (security_level * 3u) % 32u;
    key = (key << rotation) | (key >> (32u - rotation));
    key ^= level_masks[security_level];
    key = ((key & 0xAAAAAAAAu) >> 1) | ((key & 0x55555555u) << 1);
    key ^= ~seed;
    return key;
}

// for now so we can rerun many time 5/3/2026
void UDS_ResetProgrammingState(void)
{
    g_secState = SEED_STATE;
    ECU_unlock_state = LOCKED;
    sec_access_seed = 0;
    g_seqNumber = 1;
    g_remainingData = 0;

    TRACE_INFO("UDS: ResetProgrammingState called\r\n");
}



// ------------------------------------------------------------
// Services
// ------------------------------------------------------------
void UDS_Security_Access(void)
{
    if(g_secState == SEED_STATE && requestFrame.dataSize != SEC_ACCESS_SEED_REQ_LEN)
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    if(g_secState == KEY_STATE && requestFrame.dataSize != SEC_ACCESS_KEY_REQ_LEN)
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    SECURITY_ACCESS_REQ_TYPE req_type =
        (SECURITY_ACCESS_REQ_TYPE)requestFrame.dataBuffer[SEC_ACCESS_REQ_TYPE_POS];

    if(g_secState == SEED_STATE && req_type == KEY_REQ)
    {
        UDS_Create_neg_response(REQ_SEQ_ERROR, READY);
        return;
    }

    if(req_type == SEED_REQ)
    {
        srand(*((uint32_t *)(0x40040010)));
        sec_access_seed = (uint32_t)rand();

        UDS_Create_pos_response(NOTREADY);
        responseFrame.dataBuffer[2] = (sec_access_seed >> 24) & 0xFF;
        responseFrame.dataBuffer[3] = (sec_access_seed >> 16) & 0xFF;
        responseFrame.dataBuffer[4] = (sec_access_seed >> 8)  & 0xFF;
        responseFrame.dataBuffer[5] = (sec_access_seed)       & 0xFF;
        responseFrame.dataSize = 6;
        responseFrame.ready = READY;

        g_secState = KEY_STATE;
    }
    else // KEY_REQ
    {
        uint32_t key = 0;
        for(uint8_t i = 0; i < 4; i++)
        {
            key <<= 8;
            key |= requestFrame.dataBuffer[SEC_ACCESS_KEY_START_BYTE + i];
        }

        if(key == generate_recommended_key(sec_access_seed, 1))
        {
            ECU_unlock_state = UNLOCKED;
            UDS_Create_pos_response(NOTREADY);
            responseFrame.ready = READY;
        }
        else
        {
            UDS_Create_neg_response(INVALID_KEY, READY);
        }

        g_secState = SEED_STATE;
    }
}

void UDS_Session_Control(void)
{
    if(requestFrame.dataSize != 2)
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    DIAGNOSTIC_SESSION_SUBFUNC requested =
        (DIAGNOSTIC_SESSION_SUBFUNC)requestFrame.dataBuffer[SUB_BYTE_POS];

    if(requested != DEFAULT_SESSION && requested != PROGRAMMING_SESSION)
    {
        UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
        return;
    }

    currentSession = requested;

    // reset transfer state on session change
    g_seqNumber = 1;
    g_remainingData = 0;

    // leaving programming -> lock again
    if(currentSession == DEFAULT_SESSION)
        ECU_unlock_state = LOCKED;

    // Positive response: 50 <session> P2 P2*
    responseFrame.dataBuffer[0] = (uint8_t)(DIAGNOSTIC_SESSION_CONTROL + 0x40);
    responseFrame.dataBuffer[1] = (uint8_t)requested;

    // Pick any stable values for now (ms):
    // P2=50ms (0x0032), P2*=5000ms (0x1388)
    responseFrame.dataBuffer[2] = 0x00;
    responseFrame.dataBuffer[3] = 0x32;
    responseFrame.dataBuffer[4] = 0x13;
    responseFrame.dataBuffer[5] = 0x88;

    responseFrame.dataSize = 6;
    responseFrame.ready = READY;
}

void UDS_ECU_Reset(void)
{
    // Keep it callable but avoid CAN dependencies during DoIP testing
    if(requestFrame.dataSize < ECU_RESET_MIN_SIZE)
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    ECU_RESET_SUBFUNC requested_reset = (ECU_RESET_SUBFUNC)requestFrame.dataBuffer[SUB_BYTE_POS];

    if(requested_reset != SOFT_RESET && requested_reset != HARD_RESET)
    {
        UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
        return;
    }

    // Send positive response first
    UDS_Create_pos_response(NOTREADY);
    responseFrame.ready = READY;

#ifdef UDS_ENABLE_HW_RESET
    // Put your old reset code here if/when you want it enabled.
    // For now, keep disabled so you can test over DoIP without rebooting.
    (void)requested_reset;
#else
    (void)requested_reset;
#endif
}

void UDS_Read_by_ID(void)
{
    if(requestFrame.dataSize < 4)
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    uint16_t requested_ID =
        ((uint16_t)requestFrame.dataBuffer[DID_HIGH_BYTE_POS] << 8) |
         (uint16_t)requestFrame.dataBuffer[DID_LOW_BYTE_POS];

    if(checkIfIDExists((DID)requested_ID) == NOT_FOUND)
    {
        UDS_Create_neg_response(REQ_OUT_OF_RANGE, READY);
        return;
    }

    uint8_t data = get_data_by_ID((DID)requested_ID);

    // Positive response: [SID+0x40][DID_H][DID_L][DATA...]
    responseFrame.dataBuffer[0] = (uint8_t)(READ_DATA_BY_ID + 0x40);
    responseFrame.dataBuffer[1] = requestFrame.dataBuffer[DID_HIGH_BYTE_POS];
    responseFrame.dataBuffer[2] = requestFrame.dataBuffer[DID_LOW_BYTE_POS];
    responseFrame.dataBuffer[3] = data;
    responseFrame.dataSize = 4;
    responseFrame.ready = READY;
}

void UDS_Write_by_ID(void)
{
    if(requestFrame.dataSize < 5)
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    uint16_t requested_ID =
        ((uint16_t)requestFrame.dataBuffer[DID_HIGH_BYTE_POS] << 8) |
         (uint16_t)requestFrame.dataBuffer[DID_LOW_BYTE_POS];

    if(checkIfIDExists((DID)requested_ID) == NOT_FOUND)
    {
        UDS_Create_neg_response(REQ_OUT_OF_RANGE, READY);
        return;
    }

    uint8_t data = requestFrame.dataBuffer[DATA_START_POS];
    write_data_by_ID((DID)requested_ID, data);

    // Positive response echoes DID
    responseFrame.dataBuffer[0] = (uint8_t)(WRITE_DATA_BY_ID + 0x40);
    responseFrame.dataBuffer[1] = requestFrame.dataBuffer[DID_HIGH_BYTE_POS];
    responseFrame.dataBuffer[2] = requestFrame.dataBuffer[DID_LOW_BYTE_POS];
    responseFrame.dataSize = 3;
    responseFrame.ready = READY;
}

void UDS_Request_Download(void) // modified
{
    if(!(currentSession == PROGRAMMING_SESSION))
    {
        UDS_Create_neg_response(UPLOAD_DOWNLOAD_NOT_ACCEPTED, READY);
        return;
    }

    if(BL_Callbacks == NULL)
    {
        BL_Func_missing = 1;
        UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
        return;
    }

    if(requestFrame.dataSize < 3)
    {
        reqDownloadAbort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    BL_data.compression_flag = requestFrame.dataBuffer[1] >> 4;
    uint8_t nBytesLen = requestFrame.dataBuffer[2] >> 4;
    uint8_t addrSize  = requestFrame.dataBuffer[2] & 0x0F;

    if(addrSize < 1 || addrSize > ECU_ADDRESS_LENGTH || nBytesLen < 1 || nBytesLen > 4)
    {
        reqDownloadAbort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    uint8_t expected = (uint8_t)(3 + nBytesLen + addrSize);
    if(requestFrame.dataSize != expected)
    {
        reqDownloadAbort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    BL_data.mem_start_address = 0;
    BL_data.total_size = 0;

    for(uint8_t i = 3; i < (uint8_t)(3 + addrSize); i++)
    {
        BL_data.mem_start_address = (BL_data.mem_start_address << 8) | requestFrame.dataBuffer[i];
    }

    for(uint8_t i = (uint8_t)(3 + addrSize); i < expected; i++)
    {
        BL_data.total_size = (BL_data.total_size << 8) | requestFrame.dataBuffer[i];
    }

    // Prepare transfer state
    g_seqNumber = 1;
    g_remainingData = BL_data.total_size;
    BL_data.request_flag = 1;

    // Positive response: [0x74][lenFormatId][MaxNumberBlockLength(2)]
    responseFrame.dataBuffer[0] = (uint8_t)(REQUEST_DOWNLOAD + 0x40);
    responseFrame.dataBuffer[1] = 0x20;
    responseFrame.dataBuffer[2] = (uint8_t)(g_maxNumberBlockLength >> 8);
    responseFrame.dataBuffer[3] = (uint8_t)(g_maxNumberBlockLength);
    responseFrame.dataSize = 4;
    responseFrame.ready = READY;
}

void UDS_Transfer_Data(void)  // modified
{
    if(requestFrame.dataSize < 2)
    {
        transferDataAbort();
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    uint8_t  seq     = requestFrame.dataBuffer[1];
    uint32_t dataLen = (uint32_t)(requestFrame.dataSize - 2);

    // Max block length check
    if(requestFrame.dataSize > g_maxNumberBlockLength)
{
    transferDataAbort();
    UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
    return;
}

    // don't accept more than expected remaining
    if(dataLen > g_remainingData)
    {
        transferDataAbort();
        UDS_Create_neg_response(REQ_OUT_OF_RANGE, READY); // NRC 0x31
        return;
    }

    // Sequence check — modified 14.3.2026
    // Old code accepted seq==0 after 255, but the script wraps 255->1 (never sends 0).
    // Simple equality check is correct; wrap behaviour is enforced by the update below.
    if(seq != g_seqNumber)                                                  // modified 14.3.2026
    {
        transferDataAbort();
        UDS_Create_neg_response(REQ_SEQ_ERROR, READY);
        return;
    }

    BL_data.data_block_size = (uint16_t)dataLen;
    memcpy(BL_data.data, &requestFrame.dataBuffer[2], dataLen);

    g_remainingData -= dataLen;

    if(BL_Callbacks == NULL || BL_Callbacks->BL_TransferDataHandler == NULL)
    {
        BL_Func_missing = 1;
        UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
        return;
    }
    BL_Func_missing = 0;

    BL_data.request_flag = 0;   // 16/3/2026
    status_t st = BL_Callbacks->BL_TransferDataHandler();
    if(st == STATUS_ERROR)
    {
        transferDataAbort();
        UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
        return;
    }

    responseFrame.dataBuffer[0] = (uint8_t)(TRANSFER_DATA + 0x40);
    responseFrame.dataBuffer[1] = seq;          // echo received seq
    responseFrame.dataSize = 2;
    responseFrame.ready = READY;

    // Wrap 255->1 to match the script's: bsc = (bsc % 255) + 1   modified 14.3.2026
    g_seqNumber = (g_seqNumber >= 255u) ? 1u : (uint8_t)(g_seqNumber + 1u); // modified 14.3.2026
}

void UDS_Request_Transfer_Exit(void)
{
    if(requestFrame.dataSize != 1)
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    if(g_remainingData != 0)
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    g_seqNumber = 1;
    BL_data.request_flag = 1;

    responseFrame.dataBuffer[0] = (uint8_t)(REQUEST_TRANSFER_EXIT + 0x40);
    responseFrame.dataBuffer[1] = 0x00;
    responseFrame.dataSize = 1;
    responseFrame.ready = READY;
}

void UDS_Routine_Control(void)
{
    // Minimum expected: [SID][ctrlType][ridHi][ridLo][...]
    if(requestFrame.dataSize < ROUTINE_CTRL_SIZE) // ROUTINE_CTRL_SIZE = 5 in your UDS.h
    {
        UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
        return;
    }

    if(BL_Callbacks == NULL)
    {
        BL_Func_missing = 1;
        UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
        return;
    }

    uint8_t ctrlType = requestFrame.dataBuffer[1];
    uint16_t routineId = ((uint16_t)requestFrame.dataBuffer[2] << 8) |
                          (uint16_t)requestFrame.dataBuffer[3];

    // Only implement START_ROUTINE for now (enough for your sequence)
    if(ctrlType != START_ROUTINE)
    {
        UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
        return;
    }

    // -------------------------
    // ERASE_MEMORY: 31 01 FF00
    // Format (based on your old code):
    // [31][01][FF][00][addrLenFormat][addr...][len...]
    // addrLenFormat: high nibble = length bytes count, low nibble = address bytes count
    // -------------------------
    if(routineId == ERASE_MEMORY)
    {
        if(requestFrame.dataSize < 5)
        {
            UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
            return;
        }

        uint8_t nBytesLen = (uint8_t)(requestFrame.dataBuffer[4] >> 4);
        uint8_t addrSize  = (uint8_t)(requestFrame.dataBuffer[4] & 0x0F);

        if(addrSize < 1 || addrSize > ECU_ADDRESS_LENGTH || nBytesLen < 1 || nBytesLen > 4)
        {
            UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
            return;
        }

        uint8_t expected = (uint8_t)(5 + addrSize + nBytesLen);
        if(requestFrame.dataSize != expected)
        {
            UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
            return;
        }

        BL_data.ers_mem_start_address = 0;
        BL_data.ers_total_size = 0;

        // parse start address
        for(uint8_t i = 5; i < (uint8_t)(5 + addrSize); i++)
        {
            BL_data.ers_mem_start_address = (BL_data.ers_mem_start_address << 8) | requestFrame.dataBuffer[i];
        }

        // parse length
        for(uint8_t i = (uint8_t)(5 + addrSize); i < expected; i++)
        {
            BL_data.ers_total_size = (BL_data.ers_total_size << 8) | requestFrame.dataBuffer[i];
        }

        // optional: align erase size to 4 bytes (like your old code)
        uint32_t rem = (BL_data.ers_total_size % 4u);
        if(rem) BL_data.ers_total_size += (4u - rem);

        if(BL_Callbacks->BL_Erase_Memory == NULL)
        {
            BL_Func_missing = 1;
            UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
            return;
        }
        BL_Func_missing = 0;
        BL_data.request_flag = 1; // added

        status_t st = BL_Callbacks->BL_Erase_Memory();
        if(st != STATUS_SUCCESS)
        {
            UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
            return;
        }

        // Positive response: [0x71][0x01][ridHi][ridLo]
        responseFrame.dataBuffer[0] = (uint8_t)(ROUTINE_CONTROL + 0x40);
        responseFrame.dataBuffer[1] = START_ROUTINE;
        responseFrame.dataBuffer[2] = requestFrame.dataBuffer[2];
        responseFrame.dataBuffer[3] = requestFrame.dataBuffer[3];
        responseFrame.dataSize = 4;
        responseFrame.ready = READY;
        return;
    }

    // -------------------------
    // CHECK_MEMORY: 31 01 FF01 + CRC(4 bytes)
    // We'll implement the common 4-byte CRC case:
    // [31][01][FF][01][CRC3][CRC2][CRC1][CRC0]
    // -------------------------
    if(routineId == CHECK_MEMORY)
    {
        if(requestFrame.dataSize != 8)
        {
            UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
            return;
        }

        BL_data.CRC_Field = 0;
        for(uint8_t i = 4; i < 8; i++)
        {
            BL_data.CRC_Field = (BL_data.CRC_Field << 8) | requestFrame.dataBuffer[i];
        }

        if(BL_Callbacks->BL_Check_Memory == NULL)
        {
            BL_Func_missing = 1;
            UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
            return;
        }
        BL_Func_missing = 0;
        BL_data.request_flag = 1; // added

        status_t st = BL_Callbacks->BL_Check_Memory();
        if(st != STATUS_SUCCESS)
        {
            UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
            return;
        }

        responseFrame.dataBuffer[0] = (uint8_t)(ROUTINE_CONTROL + 0x40);
        responseFrame.dataBuffer[1] = START_ROUTINE;
        responseFrame.dataBuffer[2] = requestFrame.dataBuffer[2];
        responseFrame.dataBuffer[3] = requestFrame.dataBuffer[3];
        responseFrame.dataSize = 4;
        responseFrame.ready = READY;
        return;
    }

    // -------------------------
    // FINALIZE_PROGRAMMING: 31 01 FF02 [signature type + signature(optional)]
    // UDS.h defines SIGNATURE_TYPE.
    // We'll support:
    // - NO_SIGNATURE (0x00): just finalize
    // - ECDSA_SIGNATURE (0x01): copy next 64 bytes into BL_data.signature
    // -------------------------
    if(routineId == FINALIZE_PROGRAMMING)
    {
        if(BL_Callbacks->BL_Finalize_Programming == NULL)
        {
            BL_Func_missing = 1;
            UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
            return;
        }
        BL_Func_missing = 0;

        // requestFrame format:
        // [31][01][FF][02][sigType][sig bytes...]
        uint8_t sigType = requestFrame.dataBuffer[4];

        if(sigType == NO_SIGNATURE)
        {
            // ok, no extra bytes required
            if(requestFrame.dataSize != 5)
            {
                UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
                return;
            }
        }
        else if(sigType == ECDSA_SIGNATURE)
        {
            // expect exactly 64 bytes signature
            if(requestFrame.dataSize != (5 + 64))
            {
                UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
                return;
            }

            memcpy(BL_data.signature, &requestFrame.dataBuffer[5], 64);
        }
        else
        {
            UDS_Create_neg_response(SUB_FUNC_NOT_SUPPORTED, READY);
            return;
        }

        BL_data.request_flag = 1; // added
        status_t st = BL_Callbacks->BL_Finalize_Programming();
        if(st != STATUS_SUCCESS)
        {
            UDS_Create_neg_response(GENERAL_PROGRAMMING_FAILURE, READY);
            return;
        }

        responseFrame.dataBuffer[0] = (uint8_t)(ROUTINE_CONTROL + 0x40);
        responseFrame.dataBuffer[1] = START_ROUTINE;
        responseFrame.dataBuffer[2] = requestFrame.dataBuffer[2];
        responseFrame.dataBuffer[3] = requestFrame.dataBuffer[3];
        responseFrame.dataSize = 4;
        responseFrame.ready = READY;
        return;
    }

    // Unknown routine ID
    UDS_Create_neg_response(REQ_OUT_OF_RANGE, READY);
}



void UDS_Check_Memory(status_t *status)
{
    if(!status) return;

    if(BL_Callbacks == NULL || BL_Callbacks->BL_Check_Memory == NULL)
    {
        BL_Func_missing = 1;
        *status = STATUS_ERROR;
        return;
    }

    BL_Func_missing = 0;

    *status = BL_Callbacks->BL_Check_Memory();
}

void UDS_Erase_Memory(status_t *status)
{
    if(!status) return;

    if(BL_Callbacks == NULL || BL_Callbacks->BL_Erase_Memory == NULL)
    {
        BL_Func_missing = 1;
        *status = STATUS_ERROR;
        return;
    }

    BL_Func_missing = 0;

    *status = BL_Callbacks->BL_Erase_Memory();
}

// ------------------------------------------------------------
// Init / Receive dispatcher
// ------------------------------------------------------------
void UDS_Init(BL_Functions *BL_Funcs)
{
    BL_Callbacks = BL_Funcs;
    BL_Func_missing = (BL_Callbacks == NULL);

    // Boot decision (same logic you had)
    if(initVAR == 0x0)
        currentSession = DEFAULT_SESSION;
    else
        currentSession = PROGRAMMING_SESSION;

    ECU_unlock_state = LOCKED;
    g_seqNumber = 1;
    g_remainingData = 0;
}

void UDS_Receive(void)
{
    if(requestFrame.ready != READY)
        return;

    if(requestFrame.dataSize == 0)
    {
        reset_dataframe(&requestFrame);
        return;
    }

    prev_SID = SID;
    SID = (UDS_SID)requestFrame.dataBuffer[SID_POS];

    switch(SID)
    {
        case DIAGNOSTIC_SESSION_CONTROL:
            UDS_Session_Control();
            break;

        case ECU_RESET:
            UDS_ECU_Reset();
            break;

        case READ_DATA_BY_ID:
            UDS_Read_by_ID();
            break;

        case WRITE_DATA_BY_ID:
            UDS_Write_by_ID();
            break;

        case SECURITY_ACCESS:
            if(currentSession == PROGRAMMING_SESSION)
                UDS_Security_Access();
            else
                UDS_Create_neg_response(CONDITIONS_NOT_CORRECT, READY);
            break;

        case REQUEST_DOWNLOAD:
            if(currentSession == PROGRAMMING_SESSION && ECU_unlock_state == UNLOCKED)
                UDS_Request_Download();
            else
                UDS_Create_neg_response(UPLOAD_DOWNLOAD_NOT_ACCEPTED, READY);
            break;

        case TRANSFER_DATA:
            if(currentSession == PROGRAMMING_SESSION && ECU_unlock_state == UNLOCKED)
                UDS_Transfer_Data();
            else
                UDS_Create_neg_response(CONDITIONS_NOT_CORRECT, READY);
            break;

        case REQUEST_TRANSFER_EXIT:
            if(currentSession == PROGRAMMING_SESSION && ECU_unlock_state == UNLOCKED)
                UDS_Request_Transfer_Exit();
            else
                UDS_Create_neg_response(CONDITIONS_NOT_CORRECT, READY);
            break;

        case ROUTINE_CONTROL:
            if(currentSession == PROGRAMMING_SESSION && ECU_unlock_state == UNLOCKED)
                UDS_Routine_Control();
            else
                UDS_Create_neg_response(CONDITIONS_NOT_CORRECT, READY);
            break;

        case TESTER_PRESENT:
            if(requestFrame.dataSize != 2)
                UDS_Create_neg_response(WRONG_MSG_LEN_OR_FORMAT, READY);
            else
                UDS_Create_pos_response(READY);
            break;

        default:
            UDS_Create_neg_response(SERVICE_NOT_SUPPORTED, READY);
            break;
    }

    // Always consume request
    reset_dataframe(&requestFrame);
}

BL_Data *UDS_BL_Receive(void)
{
    return &BL_data;
}



