#
# Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 # Cypress Semiconductor Corporation. All Rights Reserved.
 # This software, including source code, documentation and related
 # materials ("Software"), is owned by Cypress Semiconductor Corporation
 # or one of its subsidiaries ("Cypress") and is protected by and subject to
 # worldwide patent protection (United States and foreign),
 # United States copyright laws and international treaty provisions.
 # Therefore, you may use this Software only as provided in the license
 # agreement accompanying the software package from which you
 # obtained this Software ("EULA").
 # If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 # non-transferable license to copy, modify, and compile the Software
 # source code solely for use in connection with Cypress's
 # integrated circuit products. Any reproduction, modification, translation,
 # compilation, or representation of this Software except as specified
 # above is prohibited without the express written permission of Cypress.
 #
 # Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 # EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 # reserves the right to make changes to the Software without notice. Cypress
 # does not assume any liability arising out of the application or use of the
 # Software or any product or circuit described in the Software. Cypress does
 # not authorize its products for use in any products where a malfunction or
 # failure of the Cypress product may reasonably be expected to result in
 # significant property damage, injury or death ("High Risk Product"). By
 # including Cypress's product in a High Risk Product, the manufacturer
 # of such system or application assumes all risk of such use and in doing
 # so agrees to indemnify Cypress against all liability.
#

ifeq ($(NUTTX_APP),)
# Remove once Ethernet driver is implemented for NuttX
PLATFORM_NO_GMAC  := 1
NUTTX_APP         := default
endif
include $(CURDIR)$(NUTTX_DIR)/configs/$(NUTTX_BOARD)/$(NUTTX_APP)/defconfig

##### DIRECTORIES USED TO ADD SOURCE FILES #####################################################

NUTTX_SOURCE_FOLDERS := \
        sched/init \
        sched/irq \
        sched/paging \
        sched/group \
        sched/sched \
        sched/task \
        sched/errno \
        sched/wdog \
        sched/semaphore \
        sched/signal \
        sched/pthread \
        sched/mqueue \
        sched/clock \
        sched/timer \
        sched/environ \
        sched/wqueue \
        net/socket \
        net/netdev \
        net/iob \
        net/arp \
        net/icmp \
        net/icmpv6 \
        net/neighbor \
        net/igmp \
        net/pkt \
        net/rts \
        net/local \
        net/loopback \
        net/tcp \
        net/udp \
        net/devif \
        net/route \
        net/utils \
        fs/inode \
        fs/vfs \
        fs/driver \
        fs/dirent \
        fs/aio \
        fs/mmap \
        fs/semaphore \
        fs/mqueue \
        fs/shm \
        libc/stdio \
        libc/stdlib \
        libc/unistd \
        libc/sched \
        libc/syslog \
        libc/string \
        libc/aio \
        libc/pthread \
        libc/semaphore \
        libc/signal \
        libc/mqueue \
        libc/math \
        libc/fixedmath \
        libc/net \
        libc/time \
        libc/libgen \
        libc/dirent \
        libc/termios \
        libc/spawn \
        libc/queue \
        libc/wqueue \
        libc/misc \
        libc/audio \
        mm/mm_heap \
        mm/umm_heap \
        mm/kmm_heap \
        mm/mm_gran \
        mm/shm \
        binfmt/libbuiltin \
        drivers/analog \
        drivers/audio \
        drivers/bch \
        drivers/input \
        drivers/lcd \
        drivers/mmcsd \
        drivers/mtd \
        drivers/eeprom \
        drivers/net \
        drivers/pipes \
        drivers/power \
        drivers/sensors \
        drivers/sercomm \
        drivers/serial \
        drivers/spi \
        drivers/syslog \
        drivers/usbdev \
        drivers/usbhost \
        drivers/video \
        drivers/wireless \
        arch/arm/src/$(NUTTX_PLATFORM)

ifneq ($(CONFIG_NFILE_DESCRIPTORS),0)
ifneq ($(CONFIG_DISABLE_MOUNTPOINT),y)
NUTTX_SOURCE_FOLDERS += \
        fs/mount \
        fs/fat \
        fs/romfs \
        fs/nxffs \
        fs/nfs \
        fs/smartfs \
        fs/binfs  \
        fs/procfs
endif
endif

NUTTX_SEEK_SOURCE_FOLDERS := \
        $(NUTTX_SOURCE_FOLDERS) \
        arch/arm/src/armv7-r \
        arch/arm/src/common

##### DIRECTORIES USED TO SEEK HEADERS TO INCLUDE ##############################################

NUTTX_HEADER_FOLDERS := \
        ./ \
        sched/ \
        net/ \
        fs/ \
        libc/ \
        arch/arm/src/common/

##### STATICAL SET OF SOURCE FILES TO COMPILE ##################################################

