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

NAME := nsh

$(NAME)_SOURCES := nsh_main.c \
                   nsh_init.c \
                   nsh_consolemain.c \
                   nsh_console.c \
                   nsh_session.c \
                   nsh_parse.c \
                   nsh_command.c \
                   nsh_builtin.c \
                   nsh_routecmds.c \
                   nsh_proccmds.c \
                   nsh_fscmds.c \
                   nsh_envcmds.c \
                   nsh_script.c \
                   nsh_test.c \
                   nsh_ddcmd.c \
                   nsh_dbgcmds.c \
                   nsh_mntcmds.c \
                   nsh_mmcmds.c \
                   nsh_netcmds.c \
                   nsh_netinit.c \
                   nsh_telnetd.c \
                   nsh_dhcpd.c \
                   nsh_smartfs_init.c \
                   cle.c \
                   builtin.c \
                   exec_builtin.c \
                   builtin_list.c \
                   dns_gethostip.c \
                   dns_socket.c \
                   dns_resolver.c \
                   i2c_main.c \
                   i2c_bus.c \
                   i2c_dev.c \
                   i2c_get.c \
                   i2c_set.c \
                   i2c_verf.c \
                   i2c_common.c \
                   telnetd_daemon.c \
                   telnetd_driver.c \
                   readline_common.c \
                   readline.c \
                   std_readline.c \
                   nxplayer/nxplayer.c \
                   nxplayer/nxplayer_main.c \
                   ostest/aio.c \
                   ostest/cancel.c \
                   ostest/dev_null.c \
                   ostest/mqueue.c \
                   ostest/nsem.c \
                   ostest/ostest_main.c \
                   ostest/prioinherit.c \
                   ostest/rmutex.c \
                   ostest/sem.c \
                   ostest/sighand.c \
                   ostest/timedwait.c \
                   ostest/waitpid.c \
                   ostest/barrier.c \
                   ostest/cond.c \
                   ostest/fpu.c \
                   ostest/mutex.c \
                   ostest/posixtimer.c \
                   ostest/restart.c \
                   ostest/roundrobin.c \
                   ostest/semtimed.c \
                   ostest/timedmqueue.c \
                   ostest/vfork.c \
                   flash_eraseall/flash_eraseall.c

$(NAME)_COMPONENTS += utilities/command_console \
                      utilities/command_console/wifi \
                      utilities/command_console/platform \
                      nuttx/dhcpc \
                      nuttx/dhcpd \
                      nuttx/netlib \
                      test/iperf

$(NAME)_DEFINES   += CONSOLE_ENABLE_IPERF

#Large stack needed for printf in debug mode
NuttX_START_STACK := 4000

# Enable powersave features
GLOBAL_DEFINES    += PLATFORM_POWERSAVE_DEFAULT=1 PLATFORM_WLAN_POWERSAVE_STATS=1

# Define pool sizes
GLOBAL_DEFINES    += TX_PACKET_POOL_SIZE=16
GLOBAL_DEFINES    += RX_PACKET_POOL_SIZE=32

# Define path to local headers
$(NAME)_CFLAGS    += -I$(SOURCE_ROOT)/apps/test/nsh/

# No GMAC for now
PLATFORM_NO_GMAC  := 1

GLOBAL_DEFINES    += WICED_USE_AUDIO

VALID_OSNS_COMBOS := NuttX-NuttX_NS

VALID_PLATFORMS   := BCM943909* BCM943907* BCM943903*
INVALID_PLATFORMS := BCM943909QT

NUTTX_APP         := nsh
