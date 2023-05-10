CROSS   = riscv32-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
ARCH    = $(CROSS)ar

BUILD_DIR       = $(abspath ../../Build)
RTOS_SOURCE_DIR = $(abspath ../../FreeRTOS-Kernel)
DEMO_SOURCE_DIR = $(abspath ../Common/Minimal)
VENDOR_SOURCE_DIR = $(abspath ../../Payloads)
DRIVERS_DIR = $(abspath ../../Drivers)
LOWLEVEL_DIR = $(abspath ../../Low_level)

MACHINEFLAGS = -march=rv32im -mabi=ilp32 -lm -lgcc

CPPFLAGS = \
	-I . \
	-I ../Common/include \
	-I $(RTOS_SOURCE_DIR)/include \
	-I $(VENDOR_SOURCE_DIR) \
	-I $(DRIVERS_DIR) \
	-I $(RTOS_SOURCE_DIR)/portable/GCC/RISC-V \
	-I $(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions \

CFLAGS = $(MACHINEFLAGS)\
	-Wall \
	-fmessage-length=0 \
	-ffunction-sections \
	-fdata-sections \
	-fno-builtin-printf

ASFLAGS = $(MACHINEFLAGS)
LDFLAGS = -nostartfiles -T$(LOWLEVEL_DIR)/riscv32_mod_pre.ld\
	-Xlinker --gc-sections \
	-Xlinker --defsym=__stack_size=800

ifeq ($(DEBUG), 1)
    CFLAGS += -g3 -ggdb3
else
    CFLAGS += -O2
endif
ifeq ($(QEMU), 1)
    CFLAGS += -D QEMU=1
endif

SRCS = main.c main_aes.c riscv-virt.c \
	$(DEMO_SOURCE_DIR)/EventGroupsDemo.c \
	$(DEMO_SOURCE_DIR)/TaskNotify.c \
	$(DEMO_SOURCE_DIR)/TimerDemo.c \
	$(DEMO_SOURCE_DIR)/blocktim.c \
	$(DEMO_SOURCE_DIR)/dynamic.c \
	$(DEMO_SOURCE_DIR)/recmutex.c \
	$(RTOS_SOURCE_DIR)/event_groups.c \
	$(RTOS_SOURCE_DIR)/list.c \
	$(RTOS_SOURCE_DIR)/queue.c \
	$(RTOS_SOURCE_DIR)/stream_buffer.c \
	$(RTOS_SOURCE_DIR)/tasks.c \
	$(RTOS_SOURCE_DIR)/timers.c \
	$(RTOS_SOURCE_DIR)/portable/MemMang/heap_4.c \
	$(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/port.c \
	$(VENDOR_SOURCE_DIR)/aes.c \
	$(VENDOR_SOURCE_DIR)/trng.c \
	$(DRIVERS_DIR)/uart.c \
	$(DRIVERS_DIR)/ns16550.c

ASMS = start.S vector.S \
	$(RTOS_SOURCE_DIR)/portable/GCC/RISC-V/portASM.S

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o) $(ASMS:%.S=$(BUILD_DIR)/%.o)
DEPS = $(SRCS:%.c=$(BUILD_DIR)/%.d) $(ASMS:%.S=$(BUILD_DIR)/%.d)

$(BUILD_DIR)/RTOSDemo.axf: $(OBJS) $(LOWLEVEL_DIR)/riscv32_mod_pre.ld Makefile
	rm $(LOWLEVEL_DIR)/riscv32_mod_pre.ld
	$(CC) -E -P -x c $(CPPFLAGS) $(CFLAGS) $(LOWLEVEL_DIR)/riscv32_mod.ld >> $(LOWLEVEL_DIR)/riscv32_mod_pre.ld
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.S Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(ASFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
