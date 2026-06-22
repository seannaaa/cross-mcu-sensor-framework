#ifndef GUI_APP_H
#define GUI_APP_H

#include <stdint.h>

#include "sensor_manager.h"

typedef struct {
    /* GUI 模型只保存界面最终要展示的数据。 */
    float last_temperature;
    sensor_status_t last_status;
    uint32_t last_timestamp_ms;
} gui_display_model_t;

sensor_listener_t gui_create_listener(gui_display_model_t *model);

#endif
