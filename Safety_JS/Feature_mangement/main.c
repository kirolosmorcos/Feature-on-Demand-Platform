//to write test case
#include <stdio.h>
#include <string.h>
#include "feature_management.h"

#define MY_ECU_ID 1001

// Mock time (not used yet, but required)
uint32_t platform_get_time(void)
{
    return 1715000000;
}

int main()
{
    Feature_manager fm;
    init_features(&fm);

    printf("=== Test: enable_By_request ===\n");

   LicenseToken token = {
    .license_id = 0x9C5C5DB6,
    .ecu_id = 0x18FE37AE,
    .feature_id = 0x4939,
    .valid_from = 1777168321,
    .valid_until = 1777168441,
    .issuer_id = 2,

    .signature_base64 = "TeKh9fIbBcUm4qgXmFbmH/gRmYcKo5arVVMWqdx06o5ict0WOA2a5Iv/6SbQpJ6oZmSm7qZMJic5N2/gIPsSZfE5PYi2aRJXtLQCV59ZnTrIWLeiKdvsHnQTdkxGO0hEAgHAD1cv5tDSo4YLDTrd8vIKtMzXsDVnkYo68NUvSf1s2xOs7JwOFKk5tAxO2yv215BJ+cneRipPHRIagyuHjZ9oCcX1AYV3Lj5+4ve5YO3Pip7eBVpYx4PA3yhUWtUyjCbHUctecgtRbzoVJkd2X1rOIP4LX83JJMCYMaWhz6NHvdql+VdBcEgUqXZe38qo8DzqOMRUh3BN7AFO2O00+g=="
};
    bool result = enable_By_request(&fm, 1, &token);

    if (result)
        printf("Feature ENABLED successfully\n");
    else
        printf("Feature enable FAILED (expected with fake signature)\n");

    // Print feature state
    Feature *f = get_feature_by_id(&fm, 1);

    if (f)
    {
        printf("Feature ID: %d\n", f->id);
        printf("State: %d\n", f->state);
        printf("is_enabled: %d\n", f->is_enabled);
    }

    return 0;
}