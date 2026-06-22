#include "temperature_driver.h"

static int temperature_driver_init(void *context)
{
    temperature_driver_t *driver = (temperature_driver_t *)context;

    if ((driver == NULL) || (driver->i2c == NULL) || (driver->i2c->read == NULL) || (driver->raw_buffer == NULL)) {
        return -1;
    }

    /*
     * 这里故意在 init 阶段调用 gpio / pwm / delay，
     * 用来演示同一个 driver 如何通过函数指针切到不同平台实现。
     */
    if ((driver->gpio != NULL) && (driver->gpio->write != NULL)) {
        driver->gpio->write(driver->gpio->context, driver->power_pin, 1);
    }

    if ((driver->pwm != NULL) && (driver->pwm->set != NULL)) {
        driver->pwm->set(driver->pwm->context, driver->pwm_channel, 1000U, 0.5f);
    }

    if ((driver->delay != NULL) && (driver->delay->delay_ms != NULL)) {
        driver->delay->delay_ms(driver->delay->context, 10U);
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

    /* 这里先用 0.01 摄氏度分辨率做最小示例，后续可按具体芯片再扩展。 */
    sample->type = SENSOR_TYPE_TEMPERATURE;
    sample->unit = SENSOR_UNIT_CELSIUS;
    sample->status = SENSOR_STATUS_OK;
    sample->timestamp_ms = 0U;
    sample->value = (float)raw_value / 100.0f;

    return 0;
}

const sensor_driver_ops_t temperature_driver_ops = {
    .init = temperature_driver_init,
    .read_sample = temperature_driver_read_sample,
};
