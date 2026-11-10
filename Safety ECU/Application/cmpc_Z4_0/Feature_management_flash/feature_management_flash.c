#include "feature_management_flash.h"
#include <string.h>
#include "debug.h"
static Feature_manager g_FeatureManagerRamCopy;

#define FEATURE_MGMT_IMAGE_RAW_SIZE      ((uint32_t)sizeof(FeatureManagerFlashImage))
#define FEATURE_MGMT_IMAGE_ALIGNED_SIZE  (((FEATURE_MGMT_IMAGE_RAW_SIZE + 3U) / 4U) * 4U)

static uint32_t FeatureManagementFlash_CalculateCRC32(const uint8_t *data,
                                                      uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    uint32_t j;

    if (data == NULL)
    {
        return 0U;
    }

    for (i = 0U; i < length; i++)
    {
        crc ^= (uint32_t)data[i];

        for (j = 0U; j < 8U; j++)
        {
            if ((crc & 1U) != 0U)
            {
                crc = (crc >> 1U) ^ 0xEDB88320UL;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return ~crc;
}

static uint32_t FeatureManagementFlash_CalculateImageCRC(const FeatureManagerFlashImage *image)
{
    return FeatureManagementFlash_CalculateCRC32(
        (const uint8_t *)&image->length,
        sizeof(image->length) + sizeof(image->fm)
    );
}

static bool FeatureManagementFlash_IsImageValid(const FeatureManagerFlashImage *image)
{
    uint32_t calculatedCrc;

    if (image == NULL)
    {
        return false;
    }

    if (image->start_pattern != FEATURE_MGMT_FLASH_START_PATTERN)
    {
        return false;
    }

    if (image->version != FEATURE_MGMT_FLASH_LAYOUT_VERSION)
    {
        return false;
    }

    if (image->length != sizeof(Feature_manager))
    {
        return false;
    }

    if (image->end_pattern != FEATURE_MGMT_FLASH_END_PATTERN)
    {
        return false;
    }

    calculatedCrc = FeatureManagementFlash_CalculateImageCRC(image);

    if (calculatedCrc != image->crc)
    {
        return false;
    }

    return true;
}

status_r FeatureManagementFlash_Save(const Feature_manager *fm)
{
    status_t status;
    FeatureManagerFlashImage image;
    static uint8_t programBuffer[FEATURE_MGMT_IMAGE_ALIGNED_SIZE];

    if (fm == NULL)
    {
        return WRITE_ERROR;
    }

    memset(&image, 0, sizeof(image));
    memset(programBuffer, 0xFF, sizeof(programBuffer));

    image.start_pattern = FEATURE_MGMT_FLASH_START_PATTERN;
    image.version       = FEATURE_MGMT_FLASH_LAYOUT_VERSION;
    image.length        = sizeof(Feature_manager);
    image.fm            = *fm;
    image.crc           = FeatureManagementFlash_CalculateImageCRC(&image);
    image.end_pattern   = FEATURE_MGMT_FLASH_END_PATTERN;

    memcpy(programBuffer, &image, sizeof(image));

    status = BootloaderFlash_Unlock();
    if (status != STATUS_SUCCESS)
    {
        return WRITE_ERROR;
    }
    TRACE_INFO("FM save: erase start\r\n");
    status = BootloaderFlash_Erase(FEATURE_MGMT_FLASH_ADDRESS,
                                   FEATURE_MGMT_FLASH_BLOCK_SIZE);
    if (status != STATUS_SUCCESS)
    {
        return WRITE_ERROR;
    }
    TRACE_INFO("FM save: erase ok\r\n");


    status = BootloaderFlash_VerifyBlank(FEATURE_MGMT_FLASH_ADDRESS,
                                         FEATURE_MGMT_FLASH_BLOCK_SIZE);
    if (status != STATUS_SUCCESS)
    {
        return WRITE_ERROR;
    }

    TRACE_INFO("FM save: program start\r\n");
    status = BootloaderFlash_Program(FEATURE_MGMT_FLASH_ADDRESS,
                                     FEATURE_MGMT_IMAGE_ALIGNED_SIZE,
                                     programBuffer);
    if (status != STATUS_SUCCESS)
    {
        return WRITE_ERROR;
    }

    TRACE_INFO("FM save: program ok\r\n");

    TRACE_INFO("FM save: verify/load start\r\n");
    status = BootloaderFlash_ProgramVerify(FEATURE_MGMT_FLASH_ADDRESS,
                                           FEATURE_MGMT_IMAGE_ALIGNED_SIZE,
                                           programBuffer);

    while (status == STATUS_FLASH_INPROGRESS)
    {
        status = BootloaderFlash_ProgramVerify(FEATURE_MGMT_FLASH_ADDRESS,
                                               FEATURE_MGMT_IMAGE_ALIGNED_SIZE,
                                               programBuffer);
    }


    if (status != STATUS_SUCCESS)
    {
        return WRITE_ERROR;
    }
    TRACE_INFO("FM save: verify ok\r\n");

    return WRITE_SUCCESS;
}

Feature_manager *FeatureManagementFlash_Load(status_r *readStatus)
{
    const FeatureManagerFlashImage *image =
        (const FeatureManagerFlashImage *)FEATURE_MGMT_FLASH_ADDRESS;

    if (readStatus != NULL)
    {
        *readStatus = READ_ERROR;
    }

    memset(&g_FeatureManagerRamCopy, 0, sizeof(g_FeatureManagerRamCopy));

    /*
     * Empty flash check.
     */
    if (*(volatile const uint32_t *)FEATURE_MGMT_FLASH_ADDRESS == 0xFFFFFFFFUL)
    {
        return NULL;
    }

    if (!FeatureManagementFlash_IsImageValid(image))
    {
        return NULL;
    }

    memcpy(&g_FeatureManagerRamCopy,
           &image->fm,
           sizeof(Feature_manager));

    if (readStatus != NULL)
    {
        *readStatus = READ_SUCCESS;
    }

    return &g_FeatureManagerRamCopy;
}
