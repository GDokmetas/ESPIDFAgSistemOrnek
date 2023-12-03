#ifndef LED_H_
#define LED_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "driver/gpio.h"



#define BUTTON_1_GPIO GPIO_NUM_34
#define BUTTON_2_GPIO GPIO_NUM_35

#define LED_RED GPIO_NUM_19
#define LED_GREEN GPIO_NUM_21
#define LED_BLUE GPIO_NUM_22
#define LED_YELLOW GPIO_NUM_22


#define LED_INIT() do { \
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT); \
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT); \
    gpio_set_direction(LED_BLUE, GPIO_MODE_OUTPUT); \
    gpio_set_direction(LED_YELLOW, GPIO_MODE_OUTPUT); \
    gpio_set_direction(BUTTON_1_GPIO, GPIO_MODE_INPUT); \
    gpio_set_direction(BUTTON_2_GPIO, GPIO_MODE_INPUT); \
} while(0)

#define LED_ON(gpio) gpio_set_level(gpio, 1)
#define LED_OFF(gpio) gpio_set_level(gpio, 0)
#define LED_TOGGLE(gpio) gpio_set_level(gpio, !gpio_get_level(gpio))

#define LED_ALL_OFF() do { \
    gpio_set_level(LED_RED, 0); \
    gpio_set_level(LED_GREEN, 0); \
    gpio_set_level(LED_BLUE, 0); \
    gpio_set_level(LED_YELLOW, 0); \
} while(0)

#define LED_ALL_ON() do { \
    gpio_set_level(LED_RED, 1); \
    gpio_set_level(LED_GREEN, 1); \
    gpio_set_level(LED_BLUE, 1); \
    gpio_set_level(LED_YELLOW, 1); \
} while(0)

#define BUTTON1_READ() gpio_get_level(BUTTON_1_GPIO)
#define BUTTON2_READ() gpio_get_level(BUTTON_2_GPIO)

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LED_H_ */