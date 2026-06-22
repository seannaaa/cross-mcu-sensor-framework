#include "mqtt_app.h"

#include <stdio.h>

static void mqtt_on_sample(void *context, const sensor_sample_t *sample)
{
    mqtt_publish_model_t *model = (mqtt_publish_model_t *)context;

    if ((model == NULL) || (sample == NULL) || (model->topic == NULL)) {
        return;
    }

    snprintf(model->last_topic, sizeof(model->last_topic), "%s", model->topic);
    snprintf(model->last_payload,
             sizeof(model->last_payload),
             "{\"value\":%.2f,\"unit\":\"C\"}",
             sample->value);
    model->publish_count += 1U;
}

sensor_listener_t mqtt_create_listener(mqtt_publish_model_t *model, const char *topic)
{
    sensor_listener_t listener;

    if (model != NULL) {
        model->topic = topic;
    }

    listener.context = model;
    listener.on_sample = mqtt_on_sample;
    return listener;
}
