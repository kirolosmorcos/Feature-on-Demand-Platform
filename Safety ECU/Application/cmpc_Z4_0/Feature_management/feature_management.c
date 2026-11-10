/*
 * feature_management.c
 *
 *  Created on: 19 Apr 2026
 *      Author: johns / KiroM
 */


#include <time.h>  // for simulation only (replace later with RTC)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "feature_management.h"
#include "flash_c55_driver.h"
#include "feature_management_flash.h"
#include "debug.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"
#include "mbedtls/base64.h"
#include "mbedtls/platform.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

#include "mbedtls/error.h"







#define RSA_SIGNATURE_SIZE 256

#define Feature_manager_FLASH_ADDR   (0x01280000UL)


Feature_manager gFeatureManager;
volatile bool gFeatureFlashBusy = false;


static mbedtls_pk_context g_public_key_ctx;
static bool g_public_key_ready = false;

static const unsigned char public_key_pem[] =
"-----BEGIN PUBLIC KEY-----\n"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAylkH6h+3DP1jUrar17VP\n"
"0lKCaUjYjJJIEpdB13No9f4i8jf12dJ3+QL9UjXOTQJZDtDhzXGs93ar9AFVY7rS\n"
"RbVHO6Dg5x4HpDm7jteDmGbBL+ogOemvfHvNEWfh9oZHm7LSSMZeXEnlmFh7t1yS\n"
"fR+AxR7fapsclHnAddjHxXMNOJAT29FzWY2lWmkc2s/DCGARAOO3yDg+q0KW7tgo\n"
"1mCuWziOokZMDdszXPwBxAX5J3q3dYshcbIFgKSMmlCIh9c9GN3BLJbkPW171uQP\n"
"pS4Oqt25DPYmeRIWC6XY9pd8oD4Ra33cHbSWmh/ubehjfUy7TTee0p4yHahdUPBK\n"
"bwIDAQAB\n"
"-----END PUBLIC KEY-----\n";

bool Signature_PublicKey_Init(void)
{
    int ret;

    mbedtls_pk_init(&g_public_key_ctx);

    ret = mbedtls_pk_parse_public_key(
        &g_public_key_ctx,
        (const unsigned char *)public_key_pem,
        strlen(public_key_pem) + 1
    );

    if(ret != 0)
    {
        TRACE_INFO("Boot public key parse failed ret = -0x%04X\r\n", -ret);
        g_public_key_ready = false;
        return false;
    }

    TRACE_INFO("Boot public key parse OK\r\n");
    g_public_key_ready = true;
    return true;
}

//Hardcoded Data for Diagnostics
static const DiagnosticData gDiagnosticTable[] =
{
    {
        "mc180a",
        "mercedes c180",
        80,
        25
    }
};




// ==========================
// INIT
// ==========================
void init_features(Feature_manager *fm) // init 2 functions for testing for now
{
    fm->feature_count = 3;

    fm->features[0] = (Feature){
        .id = 1,
        .task_id=1,
        .valid_until = 0,
        .is_enabled = false,
        .state = FEATURE_UNAVAILABLE
    };
    strcpy(fm->features[0].name, "Feature_1");

    fm->features[1] = (Feature){
        .id = 2,
        .task_id=2,
        .valid_until = 0,
        .is_enabled = false,
        .state = FEATURE_UNAVAILABLE
    };
    strcpy(fm->features[1].name, "Feature_2");

//    fm->features[2] = (Feature){
//        .id = 3,
//        .task_id=3,
//        .valid_until = 0,
//        .is_enabled = false,
//        .state = FEATURE_UNAVAILABLE
//    };
//    strcpy(fm->features[2].name, "Feature_3");
}


#define RSA_SIGNATURE_SIZE 256

