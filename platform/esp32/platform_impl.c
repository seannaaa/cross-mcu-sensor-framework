#include "platform_impl.h"

#include <stddef.h>

static int esp32_i2c_read(void *context,
                          uint8_t device_address,
                          uint8_t register_address,
                          uint8_t *buffer,
                          size_t buffer_size)
{
    platform_t *platform = (platform_t *)context;
    (void)device_address;
    (void)register_address;

    if ((platform == NULL) || (buffer == NULL) || (buffer_size < 2U)) {
        return -1;
    }

    /* 这里继续用 mock 温度值来模拟真实寄存器返回的两个字节。 */
    buffer[0] = (uint8_t)((platform->mock_raw_temperature >> 8) & 0xFF);
    buffer[1] = (uint8_t)(platform->mock_raw_temperature & 0xFF);
    return 0;
}

static int esp32_gpio_write(void *context, int pin, int value)
{
    platform_t *platform = (platform_t *)context;

    if (platform == NULL) {
        return -1;
    }

    /* 这里记录最近一次 GPIO 写入，方便测试验证整条链路。 */
    platform->last_gpio_pin = pin;
    platform->last_gpio_value = value;
    return 0;
}

static int esp32_pwm_set(void *context, int channel, uint32_t frequency_hz, float duty_cycle)
{
    platform_t *platform = (platform_t *)context;

    if (platform == NULL) {
        return -1;
    }

    /* 这里记录最近一次 PWM 配置，模拟真实平台的驱动调用结果。 */
    platform->last_pwm_channel = channel;
    platform->last_pwm_frequency_hz = frequency_hz;
    platform->last_pwm_duty_cycle = duty_cycle;
    return 0;
}

static uint32_t esp32_get_tick_ms(void *context)
{
    platform_t *platform = (platform_t *)context;

    if (platform == NULL) {
        return 0U;
    }

    return platform->tick_ms;
}

static void esp32_delay_ms(void *context, uint32_t delay_ms)
{
    platform_t *platform = (platform_t *)context;

    if (platform == NULL) {
        return;
    }

    /* 这里不真的延时，只把延时参数缓存下来给测试读取。 */
    platform->last_delay_ms = delay_ms;
}

platform_bundle_t platform_create_bundle(platform_t *platform)
{
    platform_bundle_t bundle = {0};

    if (platform == NULL) {
        return bundle;
    }

    /* 这里把运行期观测字段清零，避免上层读到脏数据。 */
    platform->last_gpio_pin = 0;
    platform->last_gpio_value = 0;
    platform->last_pwm_channel = 0;
    platform->last_pwm_frequency_hz = 0U;
    platform->last_pwm_duty_cycle = 0.0f;
    platform->last_delay_ms = 0U;

    /* 统一对上层暴露 platform_bundle_t，上层不用知道底下是 ESP32。 */
    bundle.i2c.context = platform;
    bundle.i2c.read = esp32_i2c_read;
    bundle.i2c.write = NULL;

    bundle.gpio.context = platform;
    bundle.gpio.write = esp32_gpio_write;

    bundle.pwm.context = platform;
    bundle.pwm.set = esp32_pwm_set;

    bundle.timer.context = platform;
    bundle.timer.get_tick_ms = esp32_get_tick_ms;

    bundle.delay.context = platform;
    bundle.delay.delay_ms = esp32_delay_ms;

    return bundle;
}
