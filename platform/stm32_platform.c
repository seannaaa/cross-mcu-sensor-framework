#include "stm32_platform.h"

/*
 * 这是 STM32 平台 demo。
 * 重点是演示函数指针如何绑定到统一 platform ops。
 */

static int stm32_i2c_read(void *context,
                          uint8_t device_address,
                          uint8_t register_address,
                          uint8_t *buffer,
                          size_t buffer_size)
{
    stm32_platform_t *platform = (stm32_platform_t *)context;
    (void)device_address;
    (void)register_address;

    if ((platform == NULL) || (buffer == NULL) || (buffer_size < 2U)) {
        return -1;
    }

    /* 真实 STM32 项目里这里通常会接 HAL_I2C_Mem_Read。 */
    buffer[0] = (uint8_t)((platform->mock_raw_temperature >> 8) & 0xFF);
    buffer[1] = (uint8_t)(platform->mock_raw_temperature & 0xFF);
    return 0;
}

static int stm32_gpio_write(void *context, int pin, int value)
{
    stm32_platform_t *platform = (stm32_platform_t *)context;

    if (platform == NULL) {
        return -1;
    }

    /* 真实 STM32 项目里这里通常会接 HAL_GPIO_WritePin。 */
    platform->last_gpio_pin = pin;
    platform->last_gpio_value = value;
    return 0;
}

static int stm32_pwm_set(void *context, int channel, uint32_t frequency_hz, float duty_cycle)
{
    stm32_platform_t *platform = (stm32_platform_t *)context;

    if (platform == NULL) {
        return -1;
    }

    /* 真实 STM32 项目里这里通常会接 TIM PWM 配置和启动。 */
    platform->last_pwm_channel = channel;
    platform->last_pwm_frequency_hz = frequency_hz;
    platform->last_pwm_duty_cycle = duty_cycle;
    return 0;
}

static uint32_t stm32_get_tick_ms(void *context)
{
    stm32_platform_t *platform = (stm32_platform_t *)context;
    return platform->systick_ms_cache;
}

static void stm32_delay_ms(void *context, uint32_t delay_ms)
{
    stm32_platform_t *platform = (stm32_platform_t *)context;

    if (platform == NULL) {
        return;
    }

    /* 真实 STM32 项目里这里通常会接 HAL_Delay。 */
    platform->last_delay_ms = delay_ms;
}

platform_bundle_t stm32_platform_create_bundle(stm32_platform_t *platform)
{
    platform_bundle_t bundle;

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
