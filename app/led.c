#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#define NRF_LOG_MODULE_NAME     led
#define NRF_LOG_LEVEL           4
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "nrf_log_ctrl.h"

#include "led.h"
#include "error_msg.h"
#include "fsm_led.h"
#include "boards.h"
#include "utils.h"

#define VERBOSE 0

/**@brief   Check if the led parameter is valid
 *
 *@param[in]    led         The LED value to test.
 *@param[in]    ret         The value to return if LED value isn't valid.
 */
#if defined(DEBUG)
#define VALID_LED(led, ret)                                                 \
    do                                                                      \
    {                                                                       \
        if (((led) < BSP_BOARD_LED_0) || ((led) > BSP_BOARD_LED_3))         \
        {                                                                   \
            NRF_LOG_ERROR("Invalid LED %d, ignoring", (led));               \
            return ret;                                                     \
        }                                                                   \
    } while (0)
#else
#define VALID_LED(led, ret)
#endif

/**@brief   Enumeration of possible LED states.
 */
typedef enum
{
    LED_STATE_Min = 0,

    LED_STATE_INIT = LED_STATE_Min, /**< Initialize the LED FSM */
    LED_STATE_ON,                   /**< LED is on solid */
    LED_STATE_OFF,                  /**< LED if off solid */
    LED_STATE_PULSE,                /**< LED is pulsing a on/off pattern */
    LED_STATE_CHANGE,               /**< LED internal state indicating a change in a pulse pattern */

    LED_STATE_Max,
} led_state_t;

/**@brief   Structure queued to a LED thread to send events to the thread.
 */
typedef struct
{
    led_state_t state;          /**< State that the LED should be in. */
    union
    {
        LedInitData init;       /**< Initialization data */
        LedPulseData pulse;     /**< Pulse data */
    };
} led_event_t;

/**@brief   Structure used to hold handles for various objects needed by the LED threads.
 */
typedef struct
{
    QueueHandle_t   queue;      /**< Queue used to receive events. */
    TimerHandle_t   timer;      /**< Timer used to pulse the LED. */
} led_thread_data_t;

#define QUEUE_EVENTS            4
#define QUEUE_ITEM_SIZE         sizeof(led_event_t)

/**@brief   Module global variable used to inidicate that the module has been
 *          initialized.
 */
static bool m_initialized = false;

/**@brief   Task handles for the LED threads.
 */
static TaskHandle_t m_threads[LEDS_NUMBER];

/**@brief   Array of structures that contain the handles for thread data objects.
 */
static led_thread_data_t m_data[LEDS_NUMBER];

/**@brief   A mapping of the name of the LED to the BSP LED index.
 */
static struct
{
    char * name;
    uint8_t led;
} m_name_map[LEDS_NUMBER] =
{
    { "LED1",   BSP_BOARD_LED_0 },
    { "LED2",   BSP_BOARD_LED_1 },
    { "LED3",   BSP_BOARD_LED_2 },
    { "LED4",   BSP_BOARD_LED_3 },
};

/**@brief Thread for sending events to the FSM running the LED.
 *
 * @param[in]   arg     Pointer used for passing some arbitrary information
 *                      (context) from the osThreadCreate() call to the thread.
 */
static void led_fsm_thread(void * arg)
{
    led_thread_data_t *self = (led_thread_data_t *) arg;
#if VERBOSE
    NRF_LOG_DEBUG("arg: %p", arg);
    NRF_LOG_DEBUG("queue: %p", self->queue);
    NRF_LOG_DEBUG("timer: %p", self->timer);
#endif
    // Define a LED object
    Led led;

    // Define a FSM
    SM_DEFINE(LED, &led);

    while (1)
    {
        led_event_t event;

        if(pdPASS != xQueueReceive(self->queue, &event, portMAX_DELAY))
        {
            NRF_LOG_WARNING("Queue unexpectedly empty!");
        }
        else
        {

            switch (event.state)
            {
            case LED_STATE_INIT:
            {
                LedInitData *data;

                data = SM_XAlloc(sizeof(LedInitData));
                if (NULL != data)
                {
                    // Send the LED and the timer on on initialization, those
                    // values are constant and won't change over the life of
                    // the FSM
                    data->led = event.init.led;
                    data->timer = event.init.timer;
                    SM_Event(LED, LED_Init, data);
                }
                else
                {
                    NRF_LOG_ERROR("Can't allocate memory for %d", event.state);
                }
            }
                break;

            case LED_STATE_ON:
                SM_Event(LED, LED_On, NULL);
                break;

            case LED_STATE_OFF:
                SM_Event(LED, LED_Off, NULL);
                break;

            case LED_STATE_PULSE:
            {
                LedPulseData *data;

                data = SM_XAlloc(sizeof(LedPulseData));
                if (NULL != data)
                {
                    data->reps = event.pulse.reps;
                    data->on_ms = event.pulse.on_ms;
                    data->off_ms = event.pulse.off_ms;
                    data->delay_ms = event.pulse.delay_ms;
                    SM_Event(LED, LED_Pulse, data);
                }
                else
                {
                    NRF_LOG_ERROR("Can't allocate memory for %d", event.state);
                }
            }
                break;

            case LED_STATE_CHANGE:
                SM_Event(LED, LED_Change, NULL);
                break;

            default:
                NRF_LOG_ERROR("Unexpected LED state: %d", event.state);
                break;
            }
        }
    }
}

