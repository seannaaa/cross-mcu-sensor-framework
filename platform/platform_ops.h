#ifndef PLATFORM_OPS_H
#define PLATFORM_OPS_H

#include <stddef.h>
#include <stdint.h>

typedef int (*platform_i2c_read_fn)(void *context,
                                    uint8_t device_address,
                                    uint8_t register_address,
                                    uint8_t *buffer,
                                    size_t buffer_size);

typedef int (*platform_i2c_write_fn)(void *context,
                                     uint8_t device_address,
                                     uint8_t register_address,
                                     const uint8_t *buffer,
                                     size_t buffer_size);

typedef int (*platform_gpio_write_fn)(void *context, int pin, int value);
typedef int (*platform_pwm_set_fn)(void *context, int channel, uint32_t frequency_hz, float duty_cycle);
typedef uint32_t (*platform_timer_get_tick_fn)(void *context);
typedef void (*platform_delay_ms_fn)(void *context, uint32_t delay_ms);

typedef struct {
    void *context;
    platform_i2c_read_fn read;
    platform_i2c_write_fn write;
} platform_i2c_ops_t;

typedef struct {
    void *context;
    platform_gpio_write_fn write;
} platform_gpio_ops_t;

typedef struct {
    void *context;
    platform_pwm_set_fn set;
} platform_pwm_ops_t;

typedef struct {
    void *context;
    platform_timer_get_tick_fn get_tick_ms;
} platform_timer_ops_t;

typedef struct {
    void *context;
    platform_delay_ms_fn delay_ms;
} platform_delay_ops_t;

typedef struct {
    platform_i2c_ops_t i2c;
    platform_gpio_ops_t gpio;
    platform_pwm_ops_t pwm;
    platform_timer_ops_t timer;
    platform_delay_ops_t delay;
} platform_bundle_t;

#endif
