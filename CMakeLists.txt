cmake_minimum_required(VERSION 3.16)
project(cross_mcu_sensor_framework C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(MCU_PLATFORM "native" CACHE STRING "Target MCU/SoC platform name")
message(STATUS "MCU_PLATFORM=${MCU_PLATFORM}")

enable_testing()

add_executable(sensor_framework_tests
    core/sensor_manager.c
    drivers/temperature_sensor_driver.c
    filters/moving_average_filter.c
    tests/test_sensor_framework.c
)

target_include_directories(sensor_framework_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/include
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/filters/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dispatch/include
)

target_compile_definitions(sensor_framework_tests PRIVATE
    MCU_PLATFORM_NAME="${MCU_PLATFORM}"
)

add_test(NAME sensor_framework_tests COMMAND sensor_framework_tests)