$(NAME)_SOURCES := \
        $(NUTTX_DIR)/fs/fs_initialize.c \
        $(NUTTX_DIR)/net/net_initialize.c

ifneq ($(CONFIG_NFILE_DESCRIPTORS),0)
        $(NAME)_SOURCES += $(NUTTX_DIR)/drivers/dev_null.c $(NUTTX_DIR)/drivers/dev_zero.c
ifneq ($(CONFIG_DISABLE_MOUNTPOINT),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/drivers/ramdisk.c $(NUTTX_DIR)/drivers/loop.c
ifeq ($(CONFIG_DRVR_WRITEBUFFER),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/drivers/rwbuffer.c
else
ifeq ($(CONFIG_DRVR_READAHEAD),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/drivers/rwbuffer.c
endif
endif
endif

ifeq ($(CONFIG_CAN),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/drivers/can.c
endif

ifeq ($(CONFIG_PWM),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/drivers/pwm.c
endif

ifeq ($(CONFIG_WATCHDOG),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/drivers/watchdog.c
endif

ifeq ($(CONFIG_TIMER),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/drivers/timer.c
endif
endif

ifneq ($(filter y,$(CONFIG_BCM4390X_SPI1) $(CONFIG_BCM4390X_SPI2)),)
        $(NAME)_SOURCES += $(NUTTX_DIR)/arch/arm/src/bcm4390x/bcm4390x_spi_board.c
endif

ifeq ($(CONFIG_I2C),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/arch/arm/src/bcm4390x/bcm4390x_i2c_board.c
endif

ifeq ($(CONFIG_AUDIO),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/audio/audio.c
endif

ifeq ($(CONFIG_CPP_OPERATORS),y)
        $(NAME)_SOURCES += $(NUTTX_DIR)/libxx/libxx_cxa_atexit.cpp \
                           $(NUTTX_DIR)/libxx/libxx_delete.cpp \
                           $(NUTTX_DIR)/libxx/libxx_eabi_atexit.cpp \
                           $(NUTTX_DIR)/libxx/libxx_new.cpp
endif


##### DYNAMICAL SET OF SOURCES TO COMPILE ######################################################

ADD_NUTTX_SOURCES :=

# Create lists of source files
define ADD_NUTTX_FOLDER_FUNC
$(eval include $(CURDIR)$(NUTTX_DIR)/$(1)/Make.defs)
$(eval ADD_NUTTX_SOURCES+=$(CSRCS))
$(eval ADD_NUTTX_SOURCES+=$(BINFMT_CSRCS))
$(eval ADD_NUTTX_SOURCES+=$(SOCK_CSRCS))
$(eval ADD_NUTTX_SOURCES+=$(NET_CSRCS))
$(eval ADD_NUTTX_SOURCES+=$(NETDEV_CSRCS))
$(eval ADD_NUTTX_SOURCES+=$(CHIP_CSRCS))
$(eval ADD_NUTTX_SOURCES+=$(CHIP_ASRCS))
$(eval ADD_NUTTX_SOURCES+=$(CMN_CSRCS))
$(eval CSRCS:=)
$(eval BINFMT_CSRCS:=)
$(eval SOCK_CSRCS:=)
$(eval NET_CSRCS:=)
$(eval NETDEV_CSRCS:=)
$(eval CHIP_CSRCS:=)
$(eval CHIP_ASRCS:=)
$(eval CMN_CSRCS:=)
endef
$(foreach FOLDER,$(NUTTX_SOURCE_FOLDERS),$(eval $(call ADD_NUTTX_FOLDER_FUNC,$(FOLDER))))

# Find in which directory source file is, and prepend path
ADD_NUTTX_FILE_IF_EXIST_FUNC = $(if $(wildcard $(CURDIR)$(1)),$(eval $(NAME)_SOURCES+=$(1)),)
FIND_NUTTX_FILE_FUNC = $(foreach FOLDER,$(NUTTX_SEEK_SOURCE_FOLDERS),$(call ADD_NUTTX_FILE_IF_EXIST_FUNC,$(NUTTX_DIR)/$(FOLDER)/$(1)))
$(foreach FILE,$(ADD_NUTTX_SOURCES),$(call FIND_NUTTX_FILE_FUNC,$(FILE)))

# Add paths to header files
$(foreach FOLDER,$(NUTTX_HEADER_FOLDERS),$(eval $(NAME)_INCLUDES+=$(NUTTX_DIR)/$(FOLDER)))

##### EXTRA BUILD SETTINGS #####################################################################

ifeq ($(CONFIG_DEBUG_SIGNATURE_BLOCK),y)
ifeq ($(TOOLCHAIN_NAME),GCC)
GLOBAL_LDFLAGS += -Wl,--undefined=g_dsb
endif
endif

################################################################################################

#$(info $(ADD_NUTTX_SOURCES))
#$(info $($(NAME)_SOURCES))
