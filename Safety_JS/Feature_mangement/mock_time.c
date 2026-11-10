#include <stdint.h>

/* ============================= */
/* Mock Time  (RTC Simulation) */
/* ============================= */

/* Start with fixed timestamp */
static uint32_t mock_time = 1700000000;

/* Return current time */
uint32_t platform_get_time(void)
{
    return mock_time;
}

/* Advance time manually (for testing) */
void platform_advance_time(uint32_t seconds)
{
    mock_time += seconds;
}

/* Set time manually */
void platform_set_time(uint32_t new_time)
{
    mock_time = new_time;
}