/*
 * Cypress Semiconductor Proprietary and Confidential. © 2016 Cypress Semiconductor.  All rights reserved.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Cypress Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Cypress Semiconductor.
 */

/** @file
 *
 * Bluetooth Low Energy Homekit accessory sample
 *
 */
#ifndef __BTLE_HOMEKIT2_LIGHTBULB_H
#define __BTLE_HOMEKIT2_LIGHTBULB_H

/*
 *  BLE handles
 */

enum
{
    HDLS_GATT                                       = 0x01,

    HDLS_GAP                                        = 0x14,
    HDLC_GAP_DEVICE_NAME                            = 0x15,
    HDLC_GAP_DEVICE_NAME_VALUE                      = 0x16,
    HDLC_GAP_APPEARANCE                             = 0x17,
    HDLC_GAP_APPEARANCE_NAME_VALUE                  = 0x18,

    HDLS_ACCESSORY_INFO                             = 0x28,
    HDLC_ACCESSORY_INFO_INSTANCE_ID                 = 0x29,
    HDLC_ACCESSORY_INFO_INSTANCE_ID_VALUE           = 0x2A,
    HDLC_ACCESSORY_INFO_IDENTIFY                    = 0x2B,
    HDLC_ACCESSORY_INFO_IDENTIFY_VALUE              = 0x2C,
    HDLD_ACCESSORY_INFO_IDENTIFY_INSTANCE_ID        = 0x2D,
    HDLC_ACCESSORY_INFO_MANUFACTURER                = 0x2E,
    HDLC_ACCESSORY_INFO_MANUFACTURER_VALUE          = 0x2F,
    HDLD_ACCESSORY_INFO_MANUFACTURER_INSTANCE_ID    = 0x30,
    HDLC_ACCESSORY_INFO_MODEL                       = 0x31,
    HDLC_ACCESSORY_INFO_MODEL_VALUE                 = 0x32,
    HDLD_ACCESSORY_INFO_MODEL_INSTANCE_ID           = 0x33,
    HDLC_ACCESSORY_INFO_NAME                        = 0x34,
    HDLC_ACCESSORY_INFO_NAME_VALUE                  = 0x35,
    HDLD_ACCESSORY_INFO_NAME_INSTANCE_ID            = 0x36,
    HDLC_ACCESSORY_INFO_SERIAL_NUMBER               = 0x37,
    HDLC_ACCESSORY_INFO_SERIAL_NUMBER_VALUE         = 0x38,
    HDLD_ACCESSORY_INFO_SERIAL_NUMBER_INSTANCE_ID   = 0x39,
    HDLC_ACCESSORY_INFO_FIRMWARE_REVISION           = 0x3A,
    HDLC_ACCESSORY_INFO_FIRMWARE_REVISION_VALUE     = 0x3B,
    HDLD_ACCESSORY_INFO_FIRMWARE_REVISION_INSTANCE_ID = 0x3C,

    HDLS_PROTOCOL_INFO                              = 0x40,
    HDLC_PROTOCOL_INFO_INSTANCE_ID                  = 0x41,
    HDLC_PROTOCOL_INFO_INSTANCE_ID_VALUE            = 0x42,
    HDLC_PROTOCOL_INFO_VERSION                      = 0x43,
    HDLC_PROTOCOL_INFO_VERSION_VALUE                = 0x44,
    HDLD_PROTOCOL_INFO_VERSION_INSTANCE_ID          = 0x45,

    HDLS_LIGHTBULB                                  = 0x50,
    HDLC_LIGHTBULB_INSTANCE_ID                      = 0x51,
    HDLC_LIGHTBULB_INSTANCE_ID_VALUE                = 0x52,
    HDLC_LIGHTBULB_BRIGHTNESS                       = 0x53,
    HDLC_LIGHTBULB_BRIGHTNESS_VALUE                 = 0x54,
    HDLD_LIGHTBULB_BRIGHTNESS_INSTANCE_ID           = 0x55,
    HDLD_LIGHTBULB_BRIGHTNESS_CLNT_CHAR_CFG         = 0x56,
    HDLC_LIGHTBULB_ON                               = 0x57,
    HDLC_LIGHTBULB_ON_VALUE                         = 0x58,
    HDLD_LIGHTBULB_ON_INSTANCE_ID                   = 0x59,
    HDLD_LIGHTBULB_ON_CLNT_CHAR_CFG                 = 0x5a,
    HDLC_LIGHTBULB_NAME                             = 0x5b,
    HDLC_LIGHTBULB_NAME_VALUE                       = 0x5c,
    HDLD_LIGHTBULB_NAME_INSTANCE_ID                 = 0x5d,
    HDLC_LIGHTBULB_HUE                              = 0x5e,
    HDLC_LIGHTBULB_HUE_VALUE                        = 0x5f,
    HDLD_LIGHTBULB_HUE_INSTANCE_ID                  = 0x60,
    HDLD_LIGHTBULB_HUE_CLNT_CHAR_CFG                = 0x61,
    HDLC_LIGHTBULB_SATURATION                       = 0x62,
    HDLC_LIGHTBULB_SATURATION_VALUE                 = 0x63,
    HDLD_LIGHTBULB_SATURATION_INSTANCE_ID           = 0x64,
    HDLD_LIGHTBULB_SATURATION_CLNT_CHAR_CFG         = 0x65,

};


/* HCI Control API */

/*
 * Apple HomeKit group code
 */
#define HCI_CONTROL_GROUP_HK                                  0xFE

/*
 * Apple HomeKit commands
 */
#define HCI_CONTROL_HK_COMMAND_READ                         ( ( HCI_CONTROL_GROUP_HK << 8 ) | 0x01 )    /* Read characteristic */
#define HCI_CONTROL_HK_COMMAND_WRITE                        ( ( HCI_CONTROL_GROUP_HK << 8 ) | 0x02 )    /* Write characteristic */
#define HCI_CONTROL_HK_COMMAND_LIST                         ( ( HCI_CONTROL_GROUP_HK << 8 ) | 0x03 )    /* List all characteristics */
#define HCI_CONTROL_HK_COMMAND_FACTORY_RESET                ( ( HCI_CONTROL_GROUP_HK << 8 ) | 0x04 )    /* Factory reset */

/*
 * Apple HomeKit events
 */
#define HCI_CONTROL_HK_EVENT_READ_RESPONSE                  ( ( HCI_CONTROL_GROUP_HK << 8 ) | 0x01 )    /* Response to read characteristic command */
#define HCI_CONTROL_HK_EVENT_UPDATE                         ( ( HCI_CONTROL_GROUP_HK << 8 ) | 0x02 )    /* Characteristic value update */
#define HCI_CONTROL_HK_EVENT_LIST_ITEM                      ( ( HCI_CONTROL_GROUP_HK << 8 ) | 0x03 )    /* Characteristic list item */

#endif

