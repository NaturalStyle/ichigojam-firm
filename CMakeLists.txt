cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(IchigoJam_pico)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

set(DVI_DEFAULT_SERIAL_CONFIG "pico_sock_cfg" CACHE STRING
	"Select a default pin configuration from common_dvi_pin_configs.h" FORCE)
include_directories(
	PicoDVI/software/include
	)
add_subdirectory(PicoDVI/software/libdvi)
add_subdirectory(PicoDVI/software/libsprite)

add_executable(IchigoJam_pico
	main.c
)

pico_set_linker_script(IchigoJam_pico ${CMAKE_CURRENT_LIST_DIR}/memmap_ichigojam_pico.ld)

# Make sure TinyUSB can find tusb_config.h
# main.cと同じ階層にあるtusb_config.hをinclude
target_include_directories(IchigoJam_pico PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# PicoDVI/software/apps/mandelbrot/CMakeLists.txt から引用
target_compile_definitions(IchigoJam_pico PRIVATE
	DVI_DEFAULT_SERIAL_CONFIG=${DVI_DEFAULT_SERIAL_CONFIG}
)

target_link_libraries(IchigoJam_pico PUBLIC
	pico_stdlib
	pico_multicore
	pico_util
	libdvi
	libsprite
	tinyusb_host
	tinyusb_board
	hardware_flash
	hardware_watchdog
	hardware_adc
	hardware_pwm
	hardware_i2c
	hardware_sleep
)

# pico_enable_stdio_usb(IchigoJam_pico 0)
pico_enable_stdio_uart(IchigoJam_pico 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(IchigoJam_pico)