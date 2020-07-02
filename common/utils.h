/**
 * @file    utils.h
 *
 * Defines some globally useful functions
 */
#ifndef __X_UTILS_H
#define __X_UTILS_H

#include "FreeRTOS.h"
#include "timers.h"

#ifdef __cplusplus
extern "C" {
#endif


/**@brief   Check if a module has been initialized before allowing it to run
 *
 *@param[in]    ret         The value to return if the module isn't initialized.
 */
#if defined(DEBUG)
#define MODULE_INITIALIZED(ret)                                             \
    do                                                                      \
    {                                                                       \
        if (!m_initialized)                                                 \
        {                                                                   \
            NRF_LOG_ERROR("%s: module not initialized", __func__);          \
            return ret;                                                     \
        }                                                                   \
    } while (0)
#else
#define MODULE_INITIALIZED(ret)
#endif


/**@brief   Log the name of a function if VERBOSE has been defined in that module.
 */
#if defined(DEBUG) && defined(VERBOSE)
#define VERBOSE_ID()                                                        \
    do                                                                      \
    {                                                                       \
        NRF_LOG_DEBUG("%s", __func__);                                      \
    } while (0)
#else
#define VERBOSE_ID()
#endif


/**@brief   Start a FreeRTOS timer.
 *
 * The NRF_...HEXDUMP... functions don't always print all of the data.
 *
 *@param[in]    timer       Handle of the timer.
 *@param[in]    name        Name of the timer.
 *@param[in]    descript    Description of the timer.
 *@param[in]    delay_ms    The number of milliseconds that the timer will wait.
 */
void utils_start_timer(TimerHandle_t timer, const char *name, char *descript, uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif  // __X_UTILS_H

