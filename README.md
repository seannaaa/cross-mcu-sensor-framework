# cross-mcu-sensor-framework

一个跨 MCU / SoC 的 sensor framework，目标是：

- `跨平台`：适配不同芯片、不同 BSP/HAL
- `可扩展`：方便增加新 sensor / 新 filter
- `解耦`：GUI、MQTT、业务逻辑都只消费统一 sample
- `可演进`：当前用 C 实现，但按 C++ abstraction 的思路设计

## 1. Design Target

这个项目优先用 `C` 来实现，因为：

- 更容易跨 MCU / RTOS / Linux 平台
- 更适合和 BSP/HAL/vendor SDK 集成
- ABI 边界更稳定

但设计思路要偏 `C++ style`：

- 面向接口，不面向具体芯片
- 上层依赖抽象 `ops`
- 各层职责单一
- 后续如果切到 C++，不用推翻架构

## 2. Core Idea

整体数据流保持单向：

```text
platform -> driver -> core -> filter -> dispatch -> adapters
```

也就是：

```text
硬件平台接口
  -> sensor driver
  -> sample 标准化
  -> filter 处理
  -> event dispatch
  -> GUI / MQTT / business logic
```

## 3. 为什么先做 Hardware Platform Layer

不同平台下，很多硬件接口写法都不一样，比如：

- `i2c`
- `spi`
- `adc`
- `gpio`
- `pwm`
- `timer/tick`
- `delay`
- `lock/mutex`

尤其你提到的 `pwm`，不同芯片的配置方式、频率设置、占空比接口都可能不同。

所以这个框架里，sensor driver 不能直接写芯片 SDK 调用，而是统一走平台抽象层，比如：

- `platform_i2c_ops`
- `platform_gpio_ops`
- `platform_pwm_ops`
- `platform_timer_ops`
- `platform_delay_ops`

这套写法本质上就是 `C 版 interface / pure virtual` 思路。

## 4. 推荐分层

### `Hardware Platform Layer`

负责平台差异收口：

- I2C / SPI / ADC / GPIO / PWM
- tick / delay
- mutex / lock

规则：

- 所有芯片差异都沉到这里
- 不允许上层直接碰寄存器 / BSP / HAL / SDK

### `Sensor Driver Layer`

负责具体 sensor 芯片逻辑，比如温度传感器驱动：

- init
- read raw data
- raw -> engineering value
- optional reset / power control

规则：

- 只能依赖 `platform_*_ops`
- 不能耦合 GUI / MQTT / business

### `Sensor Core`

负责统一 sample 管理：

- sampling flow
- timestamp / unit / status
- 调度 driver

核心抽象建议：

- `sensor_sample_t`
- `sensor_manager`

### `Filter Layer`

负责滤波，推荐先支持：

- `bypass`
- `moving average`
- `median`
- `EMA / IIR`

核心抽象建议：

- `sensor_filter_ops`

### `Dispatch/Event Layer`

负责把 sample 广播给多个消费者：

- GUI
- MQTT
- business logic

核心抽象建议：

- `sensor_event_listener`

### `Application Adapters`

上层适配层只消费 sample：

- GUI adapter
- MQTT adapter
- Business adapter

规则：

- 不能直接访问 driver
- 不能直接访问 platform 层

## 5. 温度 Sensor 示例

首个完整案例固定为 `temperature sensor`。

标准流程：

```text
platform ops
  -> temperature driver read
  -> normalize to sensor_sample_t
  -> filter
  -> dispatch
  -> GUI / MQTT / business consumers
```

例如：

- `platform_i2c_ops`：读温度芯片寄存器
- `platform_gpio_ops`：控制 sensor 电源或 reset
- `platform_pwm_ops`：如果业务后面有风扇/背光/蜂鸣器联动，也保持平台抽象

## 6. 参考工程

### MQTT 参考

路径：

`/home2/sean/BlueAirProject/SampleProject/XQD-GYJ-CAT1-APP`

参考点：

- 通信层 / 协议层 / 应用层分离
- MQTT 适合作为 adapter，而不是 core 一部分

### UI 跨平台 porting 参考

路径：

`/home/sean/gitlab_lvgl_template/lvgl_d21x/common/porting`

参考点：

- 按平台拆 `porting`
- lock / time / display / input 分边界
- 这种思路可直接扩展到 sensor hardware 抽象

### UI 与代码架构参考

路径：

`/home/sean/gitlab_lvgl_template/app_d12x_d21x_ssd202_mono【兼容4者】`

参考点：

- `pages / view / model / biz / data / uikit`
- GUI、业务、数据层与底层驱动分开

## 7. 推荐目录

```text
platform/
drivers/
core/
filters/
adapters/
examples/temperature/
tests/
docs/
```

如果后面要细化，建议：

```text
platform/
  include/
  <board_or_chip>/

drivers/
  include/
  temperature/

adapters/
  gui/
  mqtt/
  business/
```

## 8. Public Interface 建议

核心接口先收敛到这些名字：

- `platform_i2c_ops`
- `platform_spi_ops`
- `platform_gpio_ops`
- `platform_pwm_ops`
- `platform_timer_ops`
- `platform_delay_ops`
- `sensor_driver_ops`
- `sensor_filter_ops`
- `sensor_sample_t`
- `sensor_event_listener`
- `sensor_manager`

## 9. 后续切 C++ 时的映射

当前用 C 命名，后面可以自然映射成 C++：

- `platform_*_ops` -> platform interface class
- `sensor_driver_ops` -> abstract driver
- `sensor_filter_ops` -> strategy
- `sensor_event_listener` -> observer/listener
- `sensor_manager` -> controller / orchestrator

## 10. 最重要的规则

- 平台差异必须收口到 `platform layer`
- sensor driver 不能直接写芯片 SDK
- filter 不要混入业务逻辑
- GUI / MQTT / business 只能消费 sample
- 上层不要到处写芯片分支 `#ifdef`

## 11. docs

完整设计文档见：

- [cross-mcu-sensor-framework-design.md](/home2/sean/BlueAirProject/cross-mcu-sensor-framework/docs/cross-mcu-sensor-framework-design.md)
