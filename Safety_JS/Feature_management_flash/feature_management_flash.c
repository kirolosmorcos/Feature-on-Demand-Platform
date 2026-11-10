// /***************************************************************************************
//  * Module Name  : Feature Management Flash                                             *
//  * File Name    : FeatureManagementFlash.c                                             *
//  * Purpose      : Wrapper layer for persisting Feature Manager data in flash.          *
//  ***************************************************************************************/

// /****************************************************************************************
//  *                                    -=[Includes]=-                                    *
//  ****************************************************************************************/
// #include "feature_Management_Flash.h"
// #include <string.h>

// /*******************************************************************************
//  *                             Global Variables                                *
//  *******************************************************************************/

// static uint32_t volatile g_FeatureManagementFlashInit = false; //Flag for module initialization. 
// static flash_context_data_t g_FeatureManagementFlashCtxData; //Stores flash programming context.
// static uint32_t g_FeatureManagementFlashFailedAddr = 0U; //Stores failed address during flash verify blank check.  
// static Feature_manager g_FeatureManagerRamCopy; //RAM copy returned by load function

// /*******************************************************************************
//  *                             Local Definitions                               *
//  *******************************************************************************/

// #define FEATURE_MGMT_FLASH_NUM_WORD_CYCLE_BLANK_CHECK     (16U)
// #define FEATURE_MGMT_FLASH_NUM_WORD_CYCLE_PROGRAM_VERIFY  (16U)

// #define FEATURE_MGMT_FLASH_IMAGE_RAW_SIZE \
//     ((uint32_t)sizeof(FeatureManagerFlashImage))

// /* Program size must be aligned to 4 bytes */
// #define FEATURE_MGMT_FLASH_IMAGE_ALIGNED_SIZE \
//     (((FEATURE_MGMT_FLASH_IMAGE_RAW_SIZE + (C55_WORD_SIZE - 1U)) / C55_WORD_SIZE) * C55_WORD_SIZE)

// /*******************************************************************************
//  *                            Local Prototypes                                 *
//  *******************************************************************************/

// static uint32_t FeatureManagementFlash_CalculateCRC32(const uint8_t *a_pData,
//                                                       uint32_t a_Length);

// static status_t FeatureManagementFlash_Erase(uint32_t a_FlashAddress,
//                                              uint32_t a_Size);

// static status_t FeatureManagementFlash_VerifyBlank(uint32_t a_Dest,
//                                                    uint32_t a_Size);

// static status_t FeatureManagementFlash_Program(uint32_t a_Dest,
//                                                uint32_t a_Size,
//                                                uint32_t a_Source);

// static status_t FeatureManagementFlash_ProgramVerify(uint32_t a_Dest,
//                                                      uint32_t a_Size,
//                                                      uint32_t a_Source);

// static uint8_t FeatureManagementFlash_IsImageValid(const FeatureManagerFlashImage *a_pImage);

// /*******************************************************************************
//  *                                Functions                                    *
//  *******************************************************************************/

// /****************************************************************
//  * Function Name: FeatureManagementFlash_Init                  *
//  * Description  : Initializes the flash driver module.         *
//  ****************************************************************/
// status_t FeatureManagementFlash_Init(void)
// {
//     status_t returnCode = STATUS_SUCCESS;

//     if (g_FeatureManagementFlashInit == false)
//     {
//         returnCode = FLASH_DRV_Init();
//         if (returnCode == STATUS_SUCCESS)
//         {
//             g_FeatureManagementFlashInit = true;
//         }
//     }

//     return returnCode;
// }

// /****************************************************************
//  * Function Name: FeatureManagementFlash_CalculateCRC32        *
//  * Description  : Software CRC32 over input buffer.            *
//  ****************************************************************/
// static uint32_t FeatureManagementFlash_CalculateCRC32(const uint8_t *a_pData,
//                                                       uint32_t a_Length)
// {
//     uint32_t crc = 0xFFFFFFFFUL;
//     uint32_t i;
//     uint32_t j;

//     for (i = 0U; i < a_Length; i++)
//     {
//         crc ^= (uint32_t)a_pData[i];

//         for (j = 0U; j < 8U; j++)
//         {
//             if ((crc & 1U) != 0U)
//             {
//                 crc = (crc >> 1U) ^ 0xEDB88320UL;
//             }
//             else
//             {
//                 crc >>= 1U;
//             }
//         }
//     }

//     return ~crc;
// }

// /****************************************************************
//  * Function Name: FeatureManagementFlash_Erase                 *
//  * Description  : Erases reserved Code Flash Block 4.          *
//  *
//  * Reserved persistent block:
//  *   0x01100000 - 0x0113FFFF  => 256 KB Code Flash Block 4
//  ****************************************************************/
// static status_t FeatureManagementFlash_Erase(uint32_t a_FlashAddress,
//                                              uint32_t a_Size)
// {
//     status_t returnCode = STATUS_SUCCESS;
//     flash_state_t opResult;
//     flash_block_select_t blockSelect;

