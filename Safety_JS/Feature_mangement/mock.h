#ifndef MOCK_H
#define MOCK_H

#include <stdint.h>
#include "feature_management.h"

/* Time */
uint32_t platform_get_time(void);
void platform_advance_time(uint32_t seconds);
void platform_set_time(uint32_t new_time);

/* Flash */
void mock_flash_save(Feature_manager *fm);
void mock_flash_load(Feature_manager *fm);
void mock_flash_erase(void);

#endif