---
name: cross-mcu-sensor-framework
description: Use when designing or implementing a portable sensor framework across multiple MCU or SoC platforms, with hardware abstraction, pluggable filters, and decoupled GUI, MQTT, and business consumers.
---

# Cross MCU Sensor Framework

## Overview

这个 skill 不是 prompt 模板，而是这个项目的 `设计规范`.

目标：

- 用 `C` 实现核心接口
- 用 `C++ abstraction thinking` 组织架构
- 支持跨平台 hardware 抽象
- 支持 filter 可替换
- 支持 GUI / MQTT / business 解耦

## Required Layers

必须按下面分层思考：

1. `Hardware Platform Layer`
2. `Sensor Driver Layer`
3. `Sensor Core`
4. `Filter Layer`
5. `Dispatch/Event Layer`
6. `Application Adapters`

标准数据流：

```text
platform ops
  -> sensor driver read
  -> normalize sample
  -> filter
  -> dispatch
  -> GUI / MQTT / business consumers
```

## 1. Hardware Platform Layer

这一层负责屏蔽不同平台差异：

- `i2c`
- `spi`
- `adc`
- `gpio`
- `pwm`
- `timer`
- `delay`
- `lock`

建议接口：

- `platform_i2c_ops`
- `platform_spi_ops`
- `platform_adc_ops`
- `platform_gpio_ops`
- `platform_pwm_ops`
- `platform_timer_ops`
- `platform_delay_ops`
- `platform_lock_ops`

规则：

- 芯片 SDK / HAL / BSP 调用只能在这一层
- 上层不能直接访问寄存器
- 不同芯片的 `pwm/gpio/i2c/timer` 差异必须沉到这里

## 2. Sensor Driver Layer

负责具体 sensor 芯片行为。

建议接口：

- `sensor_driver_ops`

职责：

- init
- read raw data
- convert raw value
- optional reset / power control

规则：

- 只能依赖 `platform_*_ops`
- 不能耦合 GUI / MQTT / business
- 不能把 filter 写进 driver

## 3. Sensor Core

负责统一 sample 管理与调度。

建议核心抽象：

- `sensor_sample_t`
- `sensor_manager`

职责：

- sample normalize
- timestamp / unit / status
- 调用 driver
- 串联 filter 和 dispatch

## 4. Filter Layer

建议接口：

- `sensor_filter_ops`

首批 filter：

- `bypass`
- `moving_average`
- `median`
- `ema_iir`

规则：

- filter 只处理 sample
- filter 不知道平台细节
- filter 不耦合 GUI / MQTT / business

## 5. Dispatch/Event Layer

建议接口：

- `sensor_event_listener`

职责：

- 注册 listener
- 分发 sample
- 支持多个 consumer

## 6. Application Adapters

只允许作为 sample 消费者存在：

- GUI adapter
- MQTT adapter
- business adapter

规则：

- 不允许直接访问 sensor driver
- 不允许直接访问 platform ops

## Temperature Example

首个完整案例固定为 `temperature sensor`.

标准流程：

```text
platform ops
  -> temperature sensor driver
  -> sensor_sample_t
  -> filter
  -> dispatch
  -> GUI / MQTT / business
```

## Reference Projects

### MQTT 参考

`/home2/sean/BlueAirProject/SampleProject/XQD-GYJ-CAT1-APP`

借鉴点：

- 通信层 / 协议层 / 应用层分离

### UI porting 参考

`/home/sean/gitlab_lvgl_template/lvgl_d21x/common/porting`

借鉴点：

- 平台差异通过 `porting` 目录收口

### UI / code architecture 参考

`/home/sean/gitlab_lvgl_template/app_d12x_d21x_ssd202_mono【兼容4者】`

借鉴点：

- `pages / view / model / biz / data` 分层

## C -> C++ Mapping

后面如果切 C++，推荐映射如下：

- `platform_*_ops` -> interface class
- `sensor_driver_ops` -> abstract driver
- `sensor_filter_ops` -> strategy
- `sensor_event_listener` -> observer
- `sensor_manager` -> controller

## Implementation Order

实现顺序建议固定：

1. 先定义 `platform_*_ops`
2. 再定义 `sensor_driver_ops`
3. 再定义 `sensor_sample_t` / `sensor_manager`
4. 再实现 filter
5. 再实现 dispatch
6. 再补 temperature example
7. 最后接 GUI / MQTT / business adapters

## Non-Negotiable Rules

- 所有平台差异都要沉到底层
- sensor driver 不直接写芯片 SDK
- 上层不允许出现芯片分支逻辑蔓延
- 所有 consumer 只消费 sample
- 不为未来 C++ 迁移制造结构阻碍
