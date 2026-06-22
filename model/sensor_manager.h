#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#include "sensor_types.h"

typedef void (*sensor_on_sample_fn)(void *context, const sensor_sample_t *sample);

typedef struct {
    /* listener 统一抽象 GUI / MQTT / business 三类消费者。 */
    void *context;
    sensor_on_sample_fn on_sample;
} sensor_listener_t;

struct sensor_driver;
struct sensor_filter;

typedef struct {
    /* manager 是当前框架主干：driver -> filter -> listeners */
    struct sensor_driver *driver;
    struct sensor_filter *filter;
    sensor_listener_t *listeners;
    size_t listener_count;
    uint32_t timestamp_ms;
} sensor_manager_t;

int sensor_manager_init(sensor_manager_t *manager);
int sensor_manager_read_once(sensor_manager_t *manager);

#endif
