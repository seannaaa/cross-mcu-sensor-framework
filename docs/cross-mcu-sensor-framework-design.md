# Cross MCU Sensor Framework Design

## 1. Summary

这个文档定义一个 `跨 MCU / SoC` 的 sensor framework。

实现语言优先是 `C`，但设计方式要按 `C++ 抽象层思维` 来组织，这样后面如果要切 C++，可以直接映射，不需要重做架构。

首个完整案例是：

- `temperature sensor`

目标：

- 硬件平台差异收口
- sensor driver 可替换
- filter 可替换
- GUI / MQTT / business 解耦

## 2. Overall Flow

推荐统一数据流：

```text
platform -> driver -> core -> filter -> dispatch -> adapters
```

展开后就是：

```text
platform ops
  -> sensor driver read
  -> normalize sample
  -> filter
  -> dispatch
  -> GUI / MQTT / business consumers
```

## 3. Layer Design

### 3.1 Hardware Platform Layer

这是跨平台的关键层，负责屏蔽不同芯片写法差异。

重点抽象能力：

- `i2c`
- `spi`
- `adc`
- `gpio`
- `pwm`
- `timer/tick`
- `delay`
- `lock`

推荐接口：

- `platform_i2c_ops`
- `platform_spi_ops`
- `platform_adc_ops`
- `platform_gpio_ops`
- `platform_pwm_ops`
- `platform_timer_ops`
- `platform_delay_ops`
- `platform_lock_ops`

规则：

- MCU SDK / BSP / HAL / register 访问都留在这里
- 上层不允许直接碰硬件细节

典型差异案例：

- 不同芯片 `i2c` 传输接口不同
- 不同芯片 `pwm` 配置方式不同
- 不同平台 `gpio` mux / polarity 不同
- 有的系统有 mutex，有的是 bare-metal no-op

### 3.2 Sensor Driver Layer

这一层只关心 `sensor 芯片逻辑`。

推荐接口：

- `sensor_driver_ops`

职责：

- init
- raw read
- raw value convert
- optional reset / power control

规则：

- 只能依赖 `platform_*_ops`
- 不能直接调用芯片 SDK
- 不耦合 GUI / MQTT / business

### 3.3 Sensor Core

负责统一 sample 生命周期。

推荐核心抽象：

- `sensor_sample_t`
- `sensor_manager`

职责：

- normalize sample
- timestamp / unit / status
- 调度 driver
- 调度 filter
- 调度 dispatch

### 3.4 Filter Layer

负责滤波，不参与平台和业务细节。

推荐接口：

- `sensor_filter_ops`

建议首批 filter：

- `bypass`
- `moving_average`
- `median`
- `ema_iir`

规则：

- filter 只处理 sample
- filter 状态独立保存
- filter 不关心 bus / GUI / MQTT

### 3.5 Dispatch/Event Layer

负责多消费者分发。

推荐接口：

- `sensor_event_listener`

职责：

- listener 注册
- sample 分发
- 一份 sample 给多个 consumer

### 3.6 Application Adapters

上层应用统一作为消费者存在：

- GUI adapter
- MQTT adapter
- business adapter

规则：

- 只能消费 sample
- 不能回头直接访问 driver / platform

## 4. Temperature Sensor Example

首个案例固定为温度传感器。

推荐流程：

```text
platform_i2c_ops / platform_gpio_ops / optional platform_pwm_ops
  -> temperature driver
  -> sensor_sample_t
  -> filter
  -> dispatch
  -> GUI adapter
  -> MQTT adapter
  -> business adapter
```

补充说明：

虽然温度 sensor 本身不一定直接用 `pwm`，但产品业务后面可能会和风扇、加热、背光、蜂鸣器等 PWM 外设联动，所以 platform 层必须提前设计好。

## 5. Public Interface Direction

先统一这些名字：

- `platform_i2c_ops`
- `platform_spi_ops`
- `platform_adc_ops`
- `platform_gpio_ops`
- `platform_pwm_ops`
- `platform_timer_ops`
- `platform_delay_ops`
- `platform_lock_ops`
- `sensor_driver_ops`
- `sensor_filter_ops`
- `sensor_sample_t`
- `sensor_event_listener`
- `sensor_manager`

其中：

- `sensor_sample_t`：至少包含 `value / unit / timestamp / status`
- `sensor_manager`：负责 read -> filter -> dispatch 主流程

## 6. Reference Projects Mapping

### MQTT 参考

路径：

`/home2/sean/BlueAirProject/SampleProject/XQD-GYJ-CAT1-APP`

借鉴点：

- 通信层 / 协议层 / 应用层拆分

在本框架中：

- MQTT 是 adapter
- cloud topic / JSON / AT 指令不属于 core

### UI porting 参考

路径：

`/home/sean/gitlab_lvgl_template/lvgl_d21x/common/porting`

借鉴点：

- 不同平台拆 `porting`
- lock / time / display / input 分边界

在本框架中：

- 把同样思路扩展到 sensor hardware 抽象

### UI / code architecture 参考

路径：

`/home/sean/gitlab_lvgl_template/app_d12x_d21x_ssd202_mono【兼容4者】`

借鉴点：

- `pages / view / model / biz / data / uikit`

在本框架中：

- GUI / MQTT / business 都是 sample consumer

## 7. Recommended Directory

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

## 8. C to C++ Mapping

为了后面可能切 C++，这里提前定义映射关系：

- `platform_*_ops` -> platform interface class
- `sensor_driver_ops` -> abstract driver
- `sensor_filter_ops` -> strategy
- `sensor_event_listener` -> observer
- `sensor_manager` -> controller / orchestrator

原则：

- 迁移时升级实现方式
- 不重做分层结构

## 9. Recommended Implementation Order

建议实现顺序：

1. 先定义 `platform_*_ops`
2. 再定义 `sensor_driver_ops`
3. 再定义 `sensor_sample_t`
4. 再定义 `sensor_manager`
5. 再做 filter
6. 再做 dispatch
7. 再补 temperature example
8. 最后接 GUI / MQTT / business adapters

## 10. Rules

最重要的规则只有这几条：

- 所有硬件差异必须沉到 platform 层
- sensor driver 不能直接写芯片 SDK
- filter 不混入业务逻辑
- application adapters 只能消费 sample
- 上层不要到处扩散芯片 `#ifdef`
