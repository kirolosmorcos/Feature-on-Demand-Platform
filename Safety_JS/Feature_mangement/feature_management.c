/*
 * feature_management.c
 *
 *  Created on: 19 Apr 2026
 *      Author: johns
 */


#include <time.h>  // for simulation only (replace later with RTC)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "feature_management.h"
#include "mock.h"
//#include "flash_c55_driver.h"
//#include "feature_management_flash.h"
#include "mbedtls/sha256.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#define Feature_manager_FLASH_ADDR   (0x01100000UL)




const char* public_key_pem =
"-----BEGIN PUBLIC KEY-----\n"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAylkH6h+3DP1jUrar17VP\n"
"0lKCaUjYjJJIEpdB13No9f4i8jf12dJ3+QL9UjXOTQJZDtDhzXGs93ar9AFVY7rS\n"
"RbVHO6Dg5x4HpDm7jteDmGbBL+ogOemvfHvNEWfh9oZHm7LSSMZeXEnlmFh7t1yS\n"
"fR+AxR7fapsclHnAddjHxXMNOJAT29FzWY2lWmkc2s/DCGARAOO3yDg+q0KW7tgo\n"
"1mCuWziOokZMDdszXPwBxAX5J3q3dYshcbIFgKSMmlCIh9c9GN3BLJbkPW171uQP\n"
"pS4Oqt25DPYmeRIWC6XY9pd8oD4Ra33cHbSWmh/ubehjfUy7TTee0p4yHahdUPBK\n"
"bwIDAQAB\n"
"-----END PUBLIC KEY-----\n";

// ==========================
// INIT
// ==========================
void init_features(Feature_manager *fm)
{
    fm->feature_count = 1;

    fm->features[0] = (Feature){
        .id = 1,
        .valid_until = 0,
        .is_enabled = false,
        .state = FEATURE_UNAVAILABLE
    };

    strcpy(fm->features[0].name, "feature1");
}

// ==========================
// ENABLE / DISABLE
// ==========================
bool enable_feature(Feature_manager *fm, uint16_t id)
{
    Feature *f = get_feature_by_id(fm, id);
    if (!f) return false;

    f->state = FEATURE_ENABLED;
    f->is_enabled = true;

    return true;
}

bool enable_By_request(Feature_manager *fm, uint16_t id, const  LicenseToken *token)
{
    Feature *f = get_feature_by_id(fm, id);
    if (!f) return false;

    if (!signature_verification(token))
        return false;

    return enable_feature(fm, id);
}

bool disable_feature(Feature_manager *fm, uint16_t id)
{
    Feature *f = get_feature_by_id(fm, id);
    if (!f) return false;

    f->state = FEATURE_EXPIRED;
    f->is_enabled = false;

    return true;
}

bool disable_feature_By_request(Feature_manager *fm, uint16_t id, const LicenseToken *token)
{
    Feature *f = get_feature_by_id(fm, id);
    if (!f) return false;

    if (!signature_verification(token))
        return false;

    return disable_feature(fm, id);
}

// ==========================
// HELPER
// ==========================
Feature* get_feature_by_id(Feature_manager *fm, uint16_t id)
{
    for (int i = 0; i < fm->feature_count; i++)
    {
        if (fm->features[i].id == id)
            return &fm->features[i];
    }
    return NULL;
}

// ==========================
// SERIALIZATION
// ==========================
void serialize_payload(const LicenseToken *token, uint8_t *out)
{
    out[0] = token->license_id;
    out[1] = token->license_id >> 8;
    out[2] = token->license_id >> 16;
    out[3] = token->license_id >> 24;

    out[4] = token->ecu_id;
    out[5] = token->ecu_id >> 8;
    out[6] = token->ecu_id >> 16;
    out[7] = token->ecu_id >> 24;

    out[8] = token->feature_id;
    out[9] = token->feature_id >> 8;

    out[10] = token->valid_from;
    out[11] = token->valid_from >> 8;
    out[12] = token->valid_from >> 16;
    out[13] = token->valid_from >> 24;

    out[14] = token->valid_until;
    out[15] = token->valid_until >> 8;
    out[16] = token->valid_until >> 16;
    out[17] = token->valid_until >> 24;

    out[18] = token->issuer_id;
}

