#include "Bootloader.h"
#include "shared_state.h"
#include "debug.h"


//added to handle multiple blocks
static uint32_t flashoffset = 0;
static uint32_t firmwareStartAddress;
static bool g_firstBlock = false;

__attribute__((used, section(".boot_flags"))) volatile BootFlags_t g_bootFlags;

/* Global pointer to bootloader data received from UDS */
static BL_Data *gBootloaderData = NULL;

/* External function to receive bootloader data via UDS */
extern BL_Data *UDS_BL_Receive(void);

/* Tracks the most recent status of flash operations */
static status_t lastOperationStatus;

/* Cumulative size of data programmed into flash memory */
uint32_t totalProgramSize = 0;

/* Cumulative size of data programmed before padding into flash memory */
uint32_t totalActualSize = 0;

/* Indicates that programming should target the bootloader-update region */
static bool gEnterBootloaderUpdateMode = false;

/* Holds the start address for Bootloader_Program (remapped if update mode is active) */
static uint32_t programStartAddress;

/* size of the data block to program (raw or decompressed) */
static uint32_t blockSize = 0;

static uint8_t index = 0;

/* Global structure to store application-specific bootloader function handlers */
static BL_Functions gBootloaderHandlers = {
        .BL_TransferDataHandler  = NULL,
        .BL_Check_Memory         = NULL,
        .BL_Erase_Memory         = NULL,
        .BL_Finalize_Programming = NULL
};


// Initialize the bootloader with application-provided handlers and disable flash caches
status_t Bootloader_Init(BL_Functions *a_pBLHandlersConfig)
{
    /* Validate input handler configuration */
    if (a_pBLHandlersConfig == NULL ||
            a_pBLHandlersConfig->BL_TransferDataHandler == NULL ||
            a_pBLHandlersConfig->BL_Check_Memory        == NULL ||
            a_pBLHandlersConfig->BL_Erase_Memory        == NULL ||
            a_pBLHandlersConfig->BL_Finalize_Programming == NULL)
    {
        return STATUS_ERROR;
    }

    /* Store the provided bootloader function handlers for later use */
    gBootloaderHandlers = *a_pBLHandlersConfig;

    /* Disable flash controller caches to ensure safe flash operations */
    BootloaderFlash_DisableCache();

    /* Retrieve bootloader data via UDS and store it globally */
    gBootloaderData = UDS_BL_Receive();
    if (gBootloaderData == NULL)
    {
        return STATUS_ERROR;
    }

    /* Initialize the lower-level flash operations module */
    return BootloaderFlash_Init();
}


// Erase the flash region specified by UDS bootloader data and verify blank state
status_t Bootloader_Erase_Memory(void)
{
    /* Ensure that bootloader data is available */
    if (gBootloaderData == NULL)
    {
        return STATUS_ERROR;
    }


    /* Unlock flash memory for erase operation */
    lastOperationStatus = BootloaderFlash_Unlock();
    if (lastOperationStatus != STATUS_SUCCESS)
    {
        return lastOperationStatus;
    }

    /* Determine start address for erase, possibly remapped for update region */
    uint32_t eraseStartAddress = gBootloaderData->ers_mem_start_address;

    if ((eraseStartAddress >= BOOTMANAGER_START_ADDRESS && eraseStartAddress <= BOOTMANAGER_END_ADDRESS) ||
        (eraseStartAddress >= BOOTLOADER_START_ADDRESS  && eraseStartAddress <= BOOTLOADER_END_ADDRESS))
    {
        eraseStartAddress = RemapToUpdateRegion(eraseStartAddress);
        gEnterBootloaderUpdateMode  = true;
        gSharedState.updateModeFlag = 0xBBBBBBBB;

    }
    else
    {
        gSharedState.appValidFlag = 0x11111111;
        /* ---- SET FLAG: APP IS NOW INVALID ---- */
        g_bootFlags.validity_flag = APP_INVALID;
    }

    /* Erase flash memory starting at the specified address for the given size */
    lastOperationStatus = BootloaderFlash_Erase(eraseStartAddress, gBootloaderData->ers_total_size);
    if (lastOperationStatus != STATUS_SUCCESS)
    {
        return lastOperationStatus;
    }

    lastOperationStatus= BootloaderFlash_VerifyBlank(eraseStartAddress, gBootloaderData->ers_total_size);

    g_firstBlock = true;  // 16/3/2026
    /* Verify that the erased flash memory is blank */
    return lastOperationStatus;
}


