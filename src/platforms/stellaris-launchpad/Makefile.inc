CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

STELLARIS = /home/pf/opt/stellaris-launchpad
INCLUDES = -I../libopencm3/include

CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += $(INCLUDES) $(CPU_FLAGS) -DTARGET_IS_BLIZZARD_RA2 -DLM4F -DPART_LM4F120H5QR -DLPC

LINKER_SCRIPT="platforms/stellaris-launchpad/blackmagic.ld"
LDFLAGS = -nostartfiles -lc $(CPU_FLAGS) -nodefaultlibs -T$(LINKER_SCRIPT) -Wl,--gc-sections \
	-L../libopencm3/lib -lopencm3_lm4f -lnosys -lm -lgcc
	
VPATH += platforms/stellaris-launchpad

all: blackmagic.bin

blackmagic.bin: blackmagic
	$(OBJCOPY) -O binary $^ $@
