# IchigoJam Firmware Copilot Instructions

## Architecture Overview
This codebase implements IchigoJam BASIC firmware across multiple platforms. The core BASIC interpreter resides in `IchigoJam_BASIC/`, containing platform-independent logic for parsing, executing, and managing BASIC programs. Each platform-specific folder (e.g., `IchigoJam_P/`, `console/`, `SPRESENSE/`) includes hardware-specific implementations that include the core BASIC code.

Key components:
- **IchigoJam_BASIC/**: Core interpreter with modules like `basic.h` (execution engine), `ram.h` (memory layout), `vars.h` (variable handling), `screen.h` (display), `psg.h` (sound), and `tokens_v1.4.h` (BASIC syntax tokens).
- **Platform folders**: Contain `config.h` for platform-specific defines (e.g., `PLATFORM_RP2040`, `VERSION15`), and hardware interfaces like `system.h`, `display.h`, `keyboard.h`.
- **Memory model**: Fixed virtual addresses (#000-#FFF) with dedicated regions: #700 PCG, #800 VAR, #900 VRAM, #C00 LIST. See `ram.h` for exact offsets and sizes.

Data flows from user input (keyboard) through the BASIC parser, executing commands that interact with hardware via platform-specific functions.

## Critical Workflows
- **Building for Raspberry Pi Pico (IchigoJam_P/)**: Requires pico-sdk (commit 196662b), pico-extras (09c64d5), PicoDVI (579eecc). Set `PICO_SDK_PATH` and `PICO_EXTRAS_PATH`. Run `mkdir build; cd build; cmake ..; make` to generate `IchigoJam_P.uf2`.
- **Building for console**: Simple `make` in `console/` directory. Supports `make TOOLCHAIN=clang`. Run `./a.out` to execute.
- **Building for SPRESENSE**: Use Arduino SPRESENSE environment. Run `sh build.sh` in `build-SPRESENSE/`, then `sh write.sh` to flash.
- **GivetakeJam_P variant**: Extends program size to 4096 bytes, reduces storage slots to 25. Uses same build process as IchigoJam_P but with modified `config.h` (`N_FLASH_STORAGE 25`).
- **Debugging**: No integrated debugger; rely on serial output via `uart()` function. Use `PRINT` statements for debugging BASIC programs.

## Project-Specific Conventions
- **Versioning**: Define `VERSION15` in `config.h` for 1.5 features like `DAC()`. Otherwise defaults to 1.4.3.
- **Extensions**: Enable via defines like `EXT_IOT` (IoT commands), `ENABLE_I2C_BPS` (I2C baud rate control).
- **USR functions**: Processor-specific in `usr/` subfolders (e.g., `usr/armcortexm0/usr.h` for ARM Cortex-M0). Implement custom machine code routines.
- **Error handling**: Uses global `_g.err` flag. Functions check `if (_g.err) return;` (see `IJB_ERR_CHK` macros in `basic.h`).
- **Memory access**: Direct via `PEEK()`/`POKE()` to virtual addresses. Avoid modifying `ram.h` sizes without updating linker scripts.
- **BASIC syntax**: Token-based parsing (see `tokens_v1.4.h`). Arrays start at [0], variables A-Z. Extended arrays in GivetakeJam map to #C00.

## Integration Points
- **External libraries**: Pico platforms depend on pico-sdk for GPIO/I2C. SPRESENSE uses Arduino framework.
- **Hardware interfaces**: I2C for EEPROM/storage, UART for serial, PWM for sound/PSG.
- **Cross-platform communication**: Core BASIC code calls platform functions like `IJB_out()` (GPIO output), `IJB_in()` (ADC input), `screen_locate()` (cursor positioning).

Reference `IchigoJam_BASIC/main.c` for include order: platform-independent first, then platform-specific.</content>
<parameter name="filePath">c:\Users\j_rox\Documents\GitHub\ichigojam-firm\.github\copilot-instructions.md