# Platform Abstraction Refactor Design

## Background

The current project exposes platform-specific types and factory functions directly to upper layers. `build.sh` passes `MCU_PLATFORM` into CMake, but the build still compiles both `platform/esp32_platform.c` and `platform/stm32_platform.c`, so the selected platform does not actually control platform source selection. The test/demo entry in `test/test_sensor_framework.c` also hard-codes `esp32_platform_t` and `esp32_platform_create_bundle(...)`, which means application code is still platform-aware.

This refactor makes platform selection a build-time concern and removes platform-specific code from the application layer.

## Goal

Restructure the `platform` module so that:

1. `platform/` is split into per-platform implementation directories.
2. The application layer depends only on a unified platform interface and unified config structure.
3. `./build.sh <platform>` and CMake compile only the selected platform implementation.
4. The test/demo path uses the same platform-neutral API and no longer hard-codes ESP32-specific types.

## Scope

In scope:

- Reorganize platform implementation files into `platform/esp32/` and `platform/stm32/`
- Add unified public headers for platform configuration and platform bundle creation
- Update CMake source discovery to select files by `MCU_PLATFORM`
- Update the test/demo entry to use the unified platform API
- Remove obsolete top-level platform-specific public entry points

Out of scope:

- Introducing real vendor SDK integration
- Expanding platform support beyond the current ESP32 and STM32 demo implementations
- Changing driver, filter, manager, GUI, MQTT, or business logic behavior beyond what is required for the platform abstraction change

## Proposed Directory Layout

```text
platform/
  platform.h
  platform_config.h
  platform_ops.h
  esp32/
    platform_impl.h
    platform_impl.c
  stm32/
    platform_impl.h
    platform_impl.c
```

### Responsibilities

- `platform/platform_ops.h`
  - Keeps the existing function pointer abstractions and `platform_bundle_t`
  - Remains platform-agnostic

- `platform/platform_config.h`
  - Defines the unified application-facing configuration structure
  - Defines the unified runtime storage structure used by platform implementations

- `platform/platform.h`
  - Defines the single public factory entry point:
    - `platform_bundle_t platform_create_bundle(platform_runtime_t *runtime, const platform_config_t *config);`
  - Optionally exposes a macro or helper for the current build platform name only if needed by application/test code

- `platform/esp32/platform_impl.c`
  - Implements `platform_create_bundle(...)` for ESP32 builds
  - Maintains ESP32-specific runtime state internally

- `platform/stm32/platform_impl.c`
  - Implements `platform_create_bundle(...)` for STM32 builds
  - Maintains STM32-specific runtime state internally

## Unified Public API

### `platform_config_t`

The unified config intentionally contains only fields required by the existing test/demo flow:

```c
typedef struct {
    int i2c_bus;
    int power_pin;
    int pwm_channel;
    uint32_t tick_ms;
    int16_t mock_raw_temperature;
} platform_config_t;
```

Design notes:

- `i2c_bus` is the generic replacement for the current `esp32.i2c_port`
- `tick_ms` is the generic replacement for `tick_ms_cache` / `systick_ms_cache`
- `mock_raw_temperature` remains public because the current tests and demo use it as controllable input
- Platform-specific fields that are not used by the current demo are not included yet to avoid premature complexity

### `platform_runtime_t`

The application layer should not know the concrete platform runtime type. It should only own a unified runtime object that platform implementations can populate.

The runtime object must satisfy two requirements:

1. It provides enough storage for either platform implementation.
2. It exposes no platform-specific members to upper layers.

Recommended design:

```c
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
```

Why a union-backed opaque runtime:

- Application code allocates one stable type regardless of platform
- Each platform implementation gets a strongly typed internal view
- The public header remains stable if the private fields evolve, as long as the storage remains large enough

Although the two demo platforms currently share the same logical fields, they should still be treated as different internal implementations. This keeps the directory split meaningful and leaves room for future divergence without reworking the public API.

### `platform_create_bundle(...)`

The single public factory function becomes:

```c
platform_bundle_t platform_create_bundle(platform_runtime_t *runtime,
                                         const platform_config_t *config);
```

Behavior:

- Copies config into the selected platform runtime representation
- Initializes runtime bookkeeping fields used by the mock platform operations
- Returns a fully wired `platform_bundle_t`
- Is implemented by exactly one platform source file per build

Upper layers will no longer call `esp32_platform_create_bundle(...)` or `stm32_platform_create_bundle(...)`.

## Build System Design

### `build.sh`

`build.sh` already forwards `MCU_PLATFORM` to CMake and creates a platform-specific build directory. That behavior should remain.

No behavioral change is required in the script unless additional validation is desired. The real fix belongs in CMake source selection.

### `CMakeLists.txt`

The build must stop compiling every platform implementation together. Instead:

