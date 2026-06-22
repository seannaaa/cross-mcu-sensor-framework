#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "esp32_platform.h"
#include "sample_biz.h"
#include "sensor_manager.h"
#include "temperature_driver.h"

static void test_full_pipeline_with_mock_i2c(void)
{
    esp32_platform_t platform = {
        .i2c_port = 0,
        .power_pin = 21,
        .pwm_channel = 2,
        .tick_ms_cache = 1234U,
        .mock_raw_temperature = 2534,
    };
    platform_bundle_t bundle = esp32_platform_create_bundle(&platform);
    uint8_t raw_buffer[2] = {0};
    temperature_driver_t driver = {
        .device_address = 0x48,
        .temperature_register = 0x00,
        .i2c = &bundle.i2c,
        .gpio = &bundle.gpio,
        .pwm = &bundle.pwm,
        .timer = &bundle.timer,
        .delay = &bundle.delay,
        .power_pin = platform.power_pin,
        .pwm_channel = platform.pwm_channel,
        .raw_buffer = raw_buffer,
    };
    sensor_driver_t sensor_driver = {
        .context = &driver,
        .ops = &temperature_driver_ops,
    };
    moving_average_filter_t average_filter = {0};
    sensor_filter_t filter = {
        .context = &average_filter,
        .ops = &moving_average_filter_ops,
    };
    gui_display_model_t gui_model = {0};
    mqtt_publish_model_t mqtt_model = {0};
    business_rule_model_t business_model = {
        .alert_threshold = 26.0f,
    };
    sensor_listener_t listeners[] = {
        gui_create_listener(&gui_model),
        mqtt_create_listener(&mqtt_model, "sensor/temperature"),
        business_create_listener(&business_model),
    };
    sensor_manager_t manager = {
        .driver = &sensor_driver,
        .filter = &filter,
        .listeners = listeners,
        .listener_count = sizeof(listeners) / sizeof(listeners[0]),
        .timestamp_ms = bundle.timer.get_tick_ms(bundle.timer.context),
    };

    assert(sensor_manager_init(&manager) == 0);
    assert(platform.last_gpio_pin == 21);
    assert(platform.last_gpio_value == 1);
    assert(platform.last_pwm_channel == 2);
    assert(platform.last_pwm_frequency_hz == 1000U);
    assert(platform.last_delay_ms == 10U);

    assert(sensor_manager_read_once(&manager) == 0);
    assert(fabsf(gui_model.last_temperature - 25.34f) < 0.001f);
    assert(strcmp(mqtt_model.last_payload, "{\"value\":25.34,\"unit\":\"C\"}") == 0);
    assert(business_model.alert_active == 0);

    platform.mock_raw_temperature = 2734;
    manager.timestamp_ms = 2234U;
    assert(sensor_manager_read_once(&manager) == 0);
    assert(fabsf(gui_model.last_temperature - 26.34f) < 0.001f);
    assert(gui_model.last_timestamp_ms == 2234U);
    assert(strcmp(mqtt_model.last_payload, "{\"value\":26.34,\"unit\":\"C\"}") == 0);
    assert(business_model.alert_active == 1);
}

static int run_esp32_demo(void)
{
    esp32_platform_t platform = {
        .i2c_port = 0,
        .power_pin = 21,
        .pwm_channel = 2,
        .tick_ms_cache = 1000U,
        .mock_raw_temperature = 2866,
    };
    platform_bundle_t bundle = esp32_platform_create_bundle(&platform);
    uint8_t raw_buffer[2] = {0};
    temperature_driver_t driver = {
        .device_address = 0x48,
        .temperature_register = 0x00,
        .i2c = &bundle.i2c,
        .gpio = &bundle.gpio,
        .pwm = &bundle.pwm,
        .timer = &bundle.timer,
        .delay = &bundle.delay,
        .power_pin = platform.power_pin,
        .pwm_channel = platform.pwm_channel,
        .raw_buffer = raw_buffer,
    };
    sensor_driver_t sensor_driver = {
        .context = &driver,
        .ops = &temperature_driver_ops,
    };
    moving_average_filter_t average_filter = {0};
    sensor_filter_t filter = {
        .context = &average_filter,
        .ops = &moving_average_filter_ops,
    };
    gui_display_model_t gui_model = {0};
    mqtt_publish_model_t mqtt_model = {0};
    business_rule_model_t business_model = {
        .alert_threshold = 28.0f,
    };
    sensor_listener_t listeners[] = {
        gui_create_listener(&gui_model),
        mqtt_create_listener(&mqtt_model, "demo/temperature"),
        business_create_listener(&business_model),
    };
    sensor_manager_t manager = {
        .driver = &sensor_driver,
        .filter = &filter,
        .listeners = listeners,
        .listener_count = sizeof(listeners) / sizeof(listeners[0]),
        .timestamp_ms = bundle.timer.get_tick_ms(bundle.timer.context),
    };

    /*
     * 这里的 demo 固定按 ESP32 跑一条完整链路：
     * platform -> driver -> manager -> filter -> gui/mqtt/business
     *
     * 如果后面要切换 STM32 或其他芯片，
     * 只需要把 esp32_platform_create_bundle(...) 换成对应平台的 create_bundle(...)，
     * driver / manager / biz 层不用改。
     */
    if (sensor_manager_init(&manager) != 0) {
        return 1;
    }
    if (sensor_manager_read_once(&manager) != 0) {
        return 1;
    }

    printf("[esp32] GUI temperature: %.2f C\n", gui_model.last_temperature);
    printf("[esp32] MQTT topic: %s\n", mqtt_model.last_topic);
    printf("[esp32] MQTT payload: %s\n", mqtt_model.last_payload);
    printf("[esp32] Business alert: %d\n", business_model.alert_active);

    return 0;
}

int main(void)
{
    /* 先跑一遍断言测试，确认整条数据链路逻辑正确。 */
    test_full_pipeline_with_mock_i2c();

    /* 再跑一遍 ESP32 demo，把 GUI/MQTT/business 最终结果打印出来。 */
    return run_esp32_demo();
}
