#ifndef SAMPLE_BIZ_H
#define SAMPLE_BIZ_H

#include <stdint.h>

#include "sensor_manager.h"

typedef struct sensor_filter sensor_filter_t;

typedef struct {
    /* init 负责清理 filter 内部状态。 */
    int (*init)(void *context);
    /* process 只处理 sample，不碰平台和业务。 */
    int (*process)(void *context, sensor_sample_t *sample);
} sensor_filter_ops_t;

struct sensor_filter {
    void *context;
    const sensor_filter_ops_t *ops;
};

typedef struct {
    /* 这里先用 2 点历史值演示 moving average 的最小实现。 */
    float history[2];
    unsigned int count;
} moving_average_filter_t;

typedef struct {
    /* GUI 模型只保存界面最终要展示的数据。 */
    float last_temperature;
    sensor_status_t last_status;
    uint32_t last_timestamp_ms;
} gui_display_model_t;

typedef struct {
    /* MQTT 模型模拟最近一次发布的结果。 */
    const char *topic;
    char last_topic[64];
    char last_payload[128];
    uint32_t publish_count;
} mqtt_publish_model_t;

typedef struct {
    /* 业务模型用阈值判断是否触发告警。 */
    float alert_threshold;
    int alert_active;
    float last_value_above_threshold;
} business_rule_model_t;

extern const sensor_filter_ops_t moving_average_filter_ops;

sensor_listener_t gui_create_listener(gui_display_model_t *model);
sensor_listener_t mqtt_create_listener(mqtt_publish_model_t *model, const char *topic);
sensor_listener_t business_create_listener(business_rule_model_t *model);

#endif
