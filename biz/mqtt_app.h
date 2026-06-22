#ifndef MQTT_APP_H
#define MQTT_APP_H

#include <stdint.h>

#include "sensor_manager.h"

typedef struct {
    /* MQTT 模型模拟最近一次发布的结果。 */
    const char *topic;
    char last_topic[64];
    char last_payload[128];
    uint32_t publish_count;
} mqtt_publish_model_t;

sensor_listener_t mqtt_create_listener(mqtt_publish_model_t *model, const char *topic);

#endif
