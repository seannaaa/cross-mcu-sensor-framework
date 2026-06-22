#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "business_app.h"
#include "esp32_platform.h"
#include "gui_app.h"
#include "mqtt_app.h"
#include "sensor_manager.h"
#include "sensor_filter.h"
#include "temperature_driver.h"

static void test_other_filters(void)
{
    /* 这里单独验证几种常见滤波器本身的行为是否符合预期。 */
    sensor_sample_t sample;

    /* bypass: 输入多少，输出多少，常用于 bring-up 和调试阶段。 */
    bypass_filter_t bypass_filter = {0};
    sensor_filter_t bypass = {
        .context = &bypass_filter,
        .ops = &bypass_filter_ops,
    };

    /* median3: 适合抑制单点尖峰噪声。 */
    median3_filter_t median_filter = {0};
    sensor_filter_t median = {
        .context = &median_filter,
        .ops = &median3_filter_ops,
    };

    /* ema: 用 alpha 控制平滑程度，这里取 0.5 方便算结果。 */
    ema_filter_t ema_filter = {
        .alpha = 0.5f,
    };
    sensor_filter_t ema = {
        .context = &ema_filter,
        .ops = &ema_filter_ops,
    };

    /* 验证 bypass 不应该修改输入值。 */
    assert(bypass.ops->init(bypass.context) == 0);
    sample.value = 12.34f;
    assert(bypass.ops->process(bypass.context, &sample) == 0);
    assert(fabsf(sample.value - 12.34f) < 0.001f);

    /*
     * 验证 median3:
     * 第 1、2 个点只是填历史值；
     * 第 3 个点进来后，应该从 10 / 100 / 11 里取中值 11。
     */
    assert(median.ops->init(median.context) == 0);
    sample.value = 10.0f;
    assert(median.ops->process(median.context, &sample) == 0);
    assert(fabsf(sample.value - 10.0f) < 0.001f);
    sample.value = 100.0f;
    assert(median.ops->process(median.context, &sample) == 0);
    assert(fabsf(sample.value - 100.0f) < 0.001f);
    sample.value = 11.0f;
    assert(median.ops->process(median.context, &sample) == 0);
    assert(fabsf(sample.value - 11.0f) < 0.001f);

    /*
     * 验证 ema:
     * 第一个值直接作为初始输出；
     * 第二个值 30 进来后，0.5 * 30 + 0.5 * 20 = 25。
     */
    assert(ema.ops->init(ema.context) == 0);
    sample.value = 20.0f;
    assert(ema.ops->process(ema.context, &sample) == 0);
    assert(fabsf(sample.value - 20.0f) < 0.001f);
    sample.value = 30.0f;
    assert(ema.ops->process(ema.context, &sample) == 0);
    assert(fabsf(sample.value - 25.0f) < 0.001f);
}

