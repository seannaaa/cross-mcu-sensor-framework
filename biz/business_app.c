#include "business_app.h"

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

sensor_listener_t business_create_listener(business_rule_model_t *model)
{
    sensor_listener_t listener;
    listener.context = model;
    listener.on_sample = business_on_sample;
    return listener;
}
