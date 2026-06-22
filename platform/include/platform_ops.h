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

typedef struct {
    void *context;
    platform_i2c_read_fn read;
    platform_i2c_write_fn write;
} platform_i2c_ops_t;

#endif
