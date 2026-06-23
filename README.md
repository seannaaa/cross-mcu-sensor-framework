# cross-mcu-sensor-framework

一个跨 MCU / SoC 的 sensor framework demo，重点是把这条数据链路讲清楚：

```text
platform -> driver -> manager/model -> biz(filter/gui/mqtt/business) -> test
```

这版结构按你确认的方式收敛成：

```text
platform/
drivers/
model/
biz/
test/
docs/
```

## 1. 目录说明

### `platform/`

放不同芯片平台实现。

当前有：

- [platform/platform.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/platform.h:1)
- [platform/platform_config.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/platform_config.h:1)
- [platform/platform_ops.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/platform_ops.h:1)
- [platform/esp32/platform_impl.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/esp32/platform_impl.c:1)
- [platform/stm32/platform_impl.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/stm32/platform_impl.c:1)

这里负责：

- 函数指针绑定
- I2C
- GPIO
- PWM
- TIMER
- DELAY

### `drivers/`

放具体 sensor driver。

当前是：

- [drivers/temperature_driver.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/drivers/temperature_driver.h:1)
- [drivers/temperature_driver.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/drivers/temperature_driver.c:1)

### `model/`

放通用数据结构和 manager 主链路。

当前有：

- [model/sensor_types.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/model/sensor_types.h:1)
- [model/sensor_manager.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/model/sensor_manager.h:1)
- [model/sensor_manager.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/model/sensor_manager.c:1)

### `biz/`

放 filter、GUI、MQTT、business 这些上层消费逻辑。

当前有：

- [biz/sensor_filter.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/sensor_filter.h:1)
- [biz/sensor_filter.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/sensor_filter.c:1)
- [biz/gui_app.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/gui_app.h:1)
- [biz/gui_app.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/gui_app.c:1)
- [biz/mqtt_app.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/mqtt_app.h:1)
- [biz/mqtt_app.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/mqtt_app.c:1)
- [biz/business_app.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/business_app.h:1)
- [biz/business_app.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/business_app.c:1)

### `test/`

只保留一个测试文件：

- [test/test_sensor_framework.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/test/test_sensor_framework.c:1)

这一个文件里同时做了：

- 完整链路测试
- 当前构建平台的函数指针绑定 demo
- 最终输出 demo
- 注释说明为什么应用层不用再写平台分支

## 2. 代码链路怎么读

建议按这个顺序看：

1. [platform/platform_ops.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/platform_ops.h:1)
2. [platform/platform_config.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/platform_config.h:1)
3. [platform/platform.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/platform.h:1)
4. [platform/esp32/platform_impl.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/esp32/platform_impl.c:1)
5. [platform/stm32/platform_impl.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/platform/stm32/platform_impl.c:1)
6. [drivers/temperature_driver.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/drivers/temperature_driver.h:1)
7. [drivers/temperature_driver.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/drivers/temperature_driver.c:1)
8. [model/sensor_types.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/model/sensor_types.h:1)
9. [model/sensor_manager.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/model/sensor_manager.h:1)
10. [model/sensor_manager.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/model/sensor_manager.c:1)
11. [biz/sensor_filter.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/sensor_filter.h:1)
12. [biz/sensor_filter.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/sensor_filter.c:1)
13. [biz/gui_app.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/gui_app.h:1)
14. [biz/gui_app.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/gui_app.c:1)
15. [biz/mqtt_app.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/mqtt_app.h:1)
16. [biz/mqtt_app.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/mqtt_app.c:1)
17. [biz/business_app.h](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/business_app.h:1)
18. [biz/business_app.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/biz/business_app.c:1)
19. [test/test_sensor_framework.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/test/test_sensor_framework.c:1)

## 3. 函数指针怎么对应到 ESP32 / STM32

这件事的核心不是 driver 里写分支，而是当前构建平台各自实现同名的 `platform_create_bundle(...)`，统一生成 `platform_bundle_t`。

### 统一入口

应用层现在只需要：

- `platform_t`
- `platform_create_bundle(...)`

