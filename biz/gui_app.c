#include "gui_app.h"

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

sensor_listener_t gui_create_listener(gui_display_model_t *model)
{
    sensor_listener_t listener;
    listener.context = model;
    listener.on_sample = gui_on_sample;
    return listener;
}