static void test_full_pipeline_with_mock_i2c(void)
{
    /*
     * 这里验证完整数据链路：
     * esp32 platform -> temperature driver -> sensor_manager
     * -> moving_average -> gui/mqtt/business
     */
    esp32_platform_t platform = {
        .i2c_port = 0,
        .power_pin = 21,
        .pwm_channel = 2,
        .tick_ms_cache = 1234U,
        .mock_raw_temperature = 2534,
    };

    /* 通过 create_bundle 把 ESP32 这一套函数指针统一打包给上层使用。 */
    platform_bundle_t bundle = esp32_platform_create_bundle(&platform);

    /* raw_buffer 模拟 sensor 原始寄存器读回来的 2 字节数据。 */
    uint8_t raw_buffer[2] = {0};

    /* driver 只认抽象 platform ops，不关心底下到底是什么芯片 SDK。 */
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

    /* sensor_driver 是 manager 层真正拿来调用的统一驱动抽象。 */
    sensor_driver_t sensor_driver = {
        .context = &driver,
        .ops = &temperature_driver_ops,
    };

    /* 这里给完整链路挂一个 moving average filter。 */
    moving_average_filter_t average_filter = {0};
    sensor_filter_t filter = {
        .context = &average_filter,
        .ops = &moving_average_filter_ops,
    };

    /* 这三个 model 分别模拟 GUI、MQTT、业务层收到 sample 后的最终状态。 */
    gui_display_model_t gui_model = {0};
    mqtt_publish_model_t mqtt_model = {0};
    business_rule_model_t business_model = {
        .alert_threshold = 26.0f,
    };

    /* listeners 代表 sample 分发后的三个消费方。 */
    sensor_listener_t listeners[] = {
        gui_create_listener(&gui_model),
        mqtt_create_listener(&mqtt_model, "sensor/temperature"),
        business_create_listener(&business_model),
    };

    /* manager 是主干调度器，负责把整条链串起来。 */
    sensor_manager_t manager = {
        .driver = &sensor_driver,
        .filter = &filter,
        .listeners = listeners,
        .listener_count = sizeof(listeners) / sizeof(listeners[0]),
        .timestamp_ms = bundle.timer.get_tick_ms(bundle.timer.context),
    };

    /*
     * 初始化阶段会打到 driver init，
     * driver init 里又会通过函数指针触发 gpio/pwm/delay。
     */
    assert(sensor_manager_init(&manager) == 0);
    assert(platform.last_gpio_pin == 21);
    assert(platform.last_gpio_value == 1);
    assert(platform.last_pwm_channel == 2);
    assert(platform.last_pwm_frequency_hz == 1000U);
    assert(platform.last_delay_ms == 10U);

    /*
     * 第一次采样：
     * raw 2534 -> 25.34C
     * moving average 第一次不会改值
     * 业务阈值是 26，所以此时不告警
     */
    assert(sensor_manager_read_once(&manager) == 0);
    assert(fabsf(gui_model.last_temperature - 25.34f) < 0.001f);
    assert(strcmp(mqtt_model.last_payload, "{\"value\":25.34,\"unit\":\"C\"}") == 0);
    assert(business_model.alert_active == 0);

    /*
     * 第二次采样：
     * raw 2734 -> 27.34C
     * moving average 会对 25.34 和 27.34 做平均，结果 26.34C
     * 因为已经超过 26，所以业务层会触发告警
     */
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
    /* 这里不是做 assert，而是把整条链路最终结果直接打印出来。 */
    esp32_platform_t platform = {
        .i2c_port = 0,
        .power_pin = 21,
        .pwm_channel = 2,
        .tick_ms_cache = 1000U,
        .mock_raw_temperature = 2866,
    };

    /* 固定选用 ESP32 平台函数指针表。 */
    platform_bundle_t bundle = esp32_platform_create_bundle(&platform);
    uint8_t raw_buffer[2] = {0};

    /* 这一段和测试链路一致，目的是让 demo 和真实验证走同一套路径。 */
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

    /* demo 里同样挂一个 moving average，方便观察 filter 的位置。 */
    moving_average_filter_t average_filter = {0};
    sensor_filter_t filter = {
        .context = &average_filter,
        .ops = &moving_average_filter_ops,
    };

    /* 这里构造三个应用层消费者。 */
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

    /* manager 负责把 driver / filter / listeners 串成一条完整链。 */
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

    /* 这里输出的是 GUI / MQTT / business 三个消费方最终拿到的结果。 */
    printf("[esp32] GUI temperature: %.2f C\n", gui_model.last_temperature);
    printf("[esp32] MQTT topic: %s\n", mqtt_model.last_topic);
    printf("[esp32] MQTT payload: %s\n", mqtt_model.last_payload);
    printf("[esp32] Business alert: %d\n", business_model.alert_active);

    return 0;
}

int main(void)
{
    /* 先跑一遍断言测试，确认整条数据链路逻辑正确。 */
    test_other_filters();
    test_full_pipeline_with_mock_i2c();

    /* 再跑一遍 ESP32 demo，把 GUI/MQTT/business 最终结果打印出来。 */
    return run_esp32_demo();
}
