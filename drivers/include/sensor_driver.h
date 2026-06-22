#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <stdint.h>

#include "platform_ops.h"
#include "sensor_manager.h"

typedef struct sensor_driver sensor_driver_t;

typedef struct {
    int (*init)(void *context);
    int (*read_sample)(void *context, sensor_sample_t *sample);
} sensor_driver_ops_t;

struct sensor_driver {
    void *context;
    const sensor_driver_ops_t *ops;
};

typedef struct {
    uint8_t device_address;
    uint8_t temperature_register;
    platform_i2c_ops_t *i2c;
    uint8_t *raw_buffer;
} temperature_driver_t;

extern const sensor_driver_ops_t temperature_sensor_driver_ops;

#endif
