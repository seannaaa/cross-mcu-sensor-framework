#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H

#include <stdint.h>

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
    /* sample 统一描述上层真正关心的结果，而不是底层寄存器值。 */
    sensor_type_t type;
    sensor_unit_t unit;
    sensor_status_t status;
    uint32_t timestamp_ms;
    float value;
} sensor_sample_t;

#endif
