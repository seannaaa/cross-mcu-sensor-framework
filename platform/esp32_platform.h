#ifndef ESP32_PLATFORM_H
#define ESP32_PLATFORM_H

#include <stdint.h>

#include "platform_ops.h"

typedef struct {
    int i2c_port;
    int power_pin;
    int pwm_channel;
    int last_gpio_pin;
    int last_gpio_value;
    int last_pwm_channel;
    uint32_t last_pwm_frequency_hz;
    float last_pwm_duty_cycle;
    uint32_t tick_ms_cache;
    uint32_t last_delay_ms;
    int16_t mock_raw_temperature;
} esp32_platform_t;

platform_bundle_t esp32_platform_create_bundle(esp32_platform_t *platform);

#endif
