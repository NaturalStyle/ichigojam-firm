cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(blink_led
    blink_led.c
)

add_executable(blink_led_by_button
    blink_led_by_button.c
)

add_executable(i2c_test
    i2c_test.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(blink_led pico_stdlib)
target_link_libraries(blink_led_by_button pico_stdlib)
target_link_libraries(i2c_test 
    pico_stdlib
    hardware_i2c
)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(blink_led)
pico_add_extra_outputs(blink_led_by_button)
pico_add_extra_outputs(i2c_test)