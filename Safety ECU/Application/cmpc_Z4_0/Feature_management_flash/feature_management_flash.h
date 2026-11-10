#ifndef FEATURE_MANAGEMENT_FLASH_H
#define FEATURE_MANAGEMENT_FLASH_H

#include <stdint.h>
#include <stdbool.h>

#include "status.h"
#include "feature_management.h"
#include "BootloaderFlash.h"

#define FEATURE_MGMT_FLASH_ADDRESS              (0x01280000UL)
#define FEATURE_MGMT_FLASH_BLOCK_SIZE           (0x00040000UL)   /* 256 KB */

#define FEATURE_MGMT_FLASH_START_PATTERN        (0xA5A5A5A5UL)
#define FEATURE_MGMT_FLASH_END_PATTERN          (0x5A5A5A5AUL)
#define FEATURE_MGMT_FLASH_LAYOUT_VERSION       (0x00000001UL)

//typedef struct
//{
//    uint32_t start_pattern;
//    uint32_t version;
//    uint32_t length;
//    Feature_manager fm;
//    uint32_t crc;
//    uint32_t end_pattern;
//} FeatureManagerFlashImage;

status_r FeatureManagementFlash_Save(const Feature_manager *fm);
Feature_manager *FeatureManagementFlash_Load(status_r *readStatus);

#endif
