cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

set(DVI_DEFAULT_SERIAL_CONFIG "pico_sock_cfg" CACHE STRING
	"Select a default pin configuration from common_dvi_pin_configs.h" FORCE)
include_directories(
	PicoDVI/software/assets
	PicoDVI/software/include
	)
add_subdirectory(PicoDVI/software/libdvi)
add_subdirectory(PicoDVI/software/libsprite)

# rest of your project

# add_executable(hello_world
#     hello_world.c
# )

# # Add pico_stdlib library which aggregates commonly used features
# target_link_libraries(hello_world pico_stdlib)

# pico_enable_stdio_usb(hello_world 1)
# pico_enable_stdio_uart(hello_world 1)

# # create map/bin/hex/uf2 file in addition to ELF.
# pico_add_extra_outputs(hello_world)

# add_executable(colour_terminal
# 	colour_terminal.c
# 	tmds_encode_font_2bpp.S
# 	tmds_encode_font_2bpp.h
# )

# target_compile_definitions(colour_terminal PRIVATE
# 	DVI_VERTICAL_REPEAT=1
# 	)

# # We have a lot in SRAM4 (particularly TMDS LUT) but don't need much stack on
# # core 1. Probably even 256 bytes would be fine.
# target_compile_definitions(colour_terminal PRIVATE
# 	DVI_DEFAULT_SERIAL_CONFIG=${DVI_DEFAULT_SERIAL_CONFIG}
# 	PICO_CORE1_STACK_SIZE=0x200
# 	)

# target_link_libraries(colour_terminal
# 	pico_stdlib
# 	pico_multicore
# 	libdvi
# )

# # create map/bin/hex file etc.
# pico_add_extra_outputs(colour_terminal)

# # Replace TMDS with 10 bit UART (same baud rate):
# # add_definitions(-DDVI_SERIAL_DEBUG=1)
# # add_definitions(-DRUN_FROM_CRYSTAL)

# add_executable(sprite_bounce
# 	sprite_bounce.c
# )

# target_compile_options(sprite_bounce PRIVATE -Wall)

# target_compile_definitions(sprite_bounce PRIVATE
# 	DVI_DEFAULT_SERIAL_CONFIG=${DVI_DEFAULT_SERIAL_CONFIG}
# 	)

# target_link_libraries(sprite_bounce
# 	pico_stdlib
# 	pico_multicore
# 	pico_util
# 	libdvi
# 	libsprite
# )

# # create map/bin/hex file etc.
# pico_add_extra_outputs(sprite_bounce)



# Replace TMDS with 10 bit UART (same baud rate):
# add_definitions(-DDVI_SERIAL_DEBUG=1)
# add_definitions(-DRUN_FROM_CRYSTAL)

add_executable(IchigoJam_pico
	main.c
	# hid_app.c
)

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
)

# pico_enable_stdio_usb(IchigoJam_pico 0)
pico_enable_stdio_uart(IchigoJam_pico 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(IchigoJam_pico)