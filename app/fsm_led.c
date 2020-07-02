#define NRF_LOG_MODULE_NAME     fsm_led
#define NRF_LOG_LEVEL           4
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "boards.h"

#include "led.h"
#include "fsm_led.h"
#include "StateMachine.h"
#include "utils.h"

#define VERBOSE                 0

// State enumeration order must match the order of state
// method entries in the state map
enum States
{
    ST_INIT,            // 0 - Initial state, nothing can be done here
    ST_INITIALIZE,      // 1 - Initialize FSM
    ST_SOLID_OFF,       // 2 - LED is off
    ST_SOLID_ON,        // 3 - LED is on
    ST_PULSE_START,     // 4 - Start pulsing the LED
    ST_PULSE_OFF,       // 5 - LED is off but will turn on after a delay
    ST_PULSE_ON,        // 6 - LED if on but will turn off after a delay
    ST_REP_START,       // 7 - Setup the pulse pattern for a run
    ST_REP_DELAY,       // 8 - Wait before restarting a pulse pattern
    ST_REP_DEC,         // 9 - Decrement the rep counter

    ST_MAX_STATES
};

#define SM_NAME                Led

// State machine state functions
STATE_DECLARE(Init, NoEventData)
STATE_DECLARE(Initialize, LedInitData)
STATE_DECLARE(SolidOff, NoEventData)
STATE_DECLARE(SolidOn, NoEventData)
STATE_DECLARE(PulseStart, LedPulseData)
STATE_DECLARE(PulseOff, NoEventData)
STATE_DECLARE(PulseOn, NoEventData)
STATE_DECLARE(RepStart, NoEventData)
STATE_DECLARE(RepDelay, NoEventData)
STATE_DECLARE(RepDec, NoEventData)

// State map to define state function order
BEGIN_STATE_MAP(SM_NAME)
    STATE_MAP_ENTRY(Init)
    STATE_MAP_ENTRY(Initialize)
    STATE_MAP_ENTRY(SolidOff)
    STATE_MAP_ENTRY(SolidOn)
    STATE_MAP_ENTRY(PulseStart)
    STATE_MAP_ENTRY(PulseOff)
    STATE_MAP_ENTRY(PulseOn)
    STATE_MAP_ENTRY(RepStart)
    STATE_MAP_ENTRY(RepDelay)
    STATE_MAP_ENTRY(RepDec)
END_STATE_MAP(SM_NAME)

// LED initialize event.  Sent when the FSM is being initialized.
EVENT_DEFINE(LED_Init, LedInitData)
{
    VERBOSE_ID();

    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(ST_INITIALIZE)     // ST_INIT
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INITIALIZE
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_SOLID_OFF
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_SOLID_ON
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_PULSE_START
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_PULSE_OFF
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_PULSE_ON
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_START
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_REP_DELAY
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_DEC
    END_TRANSITION_MAP(SM_NAME, pEventData)
}

// LED pulse event.  Sent when the LED should start pulsing in an on/off
// pattern.
EVENT_DEFINE(LED_Pulse, LedPulseData)
{
    VERBOSE_ID();

    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INIT
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INITIALIZE
        TRANSITION_MAP_ENTRY(ST_PULSE_START)    // ST_SOLID_OFF
        TRANSITION_MAP_ENTRY(ST_PULSE_START)    // ST_SOLID_ON
        TRANSITION_MAP_ENTRY(ST_PULSE_START)    // ST_PULSE_START
        TRANSITION_MAP_ENTRY(ST_PULSE_START)    // ST_PULSE_OFF
        TRANSITION_MAP_ENTRY(ST_PULSE_START)    // ST_PULSE_ON
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_START
        TRANSITION_MAP_ENTRY(ST_PULSE_START)    // ST_REP_DELAY
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_DEC
    END_TRANSITION_MAP(SM_NAME, pEventData)
}

// LED on event.  Sent when the LED should be on.
EVENT_DEFINE(LED_On, NoEventData)
{
    VERBOSE_ID();

    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INIT
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INITIALIZE
        TRANSITION_MAP_ENTRY(ST_SOLID_ON)       // ST_SOLID_OFF
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_SOLID_ON
        TRANSITION_MAP_ENTRY(ST_SOLID_ON)       // ST_PULSE_START
        TRANSITION_MAP_ENTRY(ST_SOLID_ON)       // ST_PULSE_OFF
        TRANSITION_MAP_ENTRY(ST_SOLID_ON)       // ST_PULSE_ON
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_START
        TRANSITION_MAP_ENTRY(ST_SOLID_ON)       // ST_REP_DELAY
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_DEC
    END_TRANSITION_MAP(SM_NAME, pEventData)
}

// LED off event.  Sent when the LED should be off.
EVENT_DEFINE(LED_Off, NoEventData)
{
    VERBOSE_ID();

    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INIT
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INITIALIZE
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_SOLID_OFF
        TRANSITION_MAP_ENTRY(ST_SOLID_OFF)      // ST_SOLID_ON
        TRANSITION_MAP_ENTRY(ST_SOLID_OFF)      // ST_PULSE_START
        TRANSITION_MAP_ENTRY(ST_SOLID_OFF)      // ST_PULSE_OFF
        TRANSITION_MAP_ENTRY(ST_SOLID_OFF)      // ST_PULSE_ON
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_START
        TRANSITION_MAP_ENTRY(ST_SOLID_OFF)      // ST_REP_DELAY
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_DEC
    END_TRANSITION_MAP(SM_NAME, pEventData)
}

