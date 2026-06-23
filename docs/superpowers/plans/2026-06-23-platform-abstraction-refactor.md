# Platform Abstraction Refactor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rebuild the platform layer so build-time platform selection controls which implementation is compiled, while upper-layer code uses one unified platform configuration and factory API.

**Architecture:** Keep `platform_ops.h` as the stable cross-platform function-pointer contract, add unified public headers for config/runtime/accessors, and move chip-specific behavior into `platform/esp32/` and `platform/stm32/`. Update CMake to compile exactly one platform implementation and migrate the test/demo path to the new platform-neutral API.

**Tech Stack:** C99, CMake, Bash, assert-based C test executable

---

### Task 1: Add Failing Tests for Platform-Neutral API

**Files:**
- Modify: `test/test_sensor_framework.c`
- Test: `build/<platform>/sensor_framework_tests`

- [ ] **Step 1: Write the failing test updates**

Replace the ESP32-specific include and setup in `test/test_sensor_framework.c` with the platform-neutral include list and configuration/runtime objects shown below.

```c
#include "platform.h"
```

Replace the full-pipeline setup block with:

```c
    platform_config_t config = {
        .i2c_bus = 0,
        .power_pin = 21,
        .pwm_channel = 2,
        .tick_ms = 1234U,
        .mock_raw_temperature = 2534,
    };
    platform_runtime_t runtime = {0};
    platform_bundle_t bundle = platform_create_bundle(&runtime, &config);
```

Replace the platform assertions with:

```c
    assert(platform_runtime_get_last_gpio_pin(&runtime) == 21);
    assert(platform_runtime_get_last_gpio_value(&runtime) == 1);
    assert(platform_runtime_get_last_pwm_channel(&runtime) == 2);
    assert(platform_runtime_get_last_pwm_frequency_hz(&runtime) == 1000U);
    assert(platform_runtime_get_last_delay_ms(&runtime) == 10U);
```

Replace the second sample mutation with:

```c
    platform_runtime_set_mock_raw_temperature(&runtime, 2734);
```

Replace the demo setup block with:

```c
    platform_config_t config = {
        .i2c_bus = 0,
        .power_pin = 21,
        .pwm_channel = 2,
        .tick_ms = 1000U,
        .mock_raw_temperature = 2866,
    };
    platform_runtime_t runtime = {0};
    platform_bundle_t bundle = platform_create_bundle(&runtime, &config);
```

- [ ] **Step 2: Run the test build to verify it fails**

Run:

```bash
./build.sh esp32
```

Expected: build fails because `platform.h`, `platform_runtime_t`, `platform_create_bundle(...)`, and runtime accessor APIs do not exist yet.

- [ ] **Step 3: Commit the failing test change**

```bash
git add test/test_sensor_framework.c
git commit -m "test: require platform-neutral api"
```

### Task 2: Add Unified Public Platform API

**Files:**
- Create: `platform/platform_config.h`
- Create: `platform/platform.h`
- Modify: `platform/platform_ops.h`
- Test: `build/<platform>/sensor_framework_tests`

- [ ] **Step 1: Write the failing public API declarations**

Create `platform/platform_config.h` with:

```c
#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <stdint.h>

typedef struct {
    int i2c_bus;
    int power_pin;
    int pwm_channel;
    uint32_t tick_ms;
    int16_t mock_raw_temperature;
} platform_config_t;

typedef union {
    struct {
        int i2c_bus;
        int power_pin;
        int pwm_channel;
        int last_gpio_pin;
        int last_gpio_value;
        int last_pwm_channel;
        uint32_t last_pwm_frequency_hz;
        float last_pwm_duty_cycle;
        uint32_t tick_ms;
        uint32_t last_delay_ms;
        int16_t mock_raw_temperature;
    } esp32;
    struct {
        int i2c_bus;
        int power_pin;
        int pwm_channel;
        int last_gpio_pin;
        int last_gpio_value;
        int last_pwm_channel;
        uint32_t last_pwm_frequency_hz;
        float last_pwm_duty_cycle;
        uint32_t tick_ms;
        uint32_t last_delay_ms;
        int16_t mock_raw_temperature;
    } stm32;
} platform_runtime_storage_t;

typedef struct {
    platform_runtime_storage_t storage;
} platform_runtime_t;

#endif
```

Create `platform/platform.h` with:

