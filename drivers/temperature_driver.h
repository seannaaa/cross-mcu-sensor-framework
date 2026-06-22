#ifndef TEMPERATURE_DRIVER_H
#define TEMPERATURE_DRIVER_H

#include <stdint.h>

#include "platform_ops.h"
#include "sensor_types.h"

typedef struct sensor_driver sensor_driver_t;

typedef struct {
    /* init 用来完成 sensor 自身初始化或连通性检查。 */
    int (*init)(void *context);
    /* read_sample 负责把底层 raw data 转成统一 sample。 */
    int (*read_sample)(void *context, sensor_sample_t *sample);
} sensor_driver_ops_t;

struct sensor_driver {
    void *context;
    const sensor_driver_ops_t *ops;
};

typedef struct {
    /* 这里用最小字段演示一个温度芯片驱动需要依赖哪些平台资源。 */
    uint8_t device_address;
    uint8_t temperature_register;
    platform_i2c_ops_t *i2c;
    platform_gpio_ops_t *gpio;
    platform_pwm_ops_t *pwm;
    platform_timer_ops_t *timer;
    platform_delay_ops_t *delay;
    int power_pin;
    int pwm_channel;
    uint8_t *raw_buffer;
} temperature_driver_t;

extern const sensor_driver_ops_t temperature_driver_ops;

#endif