// LED change event.  This event would occur after the ST_PULSE_OFF or
// ST_PULSE_ON states set a timer to generate the event to flip states.  We
// don't send an internal event because the state machine will go into a
// infinite loop (on sending off event and vice versa).
EVENT_DEFINE(LED_Change, NoEventData)
{
    VERBOSE_ID();

    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INIT
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_INITIALIZE
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_SOLID_OFF
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_SOLID_ON
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_PULSE_START
        TRANSITION_MAP_ENTRY(ST_REP_DEC)        // ST_PULSE_OFF
        TRANSITION_MAP_ENTRY(ST_PULSE_OFF)      // ST_PULSE_ON
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_START
        TRANSITION_MAP_ENTRY(ST_REP_START)      // ST_REP_DELAY
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_REP_DEC
    END_TRANSITION_MAP(SM_NAME, pEventData)
}

STATE_DEFINE(Init, NoEventData)
{
    // Initial state of the FSM.  This function is never executed as we can
    // only transistion out of this state.
}

STATE_DEFINE(Initialize, LedInitData)
{
    VERBOSE_ID();

    ASSERT_TRUE(pEventData);

    Led *pData = SM_GetInstance(Led);

    pData->init.led = pEventData->led;
    pData->init.timer = pEventData->timer;

    NRF_LOG_DEBUG("%s initial", led_name(pData->init.led));

    SM_InternalEvent(ST_SOLID_OFF, NULL);
}

STATE_DEFINE(SolidOff, NoEventData)
{
    VERBOSE_ID();

    Led *pData = SM_GetInstance(Led);

    NRF_LOG_DEBUG("%s off", led_name(pData->init.led));

    bsp_board_led_off(pData->init.led);
}

STATE_DEFINE(SolidOn, NoEventData)
{
    VERBOSE_ID();

    Led *pData = SM_GetInstance(Led);

    NRF_LOG_DEBUG("%s on", led_name(pData->init.led));

    bsp_board_led_on(pData->init.led);
}

STATE_DEFINE(PulseStart, LedPulseData)
{
    VERBOSE_ID();

    Led *pData = SM_GetInstance(Led);

    ASSERT_TRUE(pEventData);

    pData->pulse.reps = pEventData->reps;
    pData->pulse.off_ms = pEventData->off_ms;
    pData->pulse.on_ms = pEventData->on_ms;
    pData->pulse.delay_ms = pEventData->delay_ms;

    if (pData->pulse.reps > 1)
    {
        NRF_LOG_DEBUG("%s pattern: %d ms on, %d ms off repeated %d times, delay %d ms",
            led_name(pData->init.led),
            pData->pulse.on_ms,
            pData->pulse.off_ms,
            pData->pulse.reps,
            pData->pulse.delay_ms
        );
    }
    else
    {
        NRF_LOG_DEBUG("%s pulse: %d ms on, %d ms off",
            led_name(pData->init.led),
            pData->pulse.on_ms,
            pData->pulse.off_ms
        );
        // Make sure we have a valid number of reps and no delay
        pData->pulse.reps = 1;
        pData->pulse.delay_ms = 0;
    }

    // Start the pulsing
    SM_InternalEvent(ST_REP_START, NULL);
}

STATE_DEFINE(PulseOff, NoEventData)
{
    VERBOSE_ID();

    Led *pData = SM_GetInstance(Led);

#if VERBOSE
    NRF_LOG_DEBUG("%s pulse on", led_name(pData->init.led));
#endif

    bsp_board_led_off(pData->init.led);

    utils_start_timer(
        pData->init.timer,
        led_name(pData->init.led),
        "off pulse",
        pData->pulse.off_ms
    );
}

STATE_DEFINE(PulseOn, NoEventData)
{
    VERBOSE_ID();

    Led *pData = SM_GetInstance(Led);

#if VERBOSE
    NRF_LOG_DEBUG("%s pulse on", led_name(pData->init.led));
#endif

    bsp_board_led_on(pData->init.led);

    utils_start_timer(
        pData->init.timer,
        led_name(pData->init.led),
        "on pulse",
        pData->pulse.on_ms
    );
}

STATE_DEFINE(RepStart, NoEventData)
{
    VERBOSE_ID();

    Led *pData = SM_GetInstance(Led);

    // Initialize the rep counter
    pData->reps = pData->pulse.reps;

    SM_InternalEvent(ST_PULSE_ON, NULL);
}

STATE_DEFINE(RepDelay, NoEventData)
{
    VERBOSE_ID();

    Led *pData = SM_GetInstance(Led);

    utils_start_timer(
        pData->init.timer,
        led_name(pData->init.led),
        "delay",
        pData->pulse.delay_ms
    );
}

STATE_DEFINE(RepDec, NoEventData)
{
    VERBOSE_ID();

    Led *pData = SM_GetInstance(Led);

    // Decrement the remaining reps in the pattern
    pData->reps -= 1;

    if (pData->reps > 0)
    {
        // Continue running
        SM_InternalEvent(ST_PULSE_ON, NULL);
    }
    else
    {
        // Last rep, check if we're a simple pattern with no delay
        if (0 == pData->pulse.delay_ms)
        {
            // Yes, immediately restart
            SM_InternalEvent(ST_REP_START, NULL);
        }
        else
        {
            // No, delay first
            SM_InternalEvent(ST_REP_DELAY, NULL);
        }
    }
}
