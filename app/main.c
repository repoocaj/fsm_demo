/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_timer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "fds.h"
#include "nrf_drv_clock.h"

#define NRF_LOG_MODULE_NAME             fsm_demo
#define NRF_LOG_LEVEL                   4
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "boards.h"
#include "version.h"
#include "led.h"
#include "error_msg.h"

/**@brief   Value used as error code on stack dump, can be used to identify
 *          stack location on stack unwind.
 */
#define DEAD_BEEF                       0xDEADBEEF

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief A function which is hooked to idle task.
 * @note Idle hook must be enabled in FreeRTOS configuration (configUSE_IDLE_HOOK).
 */
void vApplicationIdleHook( void )
{
#if NRF_LOG_ENABLED
    while(NRF_LOG_PROCESS());
#endif
}


/**@brief A function which is hooked to stack overflow.
 * @note  Checking for overflow must be enabled in FreeRTOS configuration (configCHECK_FOR_STACK_OVERFLOW).
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   signed char *pcTaskName) {
    NRF_LOG_ERROR("vApplicationStackOverflowHook(%s)", pcTaskName);
    NRF_LOG_FLUSH();
    // If you hit this breakpoint, a FreeRTOS task has
    // overflowed its stack. Examine pcTaskName and xTask to determine
    // which task and either increase its stack when creating it or reduce
    // its stack usage.
    __BKPT();
}


/**@brief A function which is hooked to memory allocation failure.
 * @note  Malloc failed hook must be enabled in FreeRTOS configuration (configUSE_MALLOC_FAILED_HOOK).
 */
void vApplicationMallocFailedHook(void) {
    NRF_LOG_ERROR("vApplicationMallocFailedHook()");
    NRF_LOG_FLUSH();
    // If you hit this breakpoint, a FreeRTOS task has
    // failed to allocate memory in a malloc.
    __BKPT();
}


/**@brief Function for initializing the clock.
 */
static void clock_init(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for displaying information about the SoftDevice installed.
 */
static void chip_info(void)
{
    uint8_t *p = (uint8_t *)&NRF_FICR->INFO.VARIANT;
    uint8_t id[17] = {0};

    NRF_LOG_DEBUG("nRF%x Variant: %c%c%c%c ", NRF_FICR->INFO.PART, p[3], p[2], p[1], p[0]);
    NRF_LOG_DEBUG("RAM: %dKB Flash: %dKB", NRF_FICR->INFO.RAM, NRF_FICR->INFO.FLASH);

    sprintf(id, "%x%x", NRF_FICR->DEVICEID[0], NRF_FICR->DEVICEID[1]);

    // id is on the stack, so copy it to internal logging buffer
    NRF_LOG_DEBUG("Device ID: %s", NRF_LOG_PUSH(id));
}


static void task_info(void)
{
    char buffer[1024] = {0};

    vTaskList(buffer);

    NRF_LOG_RAW_INFO("FreeRTOS Tasks:\n%-12s\tStatus\tPri\tHigh\tNum\n%-12s\t-----\t---\t----\t---\n", "Name", "----");
    NRF_LOG_RAW_INFO("%s\n", buffer);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize modules.
    log_init();

    // Start execution.
    NRF_LOG_RAW_INFO("\nFSM Demo v%d.%d.%d (%s)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_SUB,
#ifdef DEBUG
        "debug"
#else
        "release"
#endif
    );
    chip_info();

    clock_init();

    // Do not start any interrupt that uses system functions before system initialisation.
    // The best solution is to start the OS before any other initalisation.

    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    // Initialize modules.
    timers_init();

    // Configure board LED pins as outputs
    bsp_board_init(BSP_INIT_LEDS);

    // Create FSM's
    led_init();

    task_info();

    // Set the initial states of the LEDs
    LED_ON(BSP_BOARD_LED_0);
    LED_SLOW(BSP_BOARD_LED_1);
    LED_FAST(BSP_BOARD_LED_2);
    LED_HEARTBEAT(BSP_BOARD_LED_3);

    // Start FreeRTOS scheduler.
    vTaskStartScheduler();

    for (;;)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
