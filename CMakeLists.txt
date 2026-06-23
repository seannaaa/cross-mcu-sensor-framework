cmake_minimum_required(VERSION 3.16)
project(cross_mcu_sensor_framework C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(MCU_PLATFORM "native" CACHE STRING "Target MCU/SoC platform name")
message(STATUS "MCU_PLATFORM=${MCU_PLATFORM}")

enable_testing()

set(PLATFORM_SOURCE "")

if(MCU_PLATFORM STREQUAL "esp32")
    set(PLATFORM_SOURCE platform/esp32/platform_impl.c)
elseif(MCU_PLATFORM STREQUAL "stm32")
    set(PLATFORM_SOURCE platform/stm32/platform_impl.c)
else()
    message(FATAL_ERROR "Unsupported MCU_PLATFORM: ${MCU_PLATFORM}")
endif()

add_executable(sensor_framework_tests
    biz/business_app.c
    biz/gui_app.c
    biz/mqtt_app.c
    biz/sensor_filter.c
    drivers/temperature_driver.c
    model/sensor_manager.c
    ${PLATFORM_SOURCE}
    test/test_sensor_framework.c
)

target_include_directories(sensor_framework_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/model
    ${CMAKE_CURRENT_SOURCE_DIR}/biz
)

target_compile_definitions(sensor_framework_tests PRIVATE
    MCU_PLATFORM_NAME="${MCU_PLATFORM}"
)

add_test(NAME sensor_framework_tests COMMAND sensor_framework_tests)
