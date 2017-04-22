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
NAME               := Lib_UPnP_AV_render

$(NAME)_SOURCES    := \
						src/main.c \
						src/upnp.c \
						src/upnp_control.c \
						src/upnp_connmgr.c \
						src/upnp_transport.c \
						src/song-meta-data.c \
						src/variable-container.c \
						src/upnp_device.c \
						src/upnp_renderer.c \
						src/webserver.c \
						src/output.c \
						src/logging.c \
						src/xmldoc.c \
						src/xmlescape.c \
						src/output_wiced_audio.c

$(NAME)_INCLUDES   := \
						. \
						src

$(NAME)_DEFINES    := HAVE_CONFIG_H

$(NAME)_CFLAGS     += -std=gnu99

$(NAME)_COMPONENTS := protocols/UPNP
$(NAME)_COMPONENTS += audio/audio_client
$(NAME)_COMPONENTS += utilities/wiced_log

$(NAME)_RESOURCES  := \
						images/grender_128.png \
						images/grender_64.png

GLOBAL_INCLUDES    += .

KEEP_LIST          := *
