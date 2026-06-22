#ifndef SENSOR_FILTER_H
#define SENSOR_FILTER_H

#include "sensor_manager.h"

typedef struct sensor_filter sensor_filter_t;

typedef struct {
    int (*init)(void *context);
    int (*process)(void *context, sensor_sample_t *sample);
} sensor_filter_ops_t;

struct sensor_filter {
    void *context;
    const sensor_filter_ops_t *ops;
};

typedef struct {
    float history[2];
    unsigned int count;
} moving_average_filter_t;

extern const sensor_filter_ops_t moving_average_filter_ops;

#endif
