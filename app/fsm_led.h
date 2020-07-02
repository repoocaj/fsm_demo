#ifndef __X_FSM_LED_H
#define __X_FSM_LED_H

#include <stdint.h>

#include "DataTypes.h"
#include "StateMachine.h"

#include "FreeRTOS.h"
#include "timers.h"

// Initialize event data structure
typedef struct
{
    uint8_t led;                /**< The BSP number of the GPIO used to control the LED. */
    TimerHandle_t timer;        /**< The timer used to implement delays. */
} LedInitData;

// Pulse data structure
typedef struct
{
    uint8_t reps;               /**< The number of times the on/off cycle should be repeated. */
    uint16_t on_ms;             /**< The time that the LED will be on in milliseconds. */
    uint16_t off_ms;            /**< The time that the LED will be off in milliseconds. */
    uint16_t delay_ms;          /**< The delay between reps of on/off cycles in milliseconds . */
} LedPulseData;

// LED object structure
typedef struct
{
    LedInitData init;           /**< Data set by an initialization event. */
    LedPulseData pulse;         /**< Data set by a pulse event. */
    uint8_t reps;               /**< Number of reps remaining in the current cycle. */
} Led;

// State machine event functions
EVENT_DECLARE(LED_Init, LedInitData)
EVENT_DECLARE(LED_Pulse, LedPulseData)
EVENT_DECLARE(LED_On, NoEventData)
EVENT_DECLARE(LED_Off, NoEventData)
EVENT_DECLARE(LED_Change, NoEventData)

#endif // __X_FSM_LED_H