本质是：

```c
platform_t platform = {
    .i2c_bus = 0,
    .power_pin = 21,
    .pwm_channel = 2,
    .tick_ms = 1000U,
    .mock_raw_temperature = 2866,
};
platform_bundle_t bundle = platform_create_bundle(&platform);
```

### 平台实现

真正的平台差异下沉到：

- `platform/esp32/platform_impl.c`
- `platform/stm32/platform_impl.c`

不同平台各自把本地函数绑定到统一 bundle：

```c
bundle.i2c.read = xxx_i2c_read;
bundle.gpio.write = xxx_gpio_write;
bundle.pwm.set = xxx_pwm_set;
bundle.timer.get_tick_ms = xxx_get_tick_ms;
bundle.delay.delay_ms = xxx_delay_ms;
```

然后同一个 `temperature_driver_t` 只依赖：

- `i2c`
- `gpio`
- `pwm`
- `delay`

所以：

- 切 ESP32，就执行 `./build.sh esp32`
- 切 STM32，就执行 `./build.sh stm32`

driver 和 manager 主链路完全不用改。

当前 `test` 会随构建平台跑对应实现，不再把 ESP32 写死在应用层。

## 4. 当前 temperature driver 思路

[drivers/temperature_driver.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/drivers/temperature_driver.c:1) 里做了两件事：

### init 阶段

主动调用：

- `gpio->write(...)`
- `pwm->set(...)`
- `delay->delay_ms(...)`

这样是为了让你能明确看到：

- 函数指针不只是声明了
- 而是真的从 driver 层打到具体平台实现

### read 阶段

调用：

- `i2c->read(...)`

然后把读到的 2 字节原始值转成：

- `sensor_sample_t`

当前示例按 `0.01°C` 分辨率处理。

## 5. manager 主链路

[model/sensor_manager.c](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/model/sensor_manager.c:1) 是整个题里最关键的主干。

它负责：

1. 调 driver 读 sample
2. 写入 timestamp
3. 调 filter
4. 分发给 GUI / MQTT / business

也就是：

```text
driver -> sample -> filter -> listeners
```

## 6. biz 层做了什么

`biz/` 现在不再挤在一个文件里，而是按角色拆开：

- `sensor_filter.*`
  - 放滤波逻辑
- `gui_app.*`
  - 放 GUI 消费逻辑
- `mqtt_app.*`
  - 放 MQTT 消费逻辑
- `business_app.*`
  - 放业务判断逻辑

这里就是你要的“整个笔试题的数据链路”在上层的落点：

- filter：改 sample
- gui：显示 sample
- mqtt：发布 sample
- business：根据 sample 做阈值判断

当前已经补了几种常用滤波：

- `bypass`
  - 直接透传，适合 bring-up / debug
- `moving average`
  - 简单平滑
- `median3`
  - 适合压单点尖峰噪声
- `ema/iir`
  - 低内存、响应快、适合持续采样场景

## 7. build.sh

构建脚本还是保留：

- [build.sh](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/build.sh:1)

用法：

```bash
./build.sh esp32
./build.sh stm32
./build.sh linux_native
```

平台名会传给 CMake 的 `MCU_PLATFORM`，构建目录会分开：

```text
build/esp32/
build/stm32/
build/linux_native/
```

## 8. 如何运行

构建：

```bash
./build.sh linux_native
```

跑测试和 demo：

```bash
./build/linux_native/sensor_framework_tests
```

这个单文件测试会同时输出整条 demo 链路结果。

## 9. 当前验证结果

我实际跑过：

```bash
ctest --test-dir /home2/sean/BlueAirProject/cross-mcu-sensor-framework/build/linux_native --output-on-failure
```

结果：

- `100% tests passed, 0 tests failed out of 1`

当前 demo 输出类似：

```text
[esp32] GUI temperature: 28.66 C
[esp32] MQTT topic: demo/temperature
[esp32] MQTT payload: {"value":28.66,"unit":"C"}
[esp32] Business alert: 1
```