1. Resolve `MCU_PLATFORM`
2. Map it to a platform source directory
3. Add only that directory's implementation file(s) to the target
4. Fail configuration if the platform is unsupported

Expected platform selection logic:

- `esp32` -> `platform/esp32/platform_impl.c`
- `stm32` -> `platform/stm32/platform_impl.c`

Required include directories:

- `platform/`
- common module include paths already used by the project

This ensures:

- The selected platform actually changes the compiled implementation
- Symbol collisions are avoided because only one `platform_create_bundle(...)` implementation is linked
- Future platform additions only require adding a new directory and a mapping entry

## Application/Test Refactor

`test/test_sensor_framework.c` should be updated to depend only on:

- `platform.h`
- `platform_config.h`

It should no longer include:

- `esp32_platform.h`
- `stm32_platform.h`

### Required code changes

Replace:

```c
esp32_platform_t platform = { ... };
platform_bundle_t bundle = esp32_platform_create_bundle(&platform);
```

With:

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

For the second sample in the full-pipeline test and the demo flow, the mutable raw input and observable platform side effects should be read and updated through the active runtime storage for the selected build. Because the current tests inspect mock side effects like last GPIO and PWM writes, the public API needs one of the following:

1. A small test-only accessor helper in `platform.h`, or
2. A documented direct test use of `platform_runtime_t` storage members

Recommended choice: expose a small, platform-neutral inspection API in `platform.h` for the demo/test build path.

Example shape:

```c
int platform_runtime_get_last_gpio_pin(const platform_runtime_t *runtime);
int platform_runtime_get_last_gpio_value(const platform_runtime_t *runtime);
int platform_runtime_get_last_pwm_channel(const platform_runtime_t *runtime);
uint32_t platform_runtime_get_last_pwm_frequency_hz(const platform_runtime_t *runtime);
uint32_t platform_runtime_get_last_delay_ms(const platform_runtime_t *runtime);
void platform_runtime_set_mock_raw_temperature(platform_runtime_t *runtime, int16_t raw_temperature);
```

Reasoning:

- Keeps application code platform-neutral
- Avoids leaking the runtime union internals into unrelated modules
- Preserves the ability to assert the mock side effects in tests

The demo output can still label the platform using `MCU_PLATFORM_NAME`.

## Error Handling

The refactor should preserve the current lightweight behavior:

- If `runtime == NULL` or `config == NULL`, `platform_create_bundle(...)` should return a zero-initialized or invalid bundle
- Existing low-level callbacks should continue returning `-1` for invalid context/buffer usage
- Unsupported `MCU_PLATFORM` should fail at CMake configure time, not at runtime

This keeps failures early and predictable.

## Testing Strategy

The main behavior to preserve is that the full data path still works regardless of selected platform.

Minimum verification:

1. Build and run tests for `esp32`
2. Build and run tests for `stm32`

What the tests must still prove:

- Filter behavior remains unchanged
- Sensor manager init still triggers GPIO/PWM/delay through platform ops
- Driver read still converts mock raw temperature into expected Celsius output
- GUI, MQTT, and business listeners still receive the expected final values
- Demo path prints final GUI/MQTT/business output through the platform-neutral entry point

## Migration Notes

The old public symbols become obsolete after this refactor:

- `esp32_platform_t`
- `stm32_platform_t`
- `esp32_platform_create_bundle(...)`
- `stm32_platform_create_bundle(...)`

Call sites should migrate fully to:

- `platform_config_t`
- `platform_runtime_t`
- `platform_create_bundle(...)`

The current driver and manager abstractions do not need interface changes because they already consume `platform_bundle_t`.

## Risks and Mitigations

### Risk: Test code still needs to mutate mock runtime state

Mitigation:

- Provide a small platform-neutral runtime inspection/mutation API rather than exposing platform-specific runtime structs

### Risk: Public runtime storage becomes too tied to demo-only fields

Mitigation:

- Keep the public config minimal
- Treat runtime internals as implementation detail behind helper accessors
- Revisit the runtime layout only when real SDK integration is added

### Risk: Platform directories diverge later and break the unified config

Mitigation:

- The unified config should remain limited to common fields needed by upper layers
- Truly platform-private setup should remain in each implementation directory and be added only when a real use case appears

## Acceptance Criteria

This refactor is complete when:

1. `platform/esp32/` and `platform/stm32/` exist and contain separate implementations.
2. `CMakeLists.txt` compiles only the selected platform implementation based on `MCU_PLATFORM`.
3. `test/test_sensor_framework.c` contains no platform-specific type names or factory calls.
4. The application/test layer uses `platform_config_t`, `platform_runtime_t`, and `platform_create_bundle(...)`.
5. `./build.sh esp32` builds and runs the test target successfully.
6. `./build.sh stm32` builds and runs the test target successfully.
