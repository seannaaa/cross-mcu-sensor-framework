#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <stdint.h>

typedef struct {
    /* 这些字段由应用层填写，表示平台初始化时需要的参数。 */
    int i2c_bus;
    int power_pin;
    int pwm_channel;
    uint32_t tick_ms;
    int16_t mock_raw_temperature;

    /* 这些字段由平台实现写回，方便测试和调试观察调用结果。 */
    int last_gpio_pin;
    int last_gpio_value;
    int last_pwm_channel;
    uint32_t last_pwm_frequency_hz;
    float last_pwm_duty_cycle;
    uint32_t last_delay_ms;
} platform_t;

#endif