```c
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

#include "platform_config.h"
#include "platform_ops.h"

platform_bundle_t platform_create_bundle(platform_runtime_t *runtime,
                                         const platform_config_t *config);

int platform_runtime_get_last_gpio_pin(const platform_runtime_t *runtime);
int platform_runtime_get_last_gpio_value(const platform_runtime_t *runtime);
int platform_runtime_get_last_pwm_channel(const platform_runtime_t *runtime);
uint32_t platform_runtime_get_last_pwm_frequency_hz(const platform_runtime_t *runtime);
uint32_t platform_runtime_get_last_delay_ms(const platform_runtime_t *runtime);
void platform_runtime_set_mock_raw_temperature(platform_runtime_t *runtime, int16_t raw_temperature);

#endif
```

`platform/platform_ops.h` should remain functionally unchanged unless it needs an include cleanup after the new header split.

- [ ] **Step 2: Run build to verify link/implementation still fails**

Run:

```bash
./build.sh esp32
```

Expected: compile gets further, then fails because the new functions are declared but not implemented and the old test still cannot link.

- [ ] **Step 3: Commit the public header addition**

```bash
git add platform/platform_config.h platform/platform.h platform/platform_ops.h
git commit -m "feat: add unified platform public api"
```

### Task 3: Move ESP32 Implementation Under `platform/esp32/`

**Files:**
- Create: `platform/esp32/platform_impl.h`
- Create: `platform/esp32/platform_impl.c`
- Modify: `CMakeLists.txt`
- Test: `build/esp32/sensor_framework_tests`

- [ ] **Step 1: Write the failing ESP32 implementation files**

Create `platform/esp32/platform_impl.h` with:

```c
#ifndef PLATFORM_ESP32_IMPL_H
#define PLATFORM_ESP32_IMPL_H

#include "../platform.h"

#endif
```

Create `platform/esp32/platform_impl.c` by moving the current ESP32 mock logic behind the unified API:

```c
#include "platform_impl.h"

static int esp32_i2c_read(void *context,
                          uint8_t device_address,
                          uint8_t register_address,
                          uint8_t *buffer,
                          size_t buffer_size);
static int esp32_gpio_write(void *context, int pin, int value);
static int esp32_pwm_set(void *context, int channel, uint32_t frequency_hz, float duty_cycle);
static uint32_t esp32_get_tick_ms(void *context);
static void esp32_delay_ms(void *context, uint32_t delay_ms);

platform_bundle_t platform_create_bundle(platform_runtime_t *runtime,
                                         const platform_config_t *config);
int platform_runtime_get_last_gpio_pin(const platform_runtime_t *runtime);
int platform_runtime_get_last_gpio_value(const platform_runtime_t *runtime);
int platform_runtime_get_last_pwm_channel(const platform_runtime_t *runtime);
uint32_t platform_runtime_get_last_pwm_frequency_hz(const platform_runtime_t *runtime);
uint32_t platform_runtime_get_last_delay_ms(const platform_runtime_t *runtime);
void platform_runtime_set_mock_raw_temperature(platform_runtime_t *runtime, int16_t raw_temperature);
```

Update `CMakeLists.txt` so the target sources use `platform/esp32/platform_impl.c` instead of `platform/esp32_platform.c` when `MCU_PLATFORM=esp32`.

- [ ] **Step 2: Run build to verify the first implementation still fails correctly**

Run:

```bash
./build.sh esp32
```

Expected: build may still fail because STM32 selection logic and old source references have not been fully cleaned up yet, but it should now resolve the ESP32 symbols from the new path.

- [ ] **Step 3: Implement the minimal working ESP32 behavior**

Use the old `esp32_platform.c` logic, but write it against `platform_runtime_t` and the new unified config/accessor API. The implementation should:

- copy config values into `runtime->storage.esp32`
- use Chinese comments for non-obvious logic
- keep invalid argument behavior returning `-1`

Core runtime initialization:

```c
    runtime->storage.esp32.i2c_bus = config->i2c_bus;
    runtime->storage.esp32.power_pin = config->power_pin;
    runtime->storage.esp32.pwm_channel = config->pwm_channel;
    runtime->storage.esp32.tick_ms = config->tick_ms;
    runtime->storage.esp32.mock_raw_temperature = config->mock_raw_temperature;
```

- [ ] **Step 4: Run the ESP32 build to verify it passes**

Run:

