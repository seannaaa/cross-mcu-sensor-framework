#ifndef BUSINESS_APP_H
#define BUSINESS_APP_H

#include "sensor_manager.h"

typedef struct {
    /* 业务模型用阈值判断是否触发告警。 */
    float alert_threshold;
    int alert_active;
    float last_value_above_threshold;
} business_rule_model_t;

sensor_listener_t business_create_listener(business_rule_model_t *model);

#endif
