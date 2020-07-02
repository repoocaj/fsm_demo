/**
 * @file    dump.c
 *
 * Implements some globally useful functions
 */

#include <stdint.h>

#define NRF_LOG_MODULE_NAME     utils
#define NRF_LOG_LEVEL           4
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "utils.h"

#define VERBOSE                 0

void utils_start_timer(TimerHandle_t timer, const char *name, char *descript, uint32_t delay_ms)
{
    if (pdPASS != xTimerChangePeriod(timer, pdMS_TO_TICKS(delay_ms), 0))
    {
        NRF_LOG_ERROR("Failed to set %s %s", name, descript);
    }
#if VERBOSE
    else
    {
        NRF_LOG_DEBUG("Set %s %s to %d ms", name, descript, delay_ms);
    }
#endif

    if (pdPASS != xTimerStart(timer, 0))
    {
        NRF_LOG_ERROR("Failed to start %s %s", name, descript);
    }
#if VERBOSE
    else
    {
        NRF_LOG_DEBUG("Started %s %s", name, descript);
    }
#endif
}
