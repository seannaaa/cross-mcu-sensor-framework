#ifndef STM32_PLATFORM_H
#define STM32_PLATFORM_H

#include <stdint.h>

#include "platform_ops.h"

typedef struct {
    void *hi2c;
    void *htim;
    int power_pin;
    int pwm_channel;
    int last_gpio_pin;
    int last_gpio_value;
    int last_pwm_channel;
    uint32_t last_pwm_frequency_hz;
    float last_pwm_duty_cycle;
    uint32_t systick_ms_cache;
    uint32_t last_delay_ms;
    int16_t mock_raw_temperature;
} stm32_platform_t;

platform_bundle_t stm32_platform_create_bundle(stm32_platform_t *platform);

#endif
