# akiprobe

A CMSIS-DAP V2 implementation for
[AE-LPC11U35-MB](https://akizukidenshi.com/catalog/g/gK-12144/) and
[Raspberry Pi Pico(rp2040)](https://www.raspberrypi.com/products/raspberry-pi-pico/).

# Alternatives

- [DAPLink](https://github.com/ARMmbed/DAPLink)
- [Dapper Mime](https://github.com/majbthrd/DapperMime)

# Depends

- [TinyUSB](https://github.com/hathach/tinyusb)
- [CMSIS_6](https://github.com/ARM-software/CMSIS_6)
- [CMSIS-DAP](https://github.com/ARM-software/CMSIS-DAP)

# for [AE-LPC11U35-MB](https://akizukidenshi.com/catalog/g/gK-12144/)

## Requirements

- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- [GNU make](https://www.gnu.org/software/make/)
- [lpc_chekcusm](https://pypi.org/project/lpc-checksum/)

## Build

```console
$ git submodule update --init
$ make -C build -j
```

`build/akiprobe_crc.bin` will be generated.

## Pin assignment

| Function     | Pin | Port   |
|:------------:|:---:|:------:|
| SWCLK/TCK    | 27  | P0_21  |
| SWDIO/TMS    |  4  | P0_8   |
| TDI          |  9  | P0_12  |
| TDO          | 10  | P0_13  |
| nRESET       | 23  | P0_2   |
| Connected LED|  3  | P0_7   |
| UART TX      | 18  | P0_19  |
| UART RX      | 17  | P0_18  |

# for [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/)

## Requirements

- [pico-sdk](https://github.com/raspberrypi/pico-sdk)
- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- [cmake](https://cmake.org/)
- [ninja](https://ninja-build.org/)

## Build

```console
$ cmake -S . -B build_pico -G Ninja -DPICO_PLATFORM=rp2040 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=MinSizeRel -DPICO_SDK_PATH=../pico-sdk
$ cmake --build build_pico
```

`build_pico/akiprobe.uf2` will be generated.

## pin assignment

| Function     | Pin | Port   |
|:------------:|:---:|:------:|
| SWCLK/TCK    |  4  | GP2    |
| SWDIO/TMS    |  5  | GP3    |
| TDI          |  1  | GP0    |
| TDO/SWO      |  2  | GP1    |
| nRESET       |  9  | GP6    |
| UART TX      |  6  | GP4    |
| UART RX      |  7  | GP5    |

# Licensing

- The majority of this project is licensed under the **MIT License**.
- The following file is licensed under the **Apache License 2.0**:
  - `src/cmsis-dap/SWO.c`
- [TinyUSB](https://github.com/hathach/tinyusb) is licensed under the MIT license.
- [CMSIS_6](https://github.com/ARM-software/CMSIS_6) is licensed under the Apache 2.0 license.
- [CMSIS-DAP](https://github.com/ARM-software/CMSIS-DAP) is licensed under the Apache 2.0 license.
