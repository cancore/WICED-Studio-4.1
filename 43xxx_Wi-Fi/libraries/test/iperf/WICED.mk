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

# This contains a list of the core iperf files and required includes/defines.
#
# Make sure no variables defined in this file get overwritten in another
# makefile. (eg. with `GLOBAL_DEFINES := ...').
#
# Make sure you define the IPERF_DIR variable to instruct the compiler as to 
# where to find iperf before including this file.

NAME          := App_iperf
#IPERF_DIR    := .
IPERF_INCLUDE := $(IPERF_DIR)/include
IPERF_SRC     := $(IPERF_DIR)/src
IPERF_COMPAT  := $(IPERF_DIR)/compat
IPERF_WICED   := $(IPERF_DIR)/WICED


#==============================================================================
# Configuration
#==============================================================================
WICED_THREADS          := 0
IPERF_DEBUG            := 0
IPERF_THREAD_STACKSIZE := 512
IPERF_THREAD_PRIORITY  := 1

#==============================================================================
# iperf includes
#==============================================================================
$(NAME)_INCLUDES += $(IPERF_INCLUDE) \
                    $(IPERF_WICED)


#==============================================================================
# iperf source files
#==============================================================================
$(NAME)_SOURCES += $(IPERF_SRC)/Client.cpp \
				   $(IPERF_SRC)/Extractor.c \
                   $(IPERF_SRC)/Launch.cpp \
				   $(IPERF_SRC)/List.cpp \
                   $(IPERF_SRC)/Listener.cpp \
                   $(IPERF_SRC)/Locale.c \
                   $(IPERF_SRC)/PerfSocket.cpp \
                   $(IPERF_SRC)/ReportCSV.c \
                   $(IPERF_SRC)/ReportDefault.c \
                   $(IPERF_SRC)/Reporter.c \
                   $(IPERF_SRC)/Server.cpp \
				   $(IPERF_SRC)/service.c \
                   $(IPERF_SRC)/Settings.cpp \
				   $(IPERF_SRC)/SocketAddr.c \
				   $(IPERF_SRC)/sockets.c \
				   $(IPERF_SRC)/stdio.c \
				   $(IPERF_SRC)/tcp_window_size.c \
				   $(IPERF_SRC)/debug.c


#==============================================================================
# iperf compatability files
#==============================================================================
$(NAME)_SOURCES += $(IPERF_COMPAT)/delay.cpp \
                   $(IPERF_COMPAT)/error.c \
				   $(IPERF_COMPAT)/gettimeofday.c \
				   $(IPERF_COMPAT)/inet_ntop.c \
				   $(IPERF_COMPAT)/inet_pton.c \
				   $(IPERF_COMPAT)/setitimer.c \
				   $(IPERF_COMPAT)/signal.c \
				   $(IPERF_COMPAT)/snprintf.c \
				   $(IPERF_COMPAT)/string.c \
				   $(IPERF_COMPAT)/Thread.c
                   
                   
#==============================================================================
# iperf WICED-compatability files
#==============================================================================
$(NAME)_SOURCES += $(IPERF_WICED)/Condition_$(RTOS).c \
                   $(IPERF_WICED)/gettimeofday.c \
                   $(IPERF_WICED)/Mutex_$(RTOS).c \
                   $(IPERF_WICED)/Thread_$(RTOS).c \
                   $(IPERF_WICED)/usleep.c \
				   $(IPERF_WICED)/wiced_cpp.cpp

#==============================================================================
# iperf main
#==============================================================================
$(NAME)_SOURCES += $(IPERF_SRC)/main.cpp


#==============================================================================
# Local defines
#==============================================================================
$(NAME)_DEFINES += WICED
$(NAME)_DEFINES += IPERF_THREAD_STACKSIZE=$(IPERF_THREAD_STACKSIZE)
$(NAME)_DEFINES += IPERF_THREAD_PRIORITY=$(IPERF_THREAD_PRIORITY)

# Turn iperf threading on/off
ifeq (0, $(WICED_THREADS))
$(NAME)_DEFINES += NO_THREADS
else
ifeq (FreeRTOS, $(RTOS))
# Needed for mutexes in FreeRTOS
GLOBAL_DEFINES += configUSE_MUTEXES=1
endif
endif

ifeq ($(NETWORK),NetX)
GLOBAL_DEFINES += NX_EXTENDED_BSD_SOCKET_SUPPORT
endif

ifeq ($(NETWORK),NetX_Duo)
GLOBAL_DEFINES += NXD_EXTENDED_BSD_SOCKET_SUPPORT
endif

# Turn iperf debugging on/off
ifeq (1, $(IPERF_DEBUG))
$(NAME)_DEFINES += IPERF_DEBUG=1
else
$(NAME)_DEFINES += IPERF_DEBUG=0
endif


#==============================================================================
# Global defines
#==============================================================================
ifeq (LwIP, $(NETWORK))
# Required with LwIP to prevent redefinition of struct timeval
GLOBAL_DEFINES += LWIP_TIMEVAL_PRIVATE=0
GLOBAL_DEFINES += LWIP_SO_RCVBUF=1
endif

