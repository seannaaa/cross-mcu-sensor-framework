#include "sensor_filter.h"

#include <stddef.h>

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

    /* 用 2 点平均做最小实现，方便先把框架主链跑通。 */
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