//     (void)a_Size;

//     memset(&blockSelect, 0, sizeof(blockSelect));

//     if ((a_FlashAddress >= 0x01100000UL) && (a_FlashAddress < 0x01140000UL))
//     {
//         blockSelect.lowBlockSelect = 0x0U;
//         blockSelect.midBlockSelect = 0x0U;
//         blockSelect.highBlockSelect = 0x0U;
//         blockSelect.first256KBlockSelect = (1UL << 4); /* Code Flash Block 4 */
//         blockSelect.second256KBlockSelect = 0x0U;
//     }
//     else
//     {
//         return STATUS_ERROR;
//     }

//     returnCode = FLASH_DRV_Erase(ERS_OPT_MAIN_SPACE, &blockSelect);
//     if (returnCode != STATUS_SUCCESS)
//     {
//         return returnCode;
//     }

//     do
//     {
//         returnCode = FLASH_DRV_CheckEraseStatus(&opResult);
//     } while (returnCode == STATUS_FLASH_INPROGRESS);

//     return returnCode;
// }

// /****************************************************************
//  * Function Name: FeatureManagementFlash_VerifyBlank           *
//  * Description  : Blank-check the target flash range.          *
//  ****************************************************************/
// static status_t FeatureManagementFlash_VerifyBlank(uint32_t a_Dest,
//                                                    uint32_t a_Size)
// {
//     return FLASH_DRV_BlankCheck(a_Dest,
//                                 a_Size,
//                                 FEATURE_MGMT_FLASH_NUM_WORD_CYCLE_BLANK_CHECK,
//                                 &g_FeatureManagementFlashFailedAddr,
//                                 NULL_CALLBACK);
// }

// /****************************************************************
//  * Function Name: FeatureManagementFlash_Program               *
//  * Description  : Program image into flash.                    *
//  ****************************************************************/
// static status_t FeatureManagementFlash_Program(uint32_t a_Dest,
//                                                uint32_t a_Size,
//                                                uint32_t a_Source)
// {
//     status_t returnCode = STATUS_SUCCESS;
//     flash_state_t opResult;

//     do
//     {
//         returnCode = FLASH_DRV_CheckProgramStatus(&g_FeatureManagementFlashCtxData, &opResult);
//     } while (returnCode == STATUS_FLASH_INPROGRESS);

//     returnCode = FLASH_DRV_Program(&g_FeatureManagementFlashCtxData,
//                                    a_Dest,
//                                    a_Size,
//                                    a_Source);

//     return returnCode;
// }

// /****************************************************************
//  * Function Name: FeatureManagementFlash_ProgramVerify         *
//  * Description  : Verify programmed image.                     *
//  ****************************************************************/
// static status_t FeatureManagementFlash_ProgramVerify(uint32_t a_Dest,
//                                                      uint32_t a_Size,
//                                                      uint32_t a_Source)
// {
//     status_t returnCode = STATUS_SUCCESS;
//     flash_state_t opResult;

//     do
//     {
//         returnCode = FLASH_DRV_CheckProgramStatus(&g_FeatureManagementFlashCtxData, &opResult);
//     } while (returnCode == STATUS_FLASH_INPROGRESS);

//     returnCode = FLASH_DRV_ProgramVerify(a_Dest,
//                                          a_Size,
//                                          a_Source,
//                                          FEATURE_MGMT_FLASH_NUM_WORD_CYCLE_PROGRAM_VERIFY,
//                                          &g_FeatureManagementFlashFailedAddr,
//                                          NULL_CALLBACK);

//     return returnCode;
// }

// /****************************************************************
//  * Function Name: FeatureManagementFlash_IsImageValid          *
//  * Description  : Validate markers, length, and CRC.           *
//  ****************************************************************/
// static uint8_t FeatureManagementFlash_IsImageValid(const FeatureManagerFlashImage *a_pImage)
// {
//     uint32_t calculatedCRC;
//     uint32_t crcInputLength;

//     if (a_pImage == NULL)
//     {
//         return false;
//     }

//     if (a_pImage->start_pattern != FEATURE_MGMT_FLASH_START_PATTERN)
//     {
//         return false;
//     }

//     if (a_pImage->end_pattern != FEATURE_MGMT_FLASH_END_PATTERN)
//     {
//         return false;
//     }

//     if (a_pImage->length != sizeof(Feature_manager))
//     {
//         return false;
//     }

//     crcInputLength = sizeof(a_pImage->length) + sizeof(a_pImage->fm);