void Test_PublicKey_And_Signature(void)
{
	LicenseToken token = {
	    .license_id  = 0x00000001,
	    .ecu_id      = 0x0E800000,
	    .feature_id  = 0x0002,
	    .valid_from  = 0x00000000,
	    .valid_until = 0xFFFFFFFF,
	    .issuer_id   = 0x01,

	    .signature_base64 =
	    "MpJ2uTFHsgGNKfs8lMvb/t7bA4+Lg6Zys9bghfOo86+8yr/esBAS8uNp75F0UnLeGemHPEtmXXDbqg7fRnpS7SQUHuJ2Jfa+dm7+W8TW8bzgxJPh1ddlGcLemhsef8FqAaClrb2BA+ZXuw8mwAMvh0b65l9h80nfs/FV9B5Ms3kabHoNhsZdoG4uhgVGj4GpgW8CyGx2HBRhmFZLwmXvFNwIFbBWF99BTLMEZui0Fq3UEOII5GMTRxByFA3+YbUe5Qgflt6R4e7rej7aWd/W1rJw98rwbXS0R6tZIBaqrvQ2ZbpN1lideVIq0ewoNL9KTId8b8Cuaf+xF0ASRZJfYw=="
	};

    printf("\r\n==============================\r\n");
    printf("Signature verification test\r\n");
    printf("==============================\r\n");

    printf("license_id  = 0x%08X\r\n", token.license_id);
    printf("ecu_id      = 0x%08X\r\n", token.ecu_id);
    printf("feature_id  = 0x%04X\r\n", token.feature_id);
    printf("valid_from  = %u\r\n", token.valid_from);
    printf("valid_until = %u\r\n", token.valid_until);
    printf("issuer_id   = %u\r\n", token.issuer_id);

    if(signature_verification(&token))
    {
        printf("TEST RESULT: SIGNATURE VALID\r\n");
    }
    else
    {
        printf("TEST RESULT: SIGNATURE INVALID\r\n");
    }

    printf("==============================\r\n\r\n");
}


// ==========================
// ENABLE / DISABLE
// ==========================
bool enable_feature(Feature_manager *fm, uint16_t task_id)
{
    Feature *f = get_feature_by_task_id(fm, task_id);// modify here

    bool old_enabled;
    FeatureState old_state;

    TRACE_INFO("enable_feature entered id=%d\r\n", task_id);

    if (!f)
    {
        TRACE_INFO("enable_feature failed: feature not found\r\n");
        return false;
    }

    TRACE_INFO("enable_feature BEFORE: enabled=%d state=%d\r\n",
               f->is_enabled, f->state);

    if ((f->is_enabled == true) && (f->state == FEATURE_ENABLED))
    {
        TRACE_INFO("enable_feature: already enabled, no flash write\r\n");
        return true;
    }

    old_enabled = f->is_enabled;
    old_state = f->state;

    f->state = FEATURE_ENABLED;
    f->is_enabled = true;

    TRACE_INFO("enable_feature: saving to flash now\r\n");

    if (Feature_manager_save_to_flash(fm) != WRITE_SUCCESS)
    {
        f->is_enabled = old_enabled;
        f->state = old_state;

        TRACE_INFO("enable_feature: flash save failed, RAM rolled back\r\n");
        return false;
    }

    TRACE_INFO("enable_feature: flash save success\r\n");
    return true;
}

bool enable_By_request(Feature_manager *fm, uint16_t task_id, const LicenseToken *token)
{
    Feature *f;

    TRACE_INFO("enable_By_request entered id=%d\r\n", task_id);

    f = get_feature_by_task_id(fm, task_id);


    if (!f)
    {
        TRACE_INFO("enable_By_request failed: feature not found\r\n");
        return false;
    }

    TRACE_INFO("enable_By_request: feature found\r\n");

//    TRACE_INFO("testing the standalone signature\r\n");
//    Test_PublicKey_And_Signature();

    if (!signature_verification(token))
    {
        TRACE_INFO("enable_By_request failed: signature failed\r\n");
        return false;
    }

    TRACE_INFO("enable_By_request: signature OK\r\n");

    return enable_feature(fm, task_id);
}

