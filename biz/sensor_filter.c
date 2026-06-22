#include "sensor_filter.h"

static int bypass_init(void *context)
{
    bypass_filter_t *filter = (bypass_filter_t *)context;

    if (filter == NULL) {
        return -1;
    }

    filter->reserved = 0;
    return 0;
}

static int bypass_process(void *context, sensor_sample_t *sample)
{
    bypass_filter_t *filter = (bypass_filter_t *)context;

    if ((filter == NULL) || (sample == NULL)) {
        return -1;
    }

    /* bypass 直接透传，不修改 sample。 */
    return 0;
}

const sensor_filter_ops_t bypass_filter_ops = {
    .init = bypass_init,
    .process = bypass_process,
};

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

static float median3_value(float a, float b, float c)
{
    if (a > b) {
        float tmp = a;
        a = b;
        b = tmp;
    }

    if (b > c) {
        float tmp = b;
        b = c;
        c = tmp;
    }

    if (a > b) {
        float tmp = a;
        a = b;
        b = tmp;
    }

    return b;
}

static int median3_init(void *context)
{
    median3_filter_t *filter = (median3_filter_t *)context;

    if (filter == NULL) {
        return -1;
    }

    filter->history[0] = 0.0f;
    filter->history[1] = 0.0f;
    filter->history[2] = 0.0f;
    filter->count = 0U;
    return 0;
}

static int median3_process(void *context, sensor_sample_t *sample)
{
    median3_filter_t *filter = (median3_filter_t *)context;

    if ((filter == NULL) || (sample == NULL)) {
        return -1;
    }

    if (filter->count < 2U) {
        filter->history[filter->count] = sample->value;
        filter->count += 1U;
        return 0;
    }

    filter->history[2] = sample->value;
    sample->value = median3_value(filter->history[0], filter->history[1], filter->history[2]);
    filter->history[0] = filter->history[1];
    filter->history[1] = filter->history[2];
    return 0;
}

const sensor_filter_ops_t median3_filter_ops = {
    .init = median3_init,
    .process = median3_process,
};

static int ema_init(void *context)
{
    ema_filter_t *filter = (ema_filter_t *)context;

    if (filter == NULL) {
        return -1;
    }

    if ((filter->alpha <= 0.0f) || (filter->alpha > 1.0f)) {
        filter->alpha = 0.5f;
    }

    filter->last_output = 0.0f;
    filter->initialized = 0U;
    return 0;
}

static int ema_process(void *context, sensor_sample_t *sample)
{
    ema_filter_t *filter = (ema_filter_t *)context;

    if ((filter == NULL) || (sample == NULL)) {
        return -1;
    }

    if (filter->initialized == 0U) {
        filter->last_output = sample->value;
        filter->initialized = 1U;
        return 0;
    }

    /* EMA/IIR 用一个历史输出就能达到平滑效果，内存开销很小。 */
    filter->last_output = (filter->alpha * sample->value) + ((1.0f - filter->alpha) * filter->last_output);
    sample->value = filter->last_output;
    return 0;
}

const sensor_filter_ops_t ema_filter_ops = {
    .init = ema_init,
    .process = ema_process,
};
