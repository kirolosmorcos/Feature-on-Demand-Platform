//
// Created by johns on 16/04/2026.
//
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "feature_management.h"
#include "mock.h"

static void print_menu(void)
{
    printf("\n========== Feature Manager Test ==========\n");
    printf("1. Add feature\n");
    printf("2. Enable feature\n");
    printf("3. Disable feature\n");
    printf("4. Check feature validity\n");
    printf("5. Show remaining time\n");
    printf("6. List all features\n");
    printf("7. Advance time (seconds)\n");
    printf("8. Save to mock flash\n");
    printf("9. Load from mock flash\n");
    printf("10. Reset all features\n");
    printf("0. Exit\n");
    printf("Choose: ");
}

static void add_feature_ui(feature_manager *fm)
{
    Feature f;
    memset(&f, 0, sizeof(Feature));

    printf("Enter feature ID: ");
    scanf("%hu", &f.id);

    printf("Enter feature name: ");
    scanf("%31s", f.name);

    printf("Enter feature version: ");
    scanf("%15s", f.version);

    printf("Enter issuer: ");
    scanf("%31s", f.issuer);

    printf("Enter valid_from (epoch seconds): ");
    scanf("%u", &f.valid_from);

    printf("Has valid_until? (1=yes, 0=no): ");
    scanf("%d", (int *)&f.has_valid_until);

    if (f.has_valid_until)
    {
        printf("Enter valid_until (epoch seconds): ");
        scanf("%u", &f.valid_until);
    }
    else
    {
        f.valid_until = 0U;
    }

    /* temporary until auth is finalized */
    f.auth_hash = 0xA5A5A5A5U;
    f.state = FEATURE_DISABLED;

    if (add_feature(fm, f))
    {
        printf("Feature added successfully.\n");
    }
    else
    {
        printf("Failed to add feature.\n");
    }
}

static void enable_feature_ui(feature_manager *fm)
{
    uint16_t id;

    printf("Enter feature ID to enable: ");
    scanf("%hu", &id);

    if (enable_feature(fm, id))
    {
        printf("Feature enabled successfully.\n");
    }
    else
    {
        printf("Failed to enable feature.\n");
    }
}

static void disable_feature_ui(feature_manager *fm)
{
    uint16_t id;

    printf("Enter feature ID to disable: ");
    scanf("%hu", &id);

    if (disable_feature(fm, id))
    {
        printf("Feature disabled successfully.\n");
    }
    else
    {
        printf("Failed to disable feature.\n");
    }
}

static void check_valid_ui(feature_manager *fm)
{
    uint16_t id;

    printf("Enter feature ID to check: ");
    scanf("%hu", &id);

    if (check_valid(fm, id))
    {
        printf("Feature is currently valid and enabled.\n");
    }
    else
    {
        printf("Feature is not valid / expired / disabled.\n");
    }
}

static void remaining_time_ui(feature_manager *fm)
{
    uint16_t id;
    Feature *f;
    uint32_t remaining;

    printf("Enter feature ID: ");
    scanf("%hu", &id);

    f = get_feature_by_id(fm, id);
    if (f == NULL)
    {
        printf("Feature not found.\n");
        return;
    }

    remaining = get_remaining_time(f);

    if (f->has_valid_until == false)
    {
        printf("Feature has no expiration time.\n");
        return;
    }

    printf("Remaining time: %u seconds\n", remaining);
    printf("Remaining time: %u days, %u hours, %u minutes, %u seconds\n",
           remaining / 86400U,
           (remaining % 86400U) / 3600U,
           (remaining % 3600U) / 60U,
           remaining % 60U);
}

static void advance_time_ui(feature_manager *fm)
{
    uint32_t seconds;

    printf("Enter seconds to advance: ");
    scanf("%u", &seconds);

    platform_advance_time(seconds);
    update_all_feature_states(fm);

    printf("Time advanced by %u seconds.\n", seconds);
    printf("Current mock time: %u\n", platform_get_time());
}

int main(void)
{
    feature_manager fm;
    int choice;

    memset(&fm, 0, sizeof(fm));

    printf("Starting Feature Manager test application...\n");
    printf("Initial mock time: %u\n", platform_get_time());

    do
    {
        print_menu();
        scanf("%d", &choice);

        switch (choice)
        {
            case 1:
                add_feature_ui(&fm);
                break;

            case 2:
                enable_feature_ui(&fm);
                break;

            case 3:
                disable_feature_ui(&fm);
                break;

            case 4:
                check_valid_ui(&fm);
                break;

            case 5:
                remaining_time_ui(&fm);
                break;

            case 6:
                list_all_features(&fm);
                break;

            case 7:
                advance_time_ui(&fm);
                break;

            case 8:
                mock_flash_save(&fm);
                printf("Saved to mock flash.\n");
                break;

            case 9:
                mock_flash_load(&fm);
                printf("Loaded from mock flash.\n");
                break;

            case 10:
                reset_all_features(&fm);
                printf("All features reset.\n");
                break;

            case 0:
                printf("Exiting application.\n");
                break;

            default:
                printf("Invalid choice.\n");
                break;
        }

    } while (choice != 0);

    return 0;
}