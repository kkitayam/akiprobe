# akiprobe

A CMSIS-DAP V2 implementation for [AE-LPC11U35-MB](https://akizukidenshi.com/catalog/g/gK-12144/).

# Alternatives

- [DAPLink](https://github.com/ARMmbed/DAPLink)
- [Dapper Mime](https://github.com/majbthrd/DapperMime)

# Requirements

- [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm)
- [GNU make](https://www.gnu.org/software/make/)
- [lpc_chekcusm](https://pypi.org/project/lpc-checksum/)

# Depends

- [TinyUSB](https://github.com/hathach/tinyusb)
- [CMSIS](https://github.com/ARM-software/CMSIS_5)

# Build

```console
$ git submodule update --init lib/CMSIS_5
$ git submodule update --init lib/tinyusb
$ git submodule update --init lib/nxp_driver
$ make -C build -j
```

`build/akiprobe_crc.bin` will be generated.

# Pin assignment

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

# License

- Procject's source code files are licensed under MIT license.
- [TinyUSB](https://github.com/hathach/tinyusb) is licensed under the MIT license.
- [CMSIS](https://github.com/ARM-software/CMSIS_5) is licensed under the Apache 2.0 license.
