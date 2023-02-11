# akiprobe

A CMSIS-DAP V2 implementation for
[AE-LPC11U35-MB](https://akizukidenshi.com/catalog/g/gK-12144/) and
[Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/).

# Alternatives

- [DAPLink](https://github.com/ARMmbed/DAPLink)
- [Dapper Mime](https://github.com/majbthrd/DapperMime)

# Depends

- [TinyUSB](https://github.com/hathach/tinyusb)
- [CMSIS](https://github.com/ARM-software/CMSIS_5)

# for [AE-LPC11U35-MB](https://akizukidenshi.com/catalog/g/gK-12144/)

## Requirements

- [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm)
- [GNU make](https://www.gnu.org/software/make/)
- [lpc_chekcusm](https://pypi.org/project/lpc-checksum/)

## Build

```console
$ git submodule update --init lib/CMSIS_5
$ git submodule update --init lib/tinyusb
$ git submodule update --init lib/nxp_driver
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
- [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm)
- [cmake](https://cmake.org/)
- [ninja](https://ninja-build.org/)

## Build

```console
$ cmake -S . -B build_pico -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=MinSizeRel -DPICO_SDK_PATH=../pico-sdk
$ cmake --build build_pico
```

`build_pico/akiprobe.uf2` will be generated.

## pin assignment

| Function     | Pin | Port   |
|:------------:|:---:|:------:|
| SWCLK/TCK    |  4  | GP2    |
| SWDIO/TMS    |  5  | GP3    |
| TDI          |  6  | GP4    |
| TDO          |  7  | GP5    |
| nRESET       |  9  | GP6    |
| UART TX      | 11  | GP8    |
| UART RX      | 12  | GP9    |

# License

- Project's source code files are licensed under MIT license.
- [TinyUSB](https://github.com/hathach/tinyusb) is licensed under the MIT license.
- [CMSIS](https://github.com/ARM-software/CMSIS_5) is licensed under the Apache 2.0 license.