//     calculatedCRC = FeatureManagementFlash_CalculateCRC32((const uint8_t *)&a_pImage->length,
//                                                           crcInputLength);

//     if (calculatedCRC != a_pImage->crc)
//     {
//         return false;
//     }

//     return true;
// }

// /****************************************************************
//  * Function Name: FeatureManagementFlash_Load                  *
//  * Description  : Load and validate feature manager from flash.*
//  ****************************************************************/
// Feature_manager *FeatureManagementFlash_Load(uint32_t a_FlashAddress,
//                                              status_r *a_pReadStatus)
// {
//     const FeatureManagerFlashImage *pFlashImage;

//     if (a_pReadStatus != NULL)
//     {
//         *a_pReadStatus = READ_ERROR;
//     }

//     pFlashImage = (const FeatureManagerFlashImage *)a_FlashAddress;

//     /* Empty flash */
//     if (*(const volatile uint32_t *)a_FlashAddress == 0xFFFFFFFFUL)
//     {
//         memset(&g_FeatureManagerRamCopy, 0, sizeof(g_FeatureManagerRamCopy));
//         return NULL;
//     }

//     if (!FeatureManagementFlash_IsImageValid(pFlashImage))
//     {
//         memset(&g_FeatureManagerRamCopy, 0, sizeof(g_FeatureManagerRamCopy));
//         return NULL;
//     }

//     memcpy(&g_FeatureManagerRamCopy, &pFlashImage->fm, sizeof(Feature_manager));

//     if (a_pReadStatus != NULL)
//     {
//         *a_pReadStatus = READ_SUCCESS;
//     }

//     return &g_FeatureManagerRamCopy;
// }

// /****************************************************************
//  * Function Name: FeatureManagementFlash_Save                  *
//  * Description  : Build image, erase, blank-check, program,    *
//  *                and verify.                                  *
//  ****************************************************************/
// status_r FeatureManagementFlash_Save(const Feature_manager *a_pFeatureManager,
//                                      uint32_t a_FlashAddress)
// {
//     status_t returnCode = STATUS_SUCCESS;
//     FeatureManagerFlashImage image;
//     uint8_t programBuffer[FEATURE_MGMT_FLASH_IMAGE_ALIGNED_SIZE];

//     if (a_pFeatureManager == NULL)
//     {
//         return WRITE_ERROR;
//     }

//     /* C55 program destination must be aligned to 8 bytes */
//     if ((a_FlashAddress % C55_DWORD_SIZE) != 0U)
//     {
//         return WRITE_ERROR;
//     }

//     returnCode = FeatureManagementFlash_Init();
//     if (returnCode != STATUS_SUCCESS)
//     {
//         return WRITE_ERROR;
//     }

//     memset(&image, 0, sizeof(image));
//     memset(programBuffer, 0xFF, sizeof(programBuffer));

//     /* Build complete flash image in RAM first */
//     image.start_pattern = FEATURE_MGMT_FLASH_START_PATTERN;
//     image.version = FEATURE_MGMT_FLASH_LAYOUT_VERSION;
//     image.length = sizeof(Feature_manager);
//     image.fm = *a_pFeatureManager;
//     image.crc = FeatureManagementFlash_CalculateCRC32((const uint8_t *)&image.length,
//                                                       sizeof(image.length) + sizeof(image.fm));
//     image.end_pattern = FEATURE_MGMT_FLASH_END_PATTERN;

//     memcpy(programBuffer, &image, sizeof(image));

//     returnCode = FeatureManagementFlash_Erase(a_FlashAddress,
//                                               FEATURE_MGMT_FLASH_IMAGE_ALIGNED_SIZE);
//     if (returnCode != STATUS_SUCCESS)
//     {
//         return WRITE_ERROR;
//     }

//     returnCode = FeatureManagementFlash_VerifyBlank(a_FlashAddress,
//                                                     FEATURE_MGMT_FLASH_IMAGE_ALIGNED_SIZE);
//     if (returnCode != STATUS_SUCCESS)
//     {
//         return WRITE_ERROR;
//     }

//     returnCode = FeatureManagementFlash_Program(a_FlashAddress,
//                                                 FEATURE_MGMT_FLASH_IMAGE_ALIGNED_SIZE,
//                                                 (uint32_t)programBuffer);
//     if (returnCode != STATUS_SUCCESS)
//     {
//         return WRITE_ERROR;
//     }

//     returnCode = FeatureManagementFlash_ProgramVerify(a_FlashAddress,
//                                                       FEATURE_MGMT_FLASH_IMAGE_ALIGNED_SIZE,
//                                                       (uint32_t)programBuffer);
//     if (returnCode != STATUS_SUCCESS)
//     {
//         return WRITE_ERROR;
//     }

//     return WRITE_SUCCESS;
// }