// ==========================
// SIGNATURE VERIFY
// ==========================
bool signature_verification(const LicenseToken *token)
{
    int ret;
    uint8_t data[19];
    uint8_t hash[32];
    uint8_t signature[RSA_SIGNATURE_SIZE];
    size_t sig_len = 0;

    // 1. Decode Base64
    ret = mbedtls_base64_decode(
        signature,
        sizeof(signature),
        &sig_len,
        (const unsigned char *)token->signature_base64,
        strlen(token->signature_base64)
    );

    if (ret != 0 || sig_len != RSA_SIGNATURE_SIZE)
    {
        printf("Base64 decode failed\n");
        return false;
    }

    // 2. Serialize payload
    serialize_payload(token, data);

    // 3. Hash
    mbedtls_sha256(data, sizeof(data), hash, 0);

    // 4. Load public key
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    ret = mbedtls_pk_parse_public_key(
        &pk,
        (const unsigned char *)public_key_pem,
        strlen(public_key_pem) + 1
    );

    if (ret != 0)
    {
        printf("Public key parse failed\n");
        return false;
    }

    // 5. Verify signature
    ret = mbedtls_pk_verify(
        &pk,
        MBEDTLS_MD_SHA256,
        hash,
        sizeof(hash),
        signature,
        sig_len
    );

    mbedtls_pk_free(&pk);

    if (ret == 0)
    {
        printf("VALID SIGNATURE\n");
        return true;
    }
    else
    {
        printf("INVALID SIGNATURE\n");
        return false;
    }
}



// void update_feature_state(Feature *f)

// {

//     if (!check_time_validty(f,f->id))
//     {
//         f->state = FEATURE_EXPIRED;
//     }
//     else
//     {
//         f->state = FEATURE_ENABLED;
//     }



// }


// void update_all_feature_states(Feature_manager *fm)
// {
//     if (fm == NULL)
//         return;

//     for (int i = 0; i < fm->feature_count; i++)
//     {
//         update_feature_state(&fm->features[i]);
//     }
// }

// bool check_time_validty(Feature_manager *fm, uint16_t id)
// {
//     Feature *f = get_feature_by_id(fm, id);
//     uint32_t now;

//     if (f == NULL)
//         return;

//     now = platform_get_time(); // testing now should be replaced by the real time

//     if (now >= f->valid_until){
//         disable_feature(fm,id);
//         return false;
//     }
//     else
//         return true;

// }



void Feature_manager_init(Feature_manager *fm)
{
    if (fm != NULL)
    {
        memset(fm, 0, sizeof(Feature_manager));
    }
}


// status_r Feature_manager_load_from_flash(Feature_manager *fm)
// {
//     Feature_manager *loadedFm;
//     status_r readStatus;

//     if (fm == NULL)
//     {
//         return READ_ERROR;
//     }

//     loadedFm = FeatureManagementFlash_Load(FEATURE_MGMT_FLASH_ADDRESS, &readStatus);

//     if ((readStatus == READ_SUCCESS) && (loadedFm != NULL))
//     {
//         *fm = *loadedFm;
//         return READ_SUCCESS;
//     }

//     /* If nothing valid in flash, initialize empty manager */
//     Feature_manager_init(fm);
//     return READ_ERROR;
// }

// status_r Feature_manager_save_to_flash(const Feature_manager *fm)
// {
//     if (fm == NULL)
//     {
//         return WRITE_ERROR;
//     }

//     return FeatureManagementFlash_Save(fm, FEATURE_MGMT_FLASH_ADDRESS);
// }


// void list_all_features(Feature_manager *fm)
// {
//     if (fm == NULL)
//         return;

//     // for testing purpose
//     update_all_feature_states(fm);

//     for (int i = 0; i < fm->feature_count; i++)
//     {
//         Feature *f = &fm->features[i];

//         printf("ID: %d | Name: %s | State: %d\n",
//                f->id, f->name, f->state);
//     }
// }

// void reset_all_features(Feature_manager *fm)
// {
//     fm->feature_count = 0;

//     for (int i = 0; i < MAX_FEATURES; i++)
//     {
//         memset(&fm->features[i], 0, sizeof(Feature));
//     }
// }