// Program the flash memory with data blocks from UDS, decompress if compression flag is set, then verify
status_t Bootloader_Program(void)
{

    TRACE_INFO("BL_Program: g_firstBlock=%d flashoffset=%lu totalActual=%lu\r\n",
               (int)g_firstBlock,
               (unsigned long)flashoffset,
               (unsigned long)totalActualSize);

    /* Ensure that bootloader data is available */
    if (gBootloaderData == NULL)
    {
        return STATUS_ERROR;
    }

    if(g_firstBlock){
    	firmwareStartAddress = gBootloaderData->mem_start_address;
    	        totalProgramSize = 0;
    	        totalActualSize  = 0;
    	        flashoffset=0; // added 16/3/2026
    	        gBootloaderData->request_flag = 0; // next block skips the init
    	        g_firstBlock=0;
    }



    uint32_t *programDataBuffer;

    if (gBootloaderData->compression_flag == 1)
    {
        char decompressedBuffer[DECOMPRESSED_BUFFER_SIZE] = {0};
        volatile uint32_t decompressedBytesCount = BLDecomp_Decompress(
                (const uint8_t *)gBootloaderData->data,
                (uint8_t *)decompressedBuffer,
                gBootloaderData->data_block_size);

        blockSize       = decompressedBytesCount;
        totalActualSize += blockSize;

        uint8_t remainder = blockSize % 4;
        if (remainder)
        {
            uint8_t idx = blockSize;
            blockSize += (4 - remainder);
            while (remainder--)
            {
                decompressedBuffer[idx] = 0xFF;
                idx++;
            }
        }
        programDataBuffer = (uint32_t *)decompressedBuffer;
    }
    else
    {
        /* No compression: program directly from received data buffer */
        programDataBuffer = (uint32_t *)gBootloaderData->data;
        blockSize         = gBootloaderData->data_block_size;
        totalActualSize  += blockSize;

        uint8_t remainder = blockSize % 4;
        if (remainder)
            blockSize += (4 - remainder);
    }

    totalProgramSize += blockSize;

    /* Calculate address for programming, remap if update mode is active */
    programStartAddress = gBootloaderData->mem_start_address + flashoffset;
    if (gEnterBootloaderUpdateMode)
    {
        programStartAddress      = RemapToUpdateRegion(programStartAddress);
        gFlashedArray[index]     = programStartAddress;
        gFlashedArray[index + 1] = blockSize;
        index += 2;
    }

    /* Program flash memory with the provided data */
    lastOperationStatus = BootloaderFlash_Program(programStartAddress, blockSize, programDataBuffer);
    if (lastOperationStatus != STATUS_SUCCESS)
    {
        return lastOperationStatus;
    }

    /* Verify that flash programming was successful */
    lastOperationStatus = BootloaderFlash_ProgramVerify(programStartAddress, blockSize, programDataBuffer);

    /* Wait until program verification is no longer in progress */
    while (lastOperationStatus == STATUS_FLASH_INPROGRESS)
    {
        lastOperationStatus = BootloaderFlash_ProgramVerify(programStartAddress, blockSize, programDataBuffer);
    }

    flashoffset += blockSize;
    return lastOperationStatus;
}


// Verify programmed flash memory using CRC32
status_t Bootloader_CheckMemory(void)
{
    /* Ensure that bootloader data is available */
    if (gBootloaderData == NULL)
    {
        return STATUS_ERROR;
    }

    // 16/3/2026:
       TRACE_INFO("CRC check: addr=0x%08X size=%lu\r\n",
                  (unsigned)firmwareStartAddress,
                  (unsigned long)totalActualSize);

    /* Initialize CRC calculation module */
    BootloaderFlash_InitCRC();

    /* Compute CRC32 over the flash memory area defined by UDS bootloader data */
    uint32_t calculatedCrc=BootloaderFlash_CalculateCRC32(firmwareStartAddress,  totalActualSize);// 16/3/2026

    // 16/3/2026:
    TRACE_INFO("CRC calc=0x%08X expected=0x%08X\r\n",
                 (unsigned)calculatedCrc,
                 (unsigned)gBootloaderData->CRC_Field);

    /* Restore flash controller cache settings after flash operations */
    BootloaderFlash_RestoreCache();

    /* Store program info for BootManager (used when second core is active) */
    gSignatureArray[0]      = programStartAddress;
    gSignatureArray[1]      = (size_t)totalActualSize;
    gSharedState.sigControl = 0xAAAAAAAA;

    if (calculatedCrc == gBootloaderData->CRC_Field) {
    	if (gEnterBootloaderUpdateMode) {
    	            /* BOOTLOADER UPDATE SUCCESSFUL */
    	            /* Leave validity_flag alone. Set bootloader flag so Bootmanager knows to handle the new Bootloader image! */
    	            g_bootFlags.bootloader_flag = BOOTLOADER_FLAG_VALUE;
    	        }
    	        else {
    	            /* APPLICATION UPDATE SUCCESSFUL */
    	            /* App is valid and ready. Clear the bootloader flag so BootManager jumps to your App next reboot. */
    	            g_bootFlags.validity_flag = VALIDITY_FLAG_VALUE;
    	            g_bootFlags.bootloader_flag = BOOTLOADER_INVALID;
    	        }
            return STATUS_SUCCESS;
        } else {
        	/* CRC FAILED! */
			if (!gEnterBootloaderUpdateMode) {

				/* If App update failed, guarantee validity_flag stays invalid */
				g_bootFlags.validity_flag = APP_INVALID;
			}
            return STATUS_ERROR;
        };
}



status_t Bootloader_Finalize_Programming(void)
{
    /* Store signature bytes for when BootManager is available */
    for (int i = 0; i < 64; i += 4)
    {
        uint32_t word = ((uint32_t)gBootloaderData->signature[i])           |
                        ((uint32_t)gBootloaderData->signature[i + 1] << 8)  |
                        ((uint32_t)gBootloaderData->signature[i + 2] << 16) |
                        ((uint32_t)gBootloaderData->signature[i + 3] << 24);
        gSignatureArray[i / 4 + 2] = word;

    }

    /* Mark application as valid and return success */
    gSharedState.appValidFlag = 0x00000000;

    return STATUS_SUCCESS;
}