bool disable_feature(Feature_manager *fm, uint16_t task_id)
{
	 Feature *f = get_feature_by_task_id(fm, task_id);
    bool old_enabled;
    FeatureState old_state;

    TRACE_INFO("disable_feature entered id=%d\r\n", task_id);

    if (!f)
    {
        TRACE_INFO("disable_feature failed: feature not found\r\n");
        return false;
    }

    TRACE_INFO("disable_feature BEFORE: enabled=%d state=%d\r\n",
               f->is_enabled, f->state);

    if ((f->is_enabled == false) && (f->state == FEATURE_EXPIRED))
    {
        TRACE_INFO("disable_feature: already disabled, no flash write\r\n");
        return true;
    }

    old_enabled = f->is_enabled;
    old_state = f->state;

    f->state = FEATURE_EXPIRED;
    f->is_enabled = false;

    TRACE_INFO("disable_feature: saving to flash now\r\n");

    if (Feature_manager_save_to_flash(fm) != WRITE_SUCCESS)
    {
        f->is_enabled = old_enabled;
        f->state = old_state;

        TRACE_INFO("disable_feature: flash save failed, RAM rolled back\r\n");
        return false;
    }

    TRACE_INFO("disable_feature: flash save success\r\n");
    return true;
}
bool disable_feature_By_request(Feature_manager *fm, uint16_t task_id, const LicenseToken *token)
{
    Feature *f;

    TRACE_INFO("disable_By_request entered id=%d\r\n", task_id);

    f = get_feature_by_task_id(fm, task_id);


    if (!f)
    {
        TRACE_INFO("disable_By_request failed: feature not found\r\n");
        return false;
    }

    TRACE_INFO("disable_By_request: feature found enabled=%d state=%d\r\n",
               f->is_enabled,
               f->state);

    if (!signature_verification(token))
    {
        TRACE_INFO("disable_By_request failed: signature failed\r\n");
        return false;
    }

    TRACE_INFO("disable_By_request: signature OK\r\n");

    return disable_feature(fm, task_id);
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

Feature* get_feature_by_task_id(Feature_manager *fm, uint16_t task_id)
{
    for (int i = 0; i < fm->feature_count; i++)
    {
        if (fm->features[i].task_id == task_id)
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



bool signature_verification(const LicenseToken *token)
{
    int ret;
    uint8_t data[19];
    uint8_t hash[32];
    uint8_t signature[RSA_SIGNATURE_SIZE];
    size_t sig_len = 0;

    size_t sig_b64_len = 0; // added 10/5

    sig_b64_len = strlen(token->signature_base64); // added 10/5

    TRACE_INFO("signature strlen = %u\r\n", (unsigned int)sig_b64_len); // added 10/5

    // 1. Decode Base64
    ret = mbedtls_base64_decode(
        signature,
        sizeof(signature),
        &sig_len,
        (const unsigned char *)token->signature_base64,
        sig_b64_len // added 10/5
    );
    TRACE_INFO("Decoded signature length = %u\r\n",
               (unsigned int)sig_len); // added 10/5

    TRACE_INFO("Signature first bytes: %02X %02X %02X %02X\r\n",
               signature[0], signature[1], signature[2], signature[3]); // added 10/5

    TRACE_INFO("Signature last bytes: %02X %02X %02X %02X\r\n",
               signature[252], signature[253], signature[254], signature[255]); // added 10/5



    if (ret != 0 || sig_len != RSA_SIGNATURE_SIZE)
    {
        printf("Base64 decode failed\n");
        return false;
    }

    // 2. Serialize payload
    serialize_payload(token, data);

        uint32_t i;

        TRACE_INFO("SIGNED DATA: ");

        for(i = 0; i < sizeof(data); i++)
        {
            TRACE_INFO("%02X ", data[i]);
        }

        TRACE_INFO("\r\n");


    // 3. Hash
    mbedtls_sha256(data, sizeof(data), hash, 0);

    // 4. Load public key
    if(g_public_key_ready == false)
    {
        TRACE_INFO("Public key not ready\r\n");
        return false;
    }

    if (ret != 0)
    {
        printf("Public key parse failed\n");
        return false;
    }

    // 5. Verify signature
    ret = mbedtls_pk_verify(
        &g_public_key_ctx,
        MBEDTLS_MD_SHA256,
        hash,
        sizeof(hash),
        signature,
        sig_len
    );

    if(ret != 0)
    {
        TRACE_INFO("INVALID SIGNATURE ret = -0x%04X\r\n", -ret); // added 10/5
        return false;
    }


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






void update_feature_state(Feature *f)

{

    if (!check_time_validty(f,f->id))
    {
        f->state = FEATURE_EXPIRED;
    }
    else
    {
        f->state = FEATURE_ENABLED;
    }



}


void update_all_feature_states(Feature_manager *fm)
{
    if (fm == NULL)
        return;

    for (int i = 0; i < fm->feature_count; i++)
    {
        update_feature_state(&fm->features[i]);
    }
}

bool check_time_validty(Feature_manager *fm, uint16_t id)
{
    Feature *f = get_feature_by_id(fm, id);
    uint32_t now;

    if (f == NULL)
        return;

    //now = platform_get_time(); // testing now should be replaced by the real time

    if (now >= f->valid_until){
        disable_feature(fm,id);
        return false;
    }
    else
        return true;

}



//void Feature_manager_init(Feature_manager *fm)
//{
//    if (fm != NULL)
//    {
//        memset(fm, 0, sizeof(Feature_manager));
//    }
//}


status_r Feature_manager_load_from_flash(Feature_manager *fm)
{
    Feature_manager *pFlashData;
    status_r readStatus = READ_ERROR;

    if (fm == NULL)
    {
        return READ_ERROR;
    }

    pFlashData = FeatureManagementFlash_Load(&readStatus);

    if ((readStatus == READ_SUCCESS) && (pFlashData != NULL))
    {
        memcpy(fm, pFlashData, sizeof(Feature_manager));
        return READ_SUCCESS;
    }

    init_features(fm);
    return READ_ERROR;
}



status_r Feature_manager_save_to_flash(const Feature_manager *fm)
{
    status_r status;

    if (fm == NULL)
    {
        return WRITE_ERROR;
    }

    gFeatureFlashBusy = true;

    status = FeatureManagementFlash_Save(fm);

    gFeatureFlashBusy = false;

    return status;
}


void list_all_features(Feature_manager *fm)
{
    if (fm == NULL)
        return;

    // for testing purpose
    update_all_feature_states(fm);

    for (int i = 0; i < fm->feature_count; i++)
    {
        Feature *f = &fm->features[i];

        printf("ID: %d | Name: %s | State: %d\n",
               f->id, f->name, f->state);
    }
}

void reset_all_features(Feature_manager *fm)
{
    fm->feature_count = 0;

    for (int i = 0; i < MAX_FEATURES; i++)
    {
        memset(&fm->features[i], 0, sizeof(Feature));
    }
}




bool Diagnostic_Read(const char *car_id, DiagnosticData *data)
{
    static uint8_t scenario = 0;

    if (strcmp(car_id, "mc180a") != 0)
    {
        return false;
    }

    strcpy(data->car_id, "mc180a");
    strcpy(data->car_type, "mercedes c180");

    switch (scenario)
    {
        case 0: /* Safe */
            data->temperature = 75;
            data->pressure    = 30;
            break;

        case 1: /* High temp only */
            data->temperature = 90;
            data->pressure    = 30;
            break;

        case 2: /* Low pressure only */
            data->temperature = 75;
            data->pressure    = 20;
            break;

        default: /* Danger */
            data->temperature = 90;
            data->pressure    = 20;
            break;
    }

    scenario = (scenario + 1) % 4;

    return true;
}
