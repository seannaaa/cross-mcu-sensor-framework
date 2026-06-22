#include "sensor_driver.h"

#include <stddef.h>

static int temperature_driver_init(void *context)
{
    temperature_driver_t *driver = (temperature_driver_t *)context;

    if ((driver == NULL) || (driver->i2c == NULL) || (driver->i2c->read == NULL) || (driver->raw_buffer == NULL)) {
        return -1;
    }

    return 0;
}

static int temperature_driver_read_sample(void *context, sensor_sample_t *sample)
{
    temperature_driver_t *driver = (temperature_driver_t *)context;
    int16_t raw_value;
    int result;

    if ((driver == NULL) || (sample == NULL) || (driver->i2c == NULL) || (driver->i2c->read == NULL) ||
        (driver->raw_buffer == NULL)) {
        return -1;
    }

    result = driver->i2c->read(driver->i2c->context,
                               driver->device_address,
                               driver->temperature_register,
                               driver->raw_buffer,
                               2U);
    if (result != 0) {
        return result;
    }

    raw_value = (int16_t)(((uint16_t)driver->raw_buffer[0] << 8) | driver->raw_buffer[1]);

    /* 这里先用 0.01 摄氏度分辨率做最小示例，后续可以按具体芯片再扩展。 */
    sample->type = SENSOR_TYPE_TEMPERATURE;
    sample->unit = SENSOR_UNIT_CELSIUS;
    sample->status = SENSOR_STATUS_OK;
    sample->timestamp_ms = 0U;
    sample->value = (float)raw_value / 100.0f;

    return 0;
}

const sensor_driver_ops_t temperature_sensor_driver_ops = {
    .init = temperature_driver_init,
    .read_sample = temperature_driver_read_sample,
};
