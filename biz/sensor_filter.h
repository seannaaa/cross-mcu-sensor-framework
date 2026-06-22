#ifndef SENSOR_FILTER_H
#define SENSOR_FILTER_H

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
    /* bypass 不改变输入值，适合 bring-up 或调试阶段。 */
    int reserved;
} bypass_filter_t;

typedef struct {
    /* 这里先用 2 点历史值演示 moving average 的最小实现。 */
    float history[2];
    unsigned int count;
} moving_average_filter_t;

typedef struct {
    /* median3 适合抑制单点尖峰噪声。 */
    float history[3];
    unsigned int count;
} median3_filter_t;

typedef struct {
    /* alpha 越小越平滑，越大越跟手。 */
    float alpha;
    float last_output;
    unsigned int initialized;
} ema_filter_t;

extern const sensor_filter_ops_t bypass_filter_ops;
extern const sensor_filter_ops_t moving_average_filter_ops;
extern const sensor_filter_ops_t median3_filter_ops;
extern const sensor_filter_ops_t ema_filter_ops;

#endif