```bash
./build.sh esp32
ctest --test-dir build/esp32 --output-on-failure
```

Expected: `sensor_framework_tests` passes under `esp32`.

- [ ] **Step 5: Commit the ESP32 platform migration**

```bash
git add CMakeLists.txt platform/esp32/platform_impl.h platform/esp32/platform_impl.c
git commit -m "refactor: move esp32 platform behind unified api"
```

### Task 4: Move STM32 Implementation Under `platform/stm32/`

**Files:**
- Create: `platform/stm32/platform_impl.h`
- Create: `platform/stm32/platform_impl.c`
- Modify: `CMakeLists.txt`
- Test: `build/stm32/sensor_framework_tests`

- [ ] **Step 1: Write the failing STM32 implementation files**

Create `platform/stm32/platform_impl.h` with:

```c
#ifndef PLATFORM_STM32_IMPL_H
#define PLATFORM_STM32_IMPL_H

#include "../platform.h"

#endif
```

Create `platform/stm32/platform_impl.c` by porting the current STM32 mock logic behind the same public API names:

```c
#include "platform_impl.h"
```

Update `CMakeLists.txt` so:

- `MCU_PLATFORM=stm32` selects `platform/stm32/platform_impl.c`
- unsupported platform values trigger `message(FATAL_ERROR ...)`

- [ ] **Step 2: Run build to verify STM32 path fails before full implementation**

Run:

```bash
./build.sh stm32
```

Expected: build fails or links incorrectly until the STM32 implementation and platform selection are fully wired.

- [ ] **Step 3: Implement the minimal working STM32 behavior**

Use the old `stm32_platform.c` logic, but write it against `platform_runtime_t` and the unified config/accessor API. The implementation should mirror ESP32 behavior where the demo framework expects identical observable results.

- [ ] **Step 4: Run the STM32 build to verify it passes**

Run:

```bash
./build.sh stm32
ctest --test-dir build/stm32 --output-on-failure
```

Expected: `sensor_framework_tests` passes under `stm32`.

- [ ] **Step 5: Commit the STM32 platform migration**

```bash
git add CMakeLists.txt platform/stm32/platform_impl.h platform/stm32/platform_impl.c
git commit -m "refactor: move stm32 platform behind unified api"
```

### Task 5: Remove Old Platform Entrypoints and Finalize Build Selection

**Files:**
- Delete: `platform/esp32_platform.h`
- Delete: `platform/esp32_platform.c`
- Delete: `platform/stm32_platform.h`
- Delete: `platform/stm32_platform.c`
- Modify: `CMakeLists.txt`
- Modify: `README.md`
- Test: `build/esp32/sensor_framework_tests`
- Test: `build/stm32/sensor_framework_tests`

- [ ] **Step 1: Remove obsolete source/header references**

Delete the four old top-level platform files and ensure `CMakeLists.txt` no longer references them. Update any documentation snippet in `README.md` that still tells users to include platform-specific headers from the application layer.

- [ ] **Step 2: Run both platform builds to verify nothing still depends on old files**

Run:

```bash
./build.sh esp32
ctest --test-dir build/esp32 --output-on-failure
./build.sh stm32
ctest --test-dir build/stm32 --output-on-failure
```

Expected: both builds configure, compile, and pass tests without the deleted files.

- [ ] **Step 3: Commit the cleanup**

```bash
git add CMakeLists.txt README.md platform
git commit -m "refactor: remove old platform-specific entrypoints"
```

### Task 6: Final Verification Pass

**Files:**
- Test: `build/esp32/sensor_framework_tests`
- Test: `build/stm32/sensor_framework_tests`

- [ ] **Step 1: Rebuild and run the full verification matrix**

Run:

```bash
./build.sh esp32
ctest --test-dir build/esp32 --output-on-failure
./build.sh stm32
ctest --test-dir build/stm32 --output-on-failure
```

Expected: both platform builds are green.

- [ ] **Step 2: Capture the demo output from each platform build**

Run:

```bash
./build/esp32/sensor_framework_tests
./build/stm32/sensor_framework_tests
```

Expected: each binary prints GUI/MQTT/business results through the platform-neutral path, with the platform label reflecting `MCU_PLATFORM_NAME`.

- [ ] **Step 3: Commit the final verified state**

```bash
git add .
git commit -m "feat: finalize platform-neutral platform selection"
```
