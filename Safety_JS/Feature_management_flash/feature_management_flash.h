// /*
//  * Module Name  : Feature Management Flash                                             
//  * File Name    : FeatureManagementFlash.h                                             
//  * Purpose      : Wrapper layer for persisting Feature Manager data in flash.          
// */

// #ifndef FEATUREMANAGEMENTFLASH_H
// #define FEATUREMANAGEMENTFLASH_H

// #include <stdint.h>
// //#include "flash_c55_driver.h"
// #include "feature_management.h"

// // Macros 

// /* Fixed storage location: start of Code Flash Block 4 */
// #define FEATURE_MGMT_FLASH_ADDRESS          (0x01100000UL)

// /* Image validation constants */
// #define FEATURE_MGMT_FLASH_START_PATTERN    (0xA5A5A5A5UL)
// #define FEATURE_MGMT_FLASH_END_PATTERN      (0x5A5A5A5AUL)
// #define FEATURE_MGMT_FLASH_LAYOUT_VERSION   (0x00000000UL)


//  // Return type for load / save                                 

// typedef enum
// {
//     READ_SUCCESS,
//     READ_ERROR,
//     WRITE_SUCCESS,
//     WRITE_ERROR
// } status_r;


//  /* Flash image format*/                                           
 
// typedef struct
// {
//     uint32_t start_pattern;      /* fixed magic value */
//     uint32_t version;            /* layout version */
//     uint32_t length;             /* size of valid payload */
//     Feature_manager fm;          /* persisted feature manager data */
//     uint32_t crc;                /* CRC over length + fm */
//     uint32_t end_pattern;        /* fixed magic value */
// } FeatureManagerFlashImage;



// status_t FeatureManagementFlash_Init(void);

// Feature_manager *FeatureManagementFlash_Load(uint32_t a_FlashAddress,
//                                              status_r *a_pReadStatus);

// status_r FeatureManagementFlash_Save(const Feature_manager *a_pFeatureManager,
//                                      uint32_t a_FlashAddress);

// #endif /* FEATUREMANAGEMENTFLASH_H */