#include <stdio.h>
#include <string.h>
#include "feature_management.h"

/* ============================= */
/* Mock Flash File               */
/* ============================= */

#define FLASH_FILE_NAME "flash.bin"

/* ============================= */
/* Save to Flash (Write)         */
/* ============================= */

void mock_flash_save(Feature_manager *fm)
{
    FILE *f = fopen(FLASH_FILE_NAME, "wb");

    if (f == NULL)
        return;

    fwrite(fm, sizeof(Feature_manager), 1, f);

    fclose(f);
}

/* ============================= */
/* Load from Flash (Read)        */
/* ============================= */

void mock_flash_load(Feature_manager *fm)
{
    FILE *f = fopen(FLASH_FILE_NAME, "rb");

    if (f == NULL)
    {
        /* First boot (no flash yet) */
        memset(fm, 0, sizeof(Feature_manager));
        return;
    }

    fread(fm, sizeof(Feature_manager), 1, f);

    fclose(f);
}

/* ============================= */
/* Erase Flash                   */
/* ============================= */

void mock_flash_erase(void)
{
    FILE *f = fopen(FLASH_FILE_NAME, "wb");

    if (f != NULL)
    {
        fclose(f);
    }
}