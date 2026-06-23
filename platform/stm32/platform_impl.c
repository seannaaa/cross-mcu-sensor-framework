#include "platform_impl.h"

#include <stddef.h>

static int stm32_i2c_read(void *context,
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

    /* 这里继续用 mock 温度值来模拟 STM32 侧寄存器读回的数据。 */
    buffer[0] = (uint8_t)((platform->mock_raw_temperature >> 8) & 0xFF);
    buffer[1] = (uint8_t)(platform->mock_raw_temperature & 0xFF);
    return 0;
}

static int stm32_gpio_write(void *context, int pin, int value)
{
    platform_t *platform = (platform_t *)context;

    if (platform == NULL) {
        return -1;
    }

    /* 这里记录最近一次 GPIO 操作，便于测试断言。 */
    platform->last_gpio_pin = pin;
    platform->last_gpio_value = value;
    return 0;
}

static int stm32_pwm_set(void *context, int channel, uint32_t frequency_hz, float duty_cycle)
{
    platform_t *platform = (platform_t *)context;

    if (platform == NULL) {
        return -1;
    }

    /* 这里记录最近一次 PWM 配置，模拟定时器 PWM 输出。 */
    platform->last_pwm_channel = channel;
    platform->last_pwm_frequency_hz = frequency_hz;
    platform->last_pwm_duty_cycle = duty_cycle;
    return 0;
}

static uint32_t stm32_get_tick_ms(void *context)
{
    platform_t *platform = (platform_t *)context;

    if (platform == NULL) {
        return 0U;
    }

    return platform->tick_ms;
}

static void stm32_delay_ms(void *context, uint32_t delay_ms)
{
    platform_t *platform = (platform_t *)context;

    if (platform == NULL) {
        return;
    }

    /* 这里不做真实阻塞，只缓存参数给上层测试读取。 */
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

    /* 统一对上层暴露 platform_bundle_t，上层不用关心底层是 STM32。 */
    bundle.i2c.context = platform;
    bundle.i2c.read = stm32_i2c_read;
    bundle.i2c.write = NULL;

    bundle.gpio.context = platform;
    bundle.gpio.write = stm32_gpio_write;

    bundle.pwm.context = platform;
    bundle.pwm.set = stm32_pwm_set;

    bundle.timer.context = platform;
    bundle.timer.get_tick_ms = stm32_get_tick_ms;

    bundle.delay.context = platform;
    bundle.delay.delay_ms = stm32_delay_ms;

    return bundle;
}
