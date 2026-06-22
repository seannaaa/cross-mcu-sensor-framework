#include "sample_biz.h"

#include <stdio.h>

static int moving_average_init(void *context)
{
    moving_average_filter_t *filter = (moving_average_filter_t *)context;

    if (filter == NULL) {
        return -1;
    }

    filter->history[0] = 0.0f;
    filter->history[1] = 0.0f;
    filter->count = 0U;
    return 0;
}

static int moving_average_process(void *context, sensor_sample_t *sample)
{
    moving_average_filter_t *filter = (moving_average_filter_t *)context;

    if ((filter == NULL) || (sample == NULL)) {
        return -1;
    }

    if (filter->count == 0U) {
        filter->history[0] = sample->value;
        filter->count = 1U;
        return 0;
    }

    /* 用 2 点平均做最小实现，方便先把主链路跑通。 */
    filter->history[1] = sample->value;
    sample->value = (filter->history[0] + filter->history[1]) / 2.0f;
    filter->history[0] = filter->history[1];
    filter->count = 2U;

    return 0;
}

const sensor_filter_ops_t moving_average_filter_ops = {
    .init = moving_average_init,
    .process = moving_average_process,
};

static void gui_on_sample(void *context, const sensor_sample_t *sample)
{
    gui_display_model_t *model = (gui_display_model_t *)context;

    if ((model == NULL) || (sample == NULL)) {
        return;
    }

    model->last_temperature = sample->value;
    model->last_status = sample->status;
    model->last_timestamp_ms = sample->timestamp_ms;
}

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

static void business_on_sample(void *context, const sensor_sample_t *sample)
{
    business_rule_model_t *model = (business_rule_model_t *)context;

    if ((model == NULL) || (sample == NULL)) {
        return;
    }

    if (sample->value >= model->alert_threshold) {
        model->alert_active = 1;
        model->last_value_above_threshold = sample->value;
    } else {
        model->alert_active = 0;
    }
}

sensor_listener_t gui_create_listener(gui_display_model_t *model)
{
    sensor_listener_t listener;
    listener.context = model;
    listener.on_sample = gui_on_sample;
    return listener;
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

sensor_listener_t business_create_listener(business_rule_model_t *model)
{
    sensor_listener_t listener;
    listener.context = model;
    listener.on_sample = business_on_sample;
    return listener;
}
