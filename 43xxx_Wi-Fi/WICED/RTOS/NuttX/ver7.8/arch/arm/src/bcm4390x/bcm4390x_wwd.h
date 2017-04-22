/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

#ifndef __ARCH_ARM_SRC_BCM4390X_BCM4390X_WWD_H
#define __ARCH_ARM_SRC_BCM4390X_BCM4390X_WWD_H

#include <nuttx/config.h>

#include <stdbool.h>

#include "wwd_constants.h"
#include "network/wwd_network_constants.h"
#include "wwd_buffer.h"

#include "wiced_constants.h"

typedef enum bcm4390x_wwd_link_event
{
  BCM4390X_WWD_LINK_EVENT_UP,
  BCM4390X_WWD_LINK_EVENT_DOWN,
  BCM4390X_WWD_LINK_EVENT_WIRELESS_UP,
  BCM4390X_WWD_LINK_EVENT_WIRELESS_DOWN,
  BCM4390X_WWD_LINK_EVENT_WIRELESS_RENEW
} bcm4390x_wwd_link_event_t;

int bcm4390x_wwd_init(void);
void bcm4390x_wwd_deinit(void);

int bcm4390x_wwd_wlan_init(void);
int bcm4390x_wwd_wlan_deinit(void);

int bcm4390x_wwd_wlan_up(wiced_interface_t interface);
int bcm4390x_wwd_wlan_down(wiced_interface_t interface);

int bcm4390x_wwd_register_multicast_address(wiced_interface_t interface, const uint8_t *mac);
int bcm4390x_wwd_unregister_multicast_address(wiced_interface_t interface, const uint8_t *mac);

int bcm4390x_wwd_get_interface_mac_address(wiced_interface_t interface, uint8_t *mac);

uint8_t* bcm4390x_wwd_get_buffer_data(wiced_buffer_t buffer, uint16_t *len);

void bcm4390x_wwd_rxavail(wiced_interface_t interface, wiced_buffer_t buffer);

void bcm4390x_wwd_send_ethernet_data(wiced_interface_t interface, wiced_buffer_t buffer, uint16_t len, bool tx_enable);

uint8_t* bcm4390x_wwd_alloc_tx_buffer(wiced_buffer_t *buffer);
void bcm4390x_wwd_free_tx_buffer(wiced_buffer_t buffer);

void bcm4390x_wwd_free_rx_buffer(wiced_buffer_t buffer);

void bcm4390x_wwd_buffer_init_fifo(wiced_buffer_fifo_t *fifo);
void bcm4390x_wwd_buffer_push_to_fifo(wiced_buffer_fifo_t *fifo, wiced_buffer_t buffer);
wiced_buffer_t bcm4390x_wwd_buffer_pop_from_fifo(wiced_buffer_fifo_t *fifo);

void bcm4390x_wwd_link_event_handler(wiced_interface_t interface, bcm4390x_wwd_link_event_t event);

#endif /* __ARCH_ARM_SRC_BCM4390X_BCM4390X_WWD_H */
