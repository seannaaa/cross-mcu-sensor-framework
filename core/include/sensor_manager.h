#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#include "dispatch_listener.h"

typedef enum {
    SENSOR_TYPE_TEMPERATURE = 0,
} sensor_type_t;

typedef enum {
    SENSOR_UNIT_CELSIUS = 0,
} sensor_unit_t;

typedef enum {
    SENSOR_STATUS_OK = 0,
    SENSOR_STATUS_ERROR = 1,
} sensor_status_t;

typedef struct sensor_sample {
    sensor_type_t type;
    sensor_unit_t unit;
    sensor_status_t status;
    uint32_t timestamp_ms;
    float value;
} sensor_sample_t;

struct sensor_driver;
struct sensor_filter;

typedef struct {
    struct sensor_driver *driver;
    struct sensor_filter *filter;
    sensor_event_listener_t *listeners;
    size_t listener_count;
    uint32_t timestamp_ms;
} sensor_manager_t;

int sensor_manager_init(sensor_manager_t *manager);
int sensor_manager_read_once(sensor_manager_t *manager);

#endif
