#include "sensor_manager.h"

#include <stddef.h>

#include "sensor_filter.h"
#include "temperature_driver.h"

int sensor_manager_init(sensor_manager_t *manager)
{
    if ((manager == NULL) || (manager->driver == NULL) || (manager->driver->ops == NULL)) {
        return -1;
    }

    if (manager->driver->ops->init != NULL) {
        int result = manager->driver->ops->init(manager->driver->context);
        if (result != 0) {
            return result;
        }
    }

    if ((manager->filter != NULL) && (manager->filter->ops != NULL) && (manager->filter->ops->init != NULL)) {
        return manager->filter->ops->init(manager->filter->context);
    }

    return 0;
}

int sensor_manager_read_once(sensor_manager_t *manager)
{
    size_t index;
    sensor_sample_t sample;
    int result;

    if ((manager == NULL) || (manager->driver == NULL) || (manager->driver->ops == NULL) ||
        (manager->driver->ops->read_sample == NULL)) {
        return -1;
    }

    result = manager->driver->ops->read_sample(manager->driver->context, &sample);
    if (result != 0) {
        return result;
    }

    sample.timestamp_ms = manager->timestamp_ms;

    /* 这里统一在 manager 层串联 filter，避免业务层各自重复处理。 */
    if ((manager->filter != NULL) && (manager->filter->ops != NULL) && (manager->filter->ops->process != NULL)) {
        result = manager->filter->ops->process(manager->filter->context, &sample);
        if (result != 0) {
            return result;
        }
    }

    for (index = 0; index < manager->listener_count; ++index) {
        if (manager->listeners[index].on_sample != NULL) {
            manager->listeners[index].on_sample(manager->listeners[index].context, &sample);
        }
    }

    return 0;
}