void _led_timer_callback(TimerHandle_t xTimer)
{
    // Get the LED that needs the event
    uint32_t led = (uint32_t) pvTimerGetTimerID(xTimer);

    led_event_t event;

    // Create and queue a change event
    event.state = LED_STATE_CHANGE;

    if (pdPASS != xQueueSendToBackFromISR(m_data[led].queue, &event, NULL))
    {
        NRF_LOG_ERROR("Failed to add %d event to queue for %s",
            event.state,
            m_name_map[led].name
        );
    }
}

#if LEDS_NUMBER != 4
#error LEDS_NUMBER is expected to be 4, led_init() will need to be updated.
#endif

void led_init(void)
{
    for (int i = 0; i < LEDS_NUMBER; i++)
    {
        // We use a 32 bit value for the LED so that we can cast it to a void
        // pointer when creating the timer
        uint32_t led = m_name_map[i].led;
        char *name = m_name_map[i].name;

        // Create queue for event data
        m_data[led].queue = xQueueCreate(QUEUE_EVENTS, QUEUE_ITEM_SIZE);
        if (NULL == m_data[led].queue)
        {
            NRF_LOG_ERROR("%s event queue could not be created", name);
            NRF_LOG_FLUSH();
            return;
        }
#if VERBOSE
        else
        {
            NRF_LOG_DEBUG("%s event queue handle: %p", name, m_data[led].queue);
        }
#endif
        // Create timer to pulse LED when required
        m_data[led].timer = xTimerCreate(
            name,               // Timer name is the LED name
            1,                  // Initial timer period, unused
            pdFALSE,            // Timer doesn't autoreload
            (void * )led,       // Unique ID of the timer
            _led_timer_callback // Each timer calls the same callback
        );
        if (NULL == m_data[led].timer)
        {
            NRF_LOG_ERROR("%s timer could not be created", name);
            NRF_LOG_FLUSH();
            return;
        }
#if VERBOSE
        else
        {
            NRF_LOG_DEBUG("%s timer handle: %p", name, m_data[led].timer);
        }
#endif
        // Create a thread for the LED
        if (pdPASS != xTaskCreate(
            led_fsm_thread,         // task code
            name,                   // name
            512,                    // stack size in words
            (void *)&m_data[led],   // pvParameters
            1,                      // uxPriority -- one step above idle
            &m_threads[led])        // *pxCreatedTask
        )
        {
            NRF_LOG_ERROR("%s thread could not be created", name);
            NRF_LOG_FLUSH();
            return;
        }
#if VERBOSE
        else
        {
            NRF_LOG_DEBUG("%s thread handle: %p", name, m_threads[led]);
        }
#endif
        // Initialize the LED FSM
        led_event_t event;
        event.state = LED_STATE_INIT;
        event.init.led = led;
        event.init.timer = m_data[led].timer;

        if (pdPASS != xQueueSendToBack(m_data[led].queue, &event, 0))
        {
            NRF_LOG_ERROR("Failed to add %d event to %s queue",
                event.state,
                name
            );
        }
    }
#if VERBOSE
    NRF_LOG_DEBUG("LED threads created");
    NRF_LOG_FLUSH();
#endif
    // We're initialized to the point that we can call our methods
    m_initialized = true;
}

void led_on(uint8_t led)
{
    MODULE_INITIALIZED();

    VALID_LED(led, );

    led_event_t event;

    event.state = LED_STATE_ON;

    if (pdPASS != xQueueSendToBackFromISR(m_data[led].queue, &event, NULL))
    {
        NRF_LOG_ERROR("Failed to add %d event to queue for %s",
            event.state,
            m_name_map[led].name
        );
    }
}

void led_off(uint8_t led)
{
    MODULE_INITIALIZED();

    VALID_LED(led, );

    led_event_t event;

    event.state = LED_STATE_OFF;

    if (pdPASS != xQueueSendToBackFromISR(m_data[led].queue, &event, NULL))
    {
        NRF_LOG_ERROR("Failed to add %d event to queue for %s",
            event.state,
            m_name_map[led].name
        );
    }
}

void led_pulse(uint8_t led, uint16_t on_ms, uint16_t off_ms)
{
    led_pattern(led, 1, on_ms, off_ms, 0);
}

void led_pattern(uint8_t led, uint8_t reps, uint16_t on_ms, uint16_t off_ms, uint16_t delay_ms)
{
    MODULE_INITIALIZED();

    VALID_LED(led, );

    led_event_t event;

    event.state = LED_STATE_PULSE;
    event.pulse.reps = reps;
    event.pulse.on_ms = on_ms;
    event.pulse.off_ms = off_ms;
    event.pulse.delay_ms = delay_ms;

    if (pdPASS != xQueueSendToBackFromISR(m_data[led].queue, &event, NULL))
    {
        NRF_LOG_ERROR("Failed to add %d event to queue for %s",
            event.state,
            m_name_map[led].name
        );
    }
}

const char * led_name(uint8_t led)
{
    MODULE_INITIALIZED(NULL);

    VALID_LED(led, NULL);

    return m_name_map[led].name;
}

