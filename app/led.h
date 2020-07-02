#ifndef __X_LED_H
#define __X_LED_H

#define LED_OFF(led)        do { led_off((led)); } while (0)

#define LED_ON(led)         do { led_on((led)); } while (0)

#define LED_SLOW(led)       do { led_pulse((led), 20, 980); } while (0)

#define LED_FAST(led)       do { led_pulse((led), 20, 480); } while (0)

#define LED_HEARTBEAT(led)  do { led_pattern((led), 2, 50, 350, 600); } while (0)

// Function prototypes
void led_init(void);
void led_on(uint8_t led);
void led_off(uint8_t led);
void led_pulse(uint8_t led, uint16_t on_ms, uint16_t off_ms);
void led_pattern(uint8_t led, uint8_t reps, uint16_t on_ms, uint16_t off_ms, uint16_t delay_ms);
const char *led_name(uint8_t led);

#endif  // __X_LED_H
