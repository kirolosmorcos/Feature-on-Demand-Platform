#ifndef FEATURE_MANAGEMENT_H_
#define FEATURE_MANAGEMENT_H_

#include <stdint.h>
#include <stdbool.h>


/* ============================= */
/* Configuration                 */
/* ============================= */

#define MAX_FEATURES 3
#define RSA_SIGNATURE_SIZE 256   // RSA-2048

/* ============================= */
/* Data Types                    */
/* ============================= */

typedef enum{
    READ_SUCCESS,
    READ_ERROR,
    WRITE_SUCCESS,
    WRITE_ERROR,
}status_r;


typedef enum {
    FEATURE_UNAVAILABLE,
    FEATURE_ENABLED,
    FEATURE_EXPIRED,
    FEATURE_PENDING_DEACTIVATION
} FeatureState;

typedef struct {
    uint16_t id;
    char name[32];

    uint32_t valid_until;

    bool is_enabled;
    FeatureState state;
} Feature;

typedef struct {
    
    Feature features[MAX_FEATURES];
    uint8_t feature_count;
} Feature_manager;

typedef struct {
    uint32_t license_id;
    uint32_t ecu_id;
    uint16_t feature_id;
    uint32_t valid_from;
    uint32_t valid_until;
    uint8_t issuer_id;

    char signature_base64[400];
} LicenseToken;

typedef struct
{
    uint32_t start_pattern;      /* fixed magic value */
    uint32_t version;            /* layout version */
    uint32_t length;             /* size of valid payload */
    Feature_manager fm;          /* persisted feature manager data */
    uint32_t crc;                /* CRC over length + fm */
    uint32_t end_pattern;        /* fixed magic value */
} FeatureManagerFlashImage;


/* ============================= */
/* Platform Abstraction Layer    */
/* ============================= */

uint32_t platform_get_time(void);

/* ============================= */
/* Core Functions                */
/* ============================= */

void init_features(Feature_manager *fm);

bool enable_By_request(Feature_manager *fm, uint16_t id, const LicenseToken *token);

bool disable_feature(Feature_manager *fm, uint16_t id);

bool disable_feature_By_request(Feature_manager *fm, uint16_t id, const LicenseToken *token);

bool check_time_validty(Feature_manager *fm, uint16_t id);//john done


void Feature_manager_init(Feature_manager *fm);
status_r Feature_manager_load_from_flash(Feature_manager *fm);
status_r Feature_manager_save_to_flash(const Feature_manager *fm);
/* ============================= */
/* Supporting Functions          */
/* ============================= */

Feature* get_feature_by_id(Feature_manager *fm, uint16_t id);

void update_feature_state(Feature *f); //john done

void update_all_feature_states(Feature_manager *fm);//john

bool check_feature_available(Feature *f);//john


bool signature_verification(const LicenseToken *token);

/* ============================= */
/* Optional (you can implement later) */
/* ============================= */

// bool check_time_validty(Feature_manager *fm, uint16_t id);
// uint32_t get_remaining_time(Feature *f);
// void update_feature_state(Feature *f);
// void update_all_feature_states(Feature_manager *fm);
// bool check_feature_available(Feature *f);
// Feature_manager* load_features_from_flash();
// int save_features_to_flash();

#endif /* FEATURE_MANAGEMENT_H_ */
