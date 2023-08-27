TRIPLET=arm-none-eabi-
CC=$(TRIPLET)gcc
CXX=$(TRIPLET)g++
LD=$(TRIPLET)ld
OBJCOPY=$(TRIPLET)objcopy
GDB=$(TRIPLET)gdb
SIZE=$(TRIPLET)size

# Detect whether shell style is windows or not
# https://stackoverflow.com/questions/714100/os-detecting-makefile/52062069#52062069
ifeq '$(findstring ;,$(PATH))' ';'
CP=copy
RM=del
else
CP=cp
endif

vpath %.c ../src
vpath %.c ../src/ae_lpc11u35_mb
vpath %.c ../lib/CMSIS_5/CMSIS/DAP/Firmware/Source
vpath %.c ../lib/nxp_driver/lpcopen/lpc11uxx/lpc_chip_11uxx/src
vpath %.c ../lib/nxp_driver/lpcopen/lpc11uxx/gcc
vpath %.c ../lib/tinyusb/src
vpath %.c ../lib/tinyusb/src/common
vpath %.c ../lib/tinyusb/src/device
vpath %.c ../lib/tinyusb/src/host
vpath %.c ../lib/tinyusb/src/class/audio
vpath %.c ../lib/tinyusb/src/class/bth
vpath %.c ../lib/tinyusb/src/class/cdc
vpath %.c ../lib/tinyusb/src/class/dfu
vpath %.c ../lib/tinyusb/src/class/hid
vpath %.c ../lib/tinyusb/src/class/midi
vpath %.c ../lib/tinyusb/src/class/msc
vpath %.c ../lib/tinyusb/src/class/net
vpath %.c ../lib/tinyusb/src/class/usbtmc
#vpath %.c ../lib/tinyusb/src/class/vendor
vpath %.c ../lib/tinyusb/src/class/video
vpath %.c ../lib/tinyusb/src/portable/nxp/lpc_ip3511

OBJS=\
 dcd_lpc_ip3511.o\
 cr_startup_lpc11xx.o\
 chip_11xx.o\
 clock_11xx.o\
 gpio_11xx_1.o\
 iocon_11xx.o\
 uart_11xx.o\
 sysctl_11xx.o\
 sysinit_11xx.o\
 main.o\
 usb_descriptors.o\
 board.o\
 DAP.o\
 JTAG_DP.o\
 SW_DP.o\
 tusb.o\
 tusb_fifo.o\
 usbd.o\
 usbd_control.o\
 audio_device.o\
 cdc_device.o\
 dfu_device.o\
 dfu_rt_device.o\
 hid_device.o\
 midi_device.o\
 msc_device.o\
 ecm_rndis_device.o\
 ncm_device.o\
 usbtmc_device.o\
 video_device.o\
 cmsis_dap_device.o

TARGET_ARCH=\
 -mthumb\
 -mabi=aapcs\
 -mcpu=cortex-m0

CPPFLAGS=\
 -DCORE_M0\
 -D__USE_LPCOPEN\
 -DCFG_EXAMPLE_MSC_READONLY\
 -DCFG_TUSB_MCU=OPT_MCU_LPC11UXX\
 -DCFG_TUSB_MEM_SECTION='__attribute__((section(".data.$$RAM2")))'\
 -DCFG_TUSB_MEM_ALIGN='__attribute__((aligned(64)))' \
 -I../src\
 -I../src/ae_lpc11u35_mb\
 -I../lib/CMSIS_5/CMSIS/Core/Include\
 -I../lib/CMSIS_5/CMSIS/DAP/Firmware/Include\
 -I../lib/nxp_driver/lpcopen/lpc11uxx/lpc_chip_11uxx/inc\
 -I../lib/tinyusb/src

CFLAGS=\
 -MMD\
 -MP\
 -Os\
 -flto\
 -nostdlib\
 -ggdb\
 -fstack-usage\
 -fdata-sections\
 -ffunction-sections\
 -fsingle-precision-constant\
 -fno-strict-aliasing\
 -Wall\
 -Wextra\
 -Werror\
 -Wfatal-errors\
 -Wdouble-promotion\
 -Wstrict-prototypes\
 -Wstrict-overflow\
 -Werror-implicit-function-declaration\
 -Wfloat-equal\
 -Wundef\
 -Wshadow\
 -Wwrite-strings\
 -Wsign-compare\
 -Wmissing-format-attribute\
 -Wunreachable-code\
 -Wcast-align\
 -Wcast-function-type\
 -Wcast-qual\
 -Wnull-dereference\
 -Wuninitialized\
 -Wunused\
 -Wredundant-decls\
 -Wno-null-dereference\
 -Wno-error=unused-variable\
 -Wno-error=strict-prototypes\
 -Wno-error=unused-parameter\
 -Wno-error=redundant-decls\
 -Wno-error=cast-qual

LDLIBS=\
 -lgcc\
 -lm\
 -lnosys

LDFLAGS=\
 -Wl,-T,../src/ae_lpc11u35_mb/lpc11u35.ld\
 -Wl,-Map=$(@:%.elf=%.map)\
 -Wl,-cref\
 -Wl,-gc-sections\
 -Wl,--print-memory-usage \
 -specs=nosys.specs\
 -specs=nano.specs


.PHONY:clean
all: akiprobe_crc.bin


akiprobe.elf:$(OBJS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.elf:%.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.bin:%.elf
	$(OBJCOPY) -O binary $^ $@

%_crc.bin:%.bin
	$(CP) $< $@
	lpc_checksum $@

clean:
	$(RM) $(OBJS:%.o=%.d) $(OBJS)

-include $(OBJS:%.o=%.d)
