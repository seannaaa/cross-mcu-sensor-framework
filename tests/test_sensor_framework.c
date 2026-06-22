#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "dispatch_listener.h"
#include "platform_ops.h"
#include "sensor_driver.h"
#include "sensor_filter.h"
#include "sensor_manager.h"

typedef struct {
    sensor_sample_t last_sample;
    int call_count;
} sample_capture_t;

typedef struct {
    int16_t raw_value;
} fake_i2c_context_t;

static int fake_i2c_read(void *context,
                         uint8_t device_address,
                         uint8_t register_address,
                         uint8_t *buffer,
                         size_t buffer_size)
{
    fake_i2c_context_t *fake = (fake_i2c_context_t *)context;
    (void)device_address;
    (void)register_address;

    assert(buffer_size >= 2U);
    buffer[0] = (uint8_t)((fake->raw_value >> 8) & 0xFF);
    buffer[1] = (uint8_t)(fake->raw_value & 0xFF);
    return 0;
}

static void capture_listener(void *context, const sensor_sample_t *sample)
{
    sample_capture_t *capture = (sample_capture_t *)context;
    capture->last_sample = *sample;
    capture->call_count += 1;
}

static void test_temperature_pipeline_with_filter_and_dispatch(void)
{
    fake_i2c_context_t fake_i2c = { .raw_value = 2534 };
    uint8_t raw_buffer[2] = {0};
    platform_i2c_ops_t i2c_ops = {
        .context = &fake_i2c,
        .read = fake_i2c_read,
        .write = NULL,
    };

    temperature_driver_t driver = {
        .device_address = 0x48,
        .temperature_register = 0x00,
        .i2c = &i2c_ops,
        .raw_buffer = raw_buffer,
    };

    sensor_driver_t sensor_driver = {
        .context = &driver,
        .ops = &temperature_sensor_driver_ops,
    };

    moving_average_filter_t average_filter = {0};
    sensor_filter_t filter = {
        .context = &average_filter,
        .ops = &moving_average_filter_ops,
    };

    sample_capture_t capture = {0};
    sensor_event_listener_t listener = {
        .context = &capture,
        .on_sample = capture_listener,
    };

    sensor_manager_t manager = {
        .driver = &sensor_driver,
        .filter = &filter,
        .listeners = &listener,
        .listener_count = 1U,
        .timestamp_ms = 1234U,
    };

    int init_result = sensor_manager_init(&manager);
    assert(init_result == 0);

    int read_result = sensor_manager_read_once(&manager);
    assert(read_result == 0);
    assert(capture.call_count == 1);
    assert(capture.last_sample.type == SENSOR_TYPE_TEMPERATURE);
    assert(capture.last_sample.unit == SENSOR_UNIT_CELSIUS);
    assert(capture.last_sample.timestamp_ms == 1234U);
    assert(fabsf(capture.last_sample.value - 25.34f) < 0.001f);

    fake_i2c.raw_value = 2734;
    manager.timestamp_ms = 2234U;

    read_result = sensor_manager_read_once(&manager);
    assert(read_result == 0);
    assert(capture.call_count == 2);
    assert(capture.last_sample.timestamp_ms == 2234U);
    assert(fabsf(capture.last_sample.value - 26.34f) < 0.001f);
}

int main(void)
{
    test_temperature_pipeline_with_filter_and_dispatch();
    return 0;
}
