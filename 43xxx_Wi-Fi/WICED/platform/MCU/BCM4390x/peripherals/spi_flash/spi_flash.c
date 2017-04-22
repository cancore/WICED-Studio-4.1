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

/*
 * WICED common Sflash driver
 *
 * History :
 * 2015/11/20
 * Add Sflash feature support:
 *      1. Quad mode enable/disable/query
 *      2. Block protect configure/query
 *          - 64KB protect
 *          - 128KB protect
 *          - 256KB protect
 *          - 512KB protect
 *          - 1024KB protect
 *          - 2048KB protect
 *          - 4096KB protect
 *          - 8192KB protect
 *          - 16384KB protect
 *          - 32768KB protect
 *      3. HIGH SPEED for READ/WRITE
 *      4. Erase mode : Sector(4KB), Block(64KB), All(all chip)
 *      5. Reset Sflash
 * Testing report:
 *      1. Had testing Read/Write/Erase and configure
 *         block protect/Quad mode with below Sflash,
 *         they all work well :
 *         (1) SFLASH_ID_MX25L6433F (MACRONIX)
 *         (2) SFLASH_ID_N25Q064A   (MICRON)
 *         (3) SFLASH_ID_W25Q64FV   (WINBOND)
 */

#include "wiced.h"
#include "spi_flash.h"
#include "platform_spi_flash.h"
#include "platform/wwd_platform_interface.h"
#include <string.h>

#define WPRINT_SFLASH_INFO( args )
#if ( PLATFORM_NO_SFLASH_WRITE == 0 )
#ifdef SFLASH_MSG_DEBUG
    #undef WPRINT_SFLASH_INFO
    #define WPRINT_SFLASH_INFO( args ) WPRINT_MACRO( args )
#endif /* SFLASH_MSG_DEBUG */
#endif /* PLATFORM_NO_SFLASH_WRITE */

/******************************************************
 *                      Macros
 ******************************************************/
/* Status register bit of Common (MACRONIX) sflash */
#define STATUS_REGISTER_BIT_BUSY                    ( 0x1 )
#define STATUS_REGISTER_BIT_WRITE_ENABLE            ( 0x2 )
#define STATUS_REGISTER_BIT_BLOCK_PROTECT_0         ( 0x4 )
#define STATUS_REGISTER_BIT_BLOCK_PROTECT_1         ( 0x8 )
#define STATUS_REGISTER_BIT_BLOCK_PROTECT_2         ( 0x10 )
#define STATUS_REGISTER_BIT_BLOCK_PROTECT_3         ( 0x20 )
#define STATUS_REGISTER_BIT_QUAD_ENABLE             ( 0x40 )
#define STATUS_REGISTER_BIT_REGISTER_WRITE_PROTECT  ( 0x80 )

/* Status register bit of MICRON sflash */
#ifdef SFLASH_SUPPORT_MICRON_PARTS
#define MICRON_STATUS_REGISTER_BIT_BLOCK_PROTECT_3 ( 0x40 )
#define MICRON_SFLASH_ENH_VOLATILE_STATUS_REGISTER_HOLD ( 0x10 ) /* HOLD# */
#endif /* SFLASH_SUPPORT_MICRON_PARTS */

/* Status register bit of WINBOND sflash */
#ifdef SFLASH_SUPPORT_WINBOND_PARTS
#define WINBOND_STATUS_REGISTER_BIT_QUAD_ENABLE ( 0x200 )
#define WINBOND_STATUS_REGISTER_DATA_LENGTH ( 2 )
#endif /* SFLASH_SUPPORT_WINBOND_PARTS */

/* Stage of initialize sflash */
#define WICED_SFLASH_INIT_STAGE_0 ( 0 )
#define WICED_SFLASH_INIT_STAGE_1 ( 1 )
#define WICED_SFLASH_INIT_STAGE_2 ( 2 )
#define WICED_SFLASH_INIT_STAGE_CLEAR_ALL ( 99 )
/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
 * Common (MACRONIX) Sflash action which will link to sflash_action_t.
 */
static int common_sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode );
static wiced_bool_t common_sflash_is_quad_mode( sflash_handle_t* sflash_handle ) __attribute__ ( ( unused ) );
static int common_sflash_set_block_protect( sflash_handle_t* sflash_handle, block_protect_action_t mode ) __attribute__ ( ( unused ) );
static int common_sflash_query_block_protect( sflash_handle_t* sflash_handle, uint32_t* protect_level ) __attribute__ ( ( unused ) );
static int common_sflash_set_write_enable( sflash_handle_t* sflash_handle, onoff_action_t mode );
static wiced_bool_t common_sflash_is_write_enable( sflash_handle_t* sflash_handle ) __attribute__ ( ( unused ) );
static int common_sflash_reset( sflash_handle_t* sflash_handle ) __attribute__ ( ( unused ) );
static wiced_bool_t common_sflash_is_busy( sflash_handle_t* sflash_handle );

#ifdef SFLASH_SUPPORT_MICRON_PARTS
/**
 * MICRON Sflash action which will link to sflash_action_t.
 */
static int micron_sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode );
static wiced_bool_t micron_sflash_is_quad_mode( sflash_handle_t* sflash_handle ) __attribute__ ( ( unused ) );
static int micron_sflash_set_block_protect( sflash_handle_t* sflash_handle, block_protect_action_t mode ) __attribute__ ( ( unused ) );
static int micron_sflash_query_block_protect( sflash_handle_t* sflash_handle, uint32_t* protect_level ) __attribute__ ( ( unused ) );
#endif /* SFLASH_SUPPORT_MICRON_PARTS */

#ifdef SFLASH_SUPPORT_WINBOND_PARTS
/**
 * WINBOND Sflash action which will link to sflash_action_t.
 */
static int winbond_sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode );
static wiced_bool_t winbond_sflash_is_quad_mode( sflash_handle_t* sflash_handle ) __attribute__ ( ( unused ) );
static int winbond_sflash_set_block_protect( sflash_handle_t* sflash_handle, block_protect_action_t mode ) __attribute__ ( ( unused ) );
static int winbond_sflash_query_block_protect( sflash_handle_t* sflash_handle, uint32_t* protect_level ) __attribute__ ( ( unused ) );
static int winbond_sflash_status_register( sflash_handle_t* sflash_handle, void* mask, void* data, getset_action_t mode);
#endif /* SFLASH_SUPPORT_WINBOND_PARTS */

#ifdef SFLASH_SUPPORT_SST_PARTS
/**
 * SST Sflash action which will link to sflash_action_t.
 */
static int sst_sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode );
static wiced_bool_t sst_sflash_is_quad_mode( sflash_handle_t* sflash_handle ) __attribute__ ( ( unused ) );
#endif /* SFLASH_SUPPORT_SST_PARTS */

/* Sflash driver function */
/**
 * Query Sflash ID.
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param rx_data           refer point to receive data of sflash id
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_read_id( sflash_handle_t* sflash_handle, void* rx_data );

/**
 * Base on Sflash ID to detect sflash parameter table.
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param sflash_id         Sflash ID
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_get_capability( sflash_handle_t* sflash_handle, uint32_t sflash_id );

/**
 * Sending sflash command to SPI controller
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param command           Sflash command
 * @param data              refer point for send/receive data.
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_generic_command( sflash_handle_t* sflash_handle, sflash_command_t command, void* data );

/**
 * For setup and query status register
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param mask              specific bit of status register
 * @param data              configured bit of status register
 * @param mode              setup or query status register
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_status_register( sflash_handle_t* sflash_handle, void* mask, void* data, getset_action_t mode);

/**
 * Setup Quad mode of Sflash
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param mode              Turn on/off Quad mode
 * @param mask_bit          The status register bit of Quad mode
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode, uint32_t mask_bit );

/**
 * Setup Block Protect of Sflash
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param mode              send block protect mode to status register.
 * @param mask_bit          The status register bit of Block Protect
 * @param map_bit           Mapping each block protect bits with each block protect mode
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_set_block_protect( sflash_handle_t* sflash_handle, block_protect_action_t mode, uint32_t mask_bit, uint32_t* map_bit );

/**
 * Query Block Protect of Sflash
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param protect_level     Retrieve current block protect mode from status register.
 * @param mask_bit          The status register bit of Block Protect
 * @param map_bit           Mapping each block protect bits with each block protect mode
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_query_block_protect( sflash_handle_t* sflash_handle, uint32_t* protect_level, uint32_t mask_bit, uint32_t* map_bit );

/**
 * Setup Write Enable of Sflash ( We should set Write Enable on before any action of write data )
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param mode              setup Write Enable on/off
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_set_write_enable( sflash_handle_t* sflash_handle, onoff_action_t mode );

/**
 * Sflash reset, all non-volatile register/data will be clear.
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_reset( sflash_handle_t* sflash_handle );

/**
 * Use for query specific bit of status register.
 * ex :
 *      is_quad_mode
 *      is_write_enable
 *      is_busy
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param mask_bit          specific bit of status register
 * @return                  if specific bit had been set as 1, then return Yes(WICED_TRUE) or return No(WICED_FALSE)
 */
static wiced_bool_t sflash_is_status_register_bit_set( sflash_handle_t* sflash_handle, uint32_t mask_bit );

/**
 * After any action of write, will call sflash_wait_busy_done to wait for sflash completely ( not busy )
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @return                  return success(SFLASH_MSG_OK) or fail(SFLASH_MSG_ERROR)
 */
static int sflash_wait_busy_done( sflash_handle_t* sflash_handle );

/**
 * Each Sflash command has it's own data length to show that how many data will be read/write after issuing command.
 * @param command           Sflash command
 * @return                  return data length
 */
static uint32_t sflash_get_command_data_length( sflash_command_t command );

/**
 * De-initialize sflash by stage level.
 * @param sflash_handle     Handle structure that was initialized with @ref sflash_init
 * @param init_stage        The stage of sflash init
 * @return                  return none
 */
static void sflash_deinit_by_stage( sflash_handle_t* sflash_handle, int8_t init_stage );

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
struct sflash_capabilities_table_element
{
        uint32_t device_id;
        sflash_capabilities_t capabilities;
};

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct sflash_capabilities_table_element sflash_capabilities_table_element_t;

/******************************************************
 *               Variables Definitions
 ******************************************************/
/* Structure (sflash_action_t/sflash_speed_advance_t) for Common sflash */
static const sflash_action_t common_action = {
#if ( PLATFORM_NO_SFLASH_WRITE == 0 )
        .is_quad_mode =         common_sflash_is_quad_mode,
        .set_block_protect =    common_sflash_set_block_protect,
        .query_block_protect =  common_sflash_query_block_protect,
        .is_write_enable =      common_sflash_is_write_enable,
        .reset =                common_sflash_reset,
#endif /* PLATFORM_NO_SFLASH_WRITE */
        .set_quad_mode =        common_sflash_set_quad_mode,
        .is_busy =              common_sflash_is_busy,
        .set_write_enable =     common_sflash_set_write_enable,
        .status_register =      sflash_status_register
};

static const sflash_speed_advance_t common_speed_config = {
        .fast_read_dummy_cycle =             8,
        .high_speed_read_dummy_cycle =       6,
        .high_speed_read_x4io_support =     WICED_TRUE,
        .high_speed_read_mode_bit_support = WICED_FALSE,
        .high_speed_write_support =         WICED_TRUE,
        .high_speed_write_x4io_support =    WICED_TRUE,
        .high_speed_write_quad_address_mode = WICED_TRUE
};

static const sflash_speed_advance_t common_lowspeed_config = {
        .fast_read_dummy_cycle =            8,
        .high_speed_read_dummy_cycle =      0,
        .high_speed_read_x4io_support =     WICED_FALSE,
        .high_speed_read_mode_bit_support = WICED_FALSE,
        .high_speed_write_support =         WICED_FALSE,
        .high_speed_write_x4io_support =    WICED_FALSE,
        .high_speed_write_quad_address_mode = WICED_TRUE
};

#ifdef SFLASH_SUPPORT_MICRON_PARTS
/* Structure (sflash_action_t/sflash_speed_advance_t) for MICRON sflash */
static const sflash_action_t micron_action = {
#if ( PLATFORM_NO_SFLASH_WRITE == 0 )
        .is_quad_mode =         micron_sflash_is_quad_mode,
        .set_block_protect =    micron_sflash_set_block_protect,
        .query_block_protect =  micron_sflash_query_block_protect,
        .is_write_enable =      common_sflash_is_write_enable,
        .reset =                common_sflash_reset,
#endif /* PLATFORM_NO_SFLASH_WRITE */
        .set_quad_mode =        micron_sflash_set_quad_mode,
        .is_busy =              common_sflash_is_busy,
        .set_write_enable =     common_sflash_set_write_enable,
        .status_register =      sflash_status_register
};

static const sflash_speed_advance_t micron_speed_config = {
        .fast_read_dummy_cycle =            8,
        .high_speed_read_dummy_cycle =      10,
        .high_speed_read_x4io_support =     WICED_TRUE,
        .high_speed_read_mode_bit_support = WICED_FALSE,
        .high_speed_write_support =         WICED_TRUE,
        .high_speed_write_x4io_support =    WICED_FALSE,
        .high_speed_write_quad_address_mode = WICED_TRUE
};
#endif /* SFLASH_SUPPORT_MICRON_PARTS */

#ifdef SFLASH_SUPPORT_WINBOND_PARTS
/* Structure (sflash_action_t/sflash_speed_advance_t) for WINBOND sflash */
static const sflash_action_t winbond_action = {
#if ( PLATFORM_NO_SFLASH_WRITE == 0 )
        .is_quad_mode =         winbond_sflash_is_quad_mode,
        .set_block_protect =    winbond_sflash_set_block_protect,
        .query_block_protect =  winbond_sflash_query_block_protect,
        .is_write_enable =      common_sflash_is_write_enable,
        .reset =                common_sflash_reset,
#endif /* PLATFORM_NO_SFLASH_WRITE */
        .set_quad_mode =        winbond_sflash_set_quad_mode,
        .is_busy =              common_sflash_is_busy,
        .set_write_enable =     common_sflash_set_write_enable,
        .status_register =      winbond_sflash_status_register
};

static const sflash_speed_advance_t winbond_speed_config = {
        .fast_read_dummy_cycle =            8,
        .high_speed_read_dummy_cycle =      4,
        .high_speed_read_x4io_support =     WICED_TRUE,
        .high_speed_read_mode_bit_support = WICED_TRUE,
        .high_speed_write_support =         WICED_TRUE,
        .high_speed_write_x4io_support =    WICED_FALSE,
        .high_speed_write_quad_address_mode = WICED_TRUE
};
#endif /* SFLASH_SUPPORT_WINBOND_PARTS */

#ifdef SFLASH_SUPPORT_SST_PARTS
/* Structure (sflash_action_t) for SST sflash */
static const sflash_action_t sst_action = {
#if ( PLATFORM_NO_SFLASH_WRITE == 0 )
        .is_quad_mode =         sst_sflash_is_quad_mode,
        .set_block_protect =    common_sflash_set_block_protect,
        .query_block_protect =  common_sflash_query_block_protect,
        .is_write_enable =      common_sflash_is_write_enable,
        .reset =                common_sflash_reset,
#endif /* PLATFORM_NO_SFLASH_WRITE */
        .set_quad_mode =        sst_sflash_set_quad_mode,
        .is_busy =              common_sflash_is_busy,
        .set_write_enable =     common_sflash_set_write_enable,
        .status_register =      sflash_status_register
};
#endif /* SFLASH_SUPPORT_SST_PARTS */

#ifdef SFLASH_SUPPORT_ISSI_PARTS
/* Structure (sflash_speed_advance_t) for ISSI sflash */
static const sflash_speed_advance_t issi_speed_config = {
       .fast_read_dummy_cycle =            8,
       .high_speed_read_dummy_cycle =      4,
       .high_speed_read_x4io_support =     WICED_FALSE,
       .high_speed_read_mode_bit_support = WICED_TRUE,
       .high_speed_write_support =         WICED_TRUE,
       .high_speed_write_x4io_support =    WICED_FALSE,
       .high_speed_write_quad_address_mode = WICED_TRUE
};
#endif /* SFLASH_SUPPORT_ISSI_PARTS */

#ifdef SFLASH_SUPPORT_EON_PARTS
/* Structure (sflash_speed_advance_t) for EON sflash */
static const sflash_speed_advance_t eon_speed_config = {
       .fast_read_dummy_cycle =            8,
       .high_speed_read_dummy_cycle =      6,
       .high_speed_read_x4io_support =     WICED_TRUE,
       .high_speed_read_mode_bit_support = WICED_FALSE,
       .high_speed_write_support =         WICED_FALSE,
       .high_speed_write_x4io_support =    WICED_FALSE,
       .high_speed_write_quad_address_mode = WICED_TRUE
};
#endif /* SFLASH_SUPPORT_EON_PARTS */

/* Configure table for all sflash */
static const sflash_capabilities_table_element_t sflash_capabilities_tables[] =
{
#ifdef SFLASH_SUPPORT_MACRONIX_PARTS
        { SFLASH_ID_MX25L8006E,     { 1*MBYTE,    1,    .action = &common_action,   .speed_advance = &common_lowspeed_config } },
        { SFLASH_ID_MX25L1606E,     { 2*MBYTE,    1,    .action = &common_action,   .speed_advance = &common_speed_config } },
        { SFLASH_ID_MX25L6433F,     { 8*MBYTE,    256,  .action = &common_action,   .speed_advance = &common_speed_config } },
        { SFLASH_ID_MX25L12835F,    { 16*MBYTE,   256,  .action = &common_action,   .speed_advance = &common_speed_config } },
        { SFLASH_ID_MX25L25635F,    { 32*MBYTE,   256,  .action = &common_action,   .speed_advance = &common_speed_config } },
#endif /* SFLASH_SUPPORT_MACRONIX_PARTS */
#ifdef SFLASH_SUPPORT_SST_PARTS
        { SFLASH_ID_SST25VF080B,    { 1*MBYTE,    1,    .action = &sst_action,      .speed_advance = &common_speed_config } },
#endif /* SFLASH_SUPPORT_SST_PARTS */
#ifdef SFLASH_SUPPORT_EON_PARTS
        { SFLASH_ID_EN25QH16,       { 2*MBYTE,    1,    .action = &common_action,   .speed_advance = &eon_speed_config } },
#endif /* SFLASH_SUPPORT_EON_PARTS */
#ifdef SFLASH_SUPPORT_ISSI_PARTS
        { SFLASH_ID_ISSI25CQ032,    { 4*MBYTE,    256,  .action = &common_action,   .speed_advance = &issi_speed_config } },
        { SFLASH_ID_ISSI25LP064,    { 8*MBYTE,    256,  .action = &common_action,   .speed_advance = &issi_speed_config } },
#endif /* SFLASH_SUPPORT_ISSI_PARTS */
#ifdef SFLASH_SUPPORT_MICRON_PARTS
        { SFLASH_ID_N25Q064A,       { 8*MBYTE,    256,  .action = &micron_action,   .speed_advance = &micron_speed_config } },
#endif /* SFLASH_SUPPORT_MICRON_PARTS */
#ifdef SFLASH_SUPPORT_WINBOND_PARTS
        { SFLASH_ID_W25Q64FV,       { 8*MBYTE,    256,  .action = &winbond_action,  .speed_advance = &winbond_speed_config } },
#endif /* SFLASH_SUPPORT_WINBOND_PARTS */
        { SFLASH_ID_DEFAULT,        { 1*MBYTE,    1,    .action = NULL,             .speed_advance = NULL } }
};

/* bit map of protect block for common sflash */
static uint32_t block_protect_bit_map[ BLOCK_PRTOECT_MAX_ENUM ] =
{
        [ BLOCK_PRTOECT_TOP_0KB ] =       0,
        [ BLOCK_PRTOECT_TOP_64KB ] =      STATUS_REGISTER_BIT_BLOCK_PROTECT_0,
        [ BLOCK_PRTOECT_TOP_128KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_1,
        [ BLOCK_PRTOECT_TOP_256KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_1,
        [ BLOCK_PRTOECT_TOP_512KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_1024KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_2048KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_4096KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_1 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_8192KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_3,
        [ BLOCK_PRTOECT_TOP_16384KB ] =   STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_3,
        [ BLOCK_PRTOECT_TOP_32768KB ] =   STATUS_REGISTER_BIT_BLOCK_PROTECT_1 | STATUS_REGISTER_BIT_BLOCK_PROTECT_3
};

#ifdef SFLASH_SUPPORT_MICRON_PARTS
/* bit map of protect block for MICRON sflash */
static uint32_t micron_block_protect_bit_map[ BLOCK_PRTOECT_MAX_ENUM ] =
{
        [ BLOCK_PRTOECT_TOP_0KB ] =       0,
        [ BLOCK_PRTOECT_TOP_64KB ] =      STATUS_REGISTER_BIT_BLOCK_PROTECT_0,
        [ BLOCK_PRTOECT_TOP_128KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_1,
        [ BLOCK_PRTOECT_TOP_256KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_1,
        [ BLOCK_PRTOECT_TOP_512KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_1024KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_2048KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_4096KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_1 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_8192KB ] =    MICRON_STATUS_REGISTER_BIT_BLOCK_PROTECT_3,
};
#endif /* SFLASH_SUPPORT_MICRON_PARTS */

#ifdef SFLASH_SUPPORT_WINBOND_PARTS
/* bit map of protect block for WINBOND sflash */
static uint32_t winbond_block_protect_bit_map[ BLOCK_PRTOECT_MAX_ENUM ] =
{
        [ BLOCK_PRTOECT_TOP_0KB ] =       0,
        [ BLOCK_PRTOECT_TOP_128KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_0,
        [ BLOCK_PRTOECT_TOP_256KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_1,
        [ BLOCK_PRTOECT_TOP_512KB ] =     STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_1,
        [ BLOCK_PRTOECT_TOP_1024KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_2048KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_4096KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2,
        [ BLOCK_PRTOECT_TOP_8192KB ] =    STATUS_REGISTER_BIT_BLOCK_PROTECT_0 | STATUS_REGISTER_BIT_BLOCK_PROTECT_1 | STATUS_REGISTER_BIT_BLOCK_PROTECT_2
};
#endif /* SFLASH_SUPPORT_WINBOND_PARTS */

/*************************
 * Common Sflash function*
 *************************/
static int common_sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode )
{
    return sflash_set_quad_mode( sflash_handle, mode, STATUS_REGISTER_BIT_QUAD_ENABLE );
}

static wiced_bool_t common_sflash_is_quad_mode( sflash_handle_t* sflash_handle )
{
    return sflash_is_status_register_bit_set( sflash_handle, STATUS_REGISTER_BIT_QUAD_ENABLE );
}

static int common_sflash_set_block_protect( sflash_handle_t* sflash_handle, block_protect_action_t mode )
{
    uint32_t mask = STATUS_REGISTER_BIT_BLOCK_PROTECT_0 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_2 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_3;
    return sflash_set_block_protect( sflash_handle, mode, mask, block_protect_bit_map );
}

static int common_sflash_query_block_protect( sflash_handle_t* sflash_handle, uint32_t* protect_level )
{
    uint32_t mask = STATUS_REGISTER_BIT_BLOCK_PROTECT_0 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_2 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_3;
    return sflash_query_block_protect( sflash_handle, protect_level, mask, block_protect_bit_map );
}

static int common_sflash_set_write_enable( sflash_handle_t* sflash_handle, onoff_action_t mode )
{
    return sflash_set_write_enable( sflash_handle, mode );
}

static wiced_bool_t common_sflash_is_write_enable( sflash_handle_t* sflash_handle )
{
    return sflash_is_status_register_bit_set( sflash_handle, STATUS_REGISTER_BIT_WRITE_ENABLE );
}

static int common_sflash_reset( sflash_handle_t* sflash_handle )
{
    return sflash_reset( sflash_handle );
}

static wiced_bool_t common_sflash_is_busy( sflash_handle_t* sflash_handle )
{
    return sflash_is_status_register_bit_set( sflash_handle, STATUS_REGISTER_BIT_BUSY );
}

/*****************************************
 *   Area of sflash specific definition  *
 *****************************************/
 /* SFLASH - MICRON */
#ifdef SFLASH_SUPPORT_MICRON_PARTS
static int micron_sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode )
{
    int result;
    uint32_t data = 0;

    UNUSED_PARAMETER( mode );

    /* Disable function of HOLD# for Quad mode */
    result = sflash_handle->capabilities->action->set_write_enable( sflash_handle, TURN_ON );
    if ( result != SFLASH_MSG_OK )
    {
        WPRINT_SFLASH_INFO( ( "!!! [%s] - set WRITE_ENABLE, Fail!!!\n", __FUNCTION__ ) );
        return result;
    }
    result = sflash_generic_command( sflash_handle, SFLASH_READ_ENH_VOLATILE_REGISTER, &data );
    if ( result != SFLASH_MSG_OK )
    {
        WPRINT_SFLASH_INFO( ( "!!! [%s] - read SFLASH_READ_ENH_VOLATILE_REGISTER, Fail!!!\n", __FUNCTION__ ) );
        return result;
    }
    data &= ( unsigned char )~( MICRON_SFLASH_ENH_VOLATILE_STATUS_REGISTER_HOLD );
    result = sflash_generic_command( sflash_handle, SFLASH_WRITE_ENH_VOLATILE_REGISTER, &data );
    if ( result != SFLASH_MSG_OK )
    {
        WPRINT_SFLASH_INFO( ( "!!! [%s] - write SFLASH_READ_ENH_VOLATILE_REGISTER, Fail!!!\n", __FUNCTION__ ) );
        return result;
    }
    WPRINT_SFLASH_INFO( ( "Ignoring HOLD# for Quad mode, done!\n" ) );
    WPRINT_SFLASH_INFO( ( "MICRON Sflash not support to configure Quad mode. ( default : enabled )\n" ) );
    return result;
}

static wiced_bool_t micron_sflash_is_quad_mode( sflash_handle_t* sflash_handle )
{
    UNUSED_PARAMETER( sflash_handle );
    return WICED_TRUE;
}

static int micron_sflash_set_block_protect( sflash_handle_t* sflash_handle, block_protect_action_t mode )
{
    uint32_t mask = STATUS_REGISTER_BIT_BLOCK_PROTECT_0 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_2 |
                    MICRON_STATUS_REGISTER_BIT_BLOCK_PROTECT_3;
    return sflash_set_block_protect( sflash_handle, mode, mask, micron_block_protect_bit_map );
}

static int micron_sflash_query_block_protect( sflash_handle_t* sflash_handle, uint32_t *protect_level )
{
    uint32_t mask = STATUS_REGISTER_BIT_BLOCK_PROTECT_0 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_2 |
                    MICRON_STATUS_REGISTER_BIT_BLOCK_PROTECT_3;
    return sflash_query_block_protect( sflash_handle, protect_level, mask, micron_block_protect_bit_map );
}
#endif /* SFLASH_SUPPORT_MICRON_PARTS */

 /* SFLASH - WINBOND */
#ifdef SFLASH_SUPPORT_WINBOND_PARTS
static int winbond_sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode )
{
    return sflash_set_quad_mode( sflash_handle, mode, WINBOND_STATUS_REGISTER_BIT_QUAD_ENABLE );
}

static wiced_bool_t winbond_sflash_is_quad_mode( sflash_handle_t* sflash_handle )
{
    return sflash_is_status_register_bit_set( sflash_handle, WINBOND_STATUS_REGISTER_BIT_QUAD_ENABLE );
}

static int winbond_sflash_set_block_protect( sflash_handle_t* sflash_handle, block_protect_action_t mode )
{
    uint32_t mask = STATUS_REGISTER_BIT_BLOCK_PROTECT_0 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_2;
    return sflash_set_block_protect( sflash_handle, mode, mask, winbond_block_protect_bit_map );
}

static int winbond_sflash_query_block_protect( sflash_handle_t* sflash_handle, uint32_t* protect_level )
{
    uint32_t mask = STATUS_REGISTER_BIT_BLOCK_PROTECT_0 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_1 |
                    STATUS_REGISTER_BIT_BLOCK_PROTECT_2;
    return sflash_query_block_protect( sflash_handle, protect_level, mask, winbond_block_protect_bit_map );
}

static int winbond_sflash_status_register( sflash_handle_t* sflash_handle, void* mask, void* data, getset_action_t mode)
{
    int result;
    uint32_t write_data_len = WINBOND_STATUS_REGISTER_DATA_LENGTH;
    uint32_t *target_data = ( ( uint32_t* ) data );
    uint32_t *target_mask = ( ( uint32_t* ) mask );
    uint32_t temp_data = *( ( uint32_t* ) data );
    uint32_t target_data2;

    result = sflash_generic_command( sflash_handle, SFLASH_READ_STATUS_REGISTER, target_data );
    if ( result != SFLASH_MSG_OK )
    {
        return result;
    }
    result = sflash_generic_command( sflash_handle, SFLASH_READ_STATUS_REGISTER2, &target_data2 );
    if ( result != SFLASH_MSG_OK )
    {
        return result;
    }
    *target_data = ( unsigned short )( ( target_data2 << 8 ) | *target_data );

    switch ( mode )
    {
        case SETUP_ACTION :
            if ( ( *target_data & *target_mask ) == temp_data )
            {
                WPRINT_SFLASH_INFO( ( "Status register is no change!\n" ) );
                return SFLASH_MSG_OK;
            }

            *target_data &= ~( *target_mask );
            *target_data |= temp_data;
            result = sflash_handle->capabilities->action->set_write_enable( sflash_handle, TURN_ON );
            if ( result != SFLASH_MSG_OK )
            {
                return result;
            }
            result = spi_sflash_send_command( sflash_handle, SFLASH_WRITE_STATUS_REGISTER, 0, target_data, &write_data_len );
            if ( result != SFLASH_MSG_OK )
            {
                return result;
            }
            result = sflash_wait_busy_done( sflash_handle );
            if ( result != SFLASH_MSG_OK )
            {
                return SFLASH_MSG_ERROR;
            }
            WPRINT_SFLASH_INFO( ( "[SETUP_ACTION] done!\n" ) );
            break;
        case QUERY_ACTION :
            WPRINT_SFLASH_INFO( ( "[QUERY_ACTION] Latest register status = %d\n", *( ( int* ) data ) ) );
            break;
        default :
            WPRINT_SFLASH_INFO( ( "!!! Warning, use wrong mode for %s !!!\n", __FUNCTION__ ) );
            return SFLASH_MSG_ERROR;
            break;
    }
    return result;
}
#endif /* SFLASH_SUPPORT_WINBOND_PARTS */

 /* SFLASH - SST */
#ifdef SFLASH_SUPPORT_SST_PARTS
static int sst_sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode )
{
    UNUSED_PARAMETER( sflash_handle );
    UNUSED_PARAMETER( mode );

    WPRINT_SFLASH_INFO( ( "SST Sflash not support Quad mode.\n" ) );
    return SFLASH_MSG_ERROR;
}

static wiced_bool_t sst_sflash_is_quad_mode( sflash_handle_t* sflash_handle )
{
    UNUSED_PARAMETER( sflash_handle );
    return WICED_FALSE;
}
#endif /* SFLASH_SUPPORT_SST_PARTS */

/*********************************
 * Sflash driver private function *
 *********************************/
static int sflash_read_id( sflash_handle_t* sflash_handle, void* rx_data )
{
    int result;

    result = sflash_generic_command( sflash_handle, SFLASH_READID_JEDEC_ID, rx_data );
    *( ( uint32_t* )rx_data ) &= SFLASHID_MASK;
    WPRINT_SFLASH_INFO( ( "Get Sflash ID (0x%lx)\n", *( ( uint32_t* )rx_data ) ) );

    return result;
}

static int sflash_get_capability( sflash_handle_t* sflash_handle, uint32_t sflash_id )
{
    const sflash_capabilities_table_element_t* capabilities_element = sflash_capabilities_tables;

    while ( ( capabilities_element->device_id != SFLASH_ID_DEFAULT ) &&
            ( capabilities_element->device_id != sflash_id ) )
    {
        capabilities_element++;
    }

    if ( capabilities_element->device_id == sflash_id )
    {
        sflash_handle->device_id = sflash_id;
        sflash_handle->read_blocking = WICED_TRUE;
        sflash_handle->capabilities = &capabilities_element->capabilities;
        return SFLASH_MSG_OK;
    }
    return SFLASH_MSG_ERROR;
}

static int sflash_generic_command( sflash_handle_t* sflash_handle, sflash_command_t command, void* data )
{
    int result;
    uint32_t data_length = sflash_get_command_data_length( command );

    result = spi_sflash_send_command( sflash_handle, command, 0, data, &data_length );

    if ( result != SFLASH_MSG_OK )
    {
        WPRINT_SFLASH_INFO( ( "[%s] - issue sflash command(0x%x) fail !\n\n", __FUNCTION__, command ) );
    }
    return result;
}

static int sflash_status_register( sflash_handle_t* sflash_handle, void* mask, void* data, getset_action_t mode )
{
    int result;
    uint32_t *target_data = ( ( uint32_t* ) data );
    uint32_t *target_mask = ( ( uint32_t* ) mask );
    uint32_t temp_data = *( ( uint32_t* ) data );

    result = sflash_generic_command( sflash_handle, SFLASH_READ_STATUS_REGISTER, target_data );
    if ( result != SFLASH_MSG_OK )
    {
        return result;
    }

    switch ( mode )
    {
        case SETUP_ACTION :
            if ( ( *target_data & *target_mask ) == temp_data )
            {
                WPRINT_SFLASH_INFO( ( "Status register is no change!\n" ) );
                return SFLASH_MSG_OK;
            }

            *target_data &= ~( *target_mask );
            *target_data |= temp_data;
            result = sflash_handle->capabilities->action->set_write_enable( sflash_handle, TURN_ON );
            if ( result != SFLASH_MSG_OK )
            {
                return result;
            }
            result = sflash_generic_command( sflash_handle, SFLASH_WRITE_STATUS_REGISTER, target_data );
            if ( result != SFLASH_MSG_OK )
            {
                return result;
            }
            result = sflash_wait_busy_done( sflash_handle );
            if ( result != SFLASH_MSG_OK )
            {
                return result;
            }
            WPRINT_SFLASH_INFO( ( "[SETUP_ACTION] done!\n" ) );
            break;
        case QUERY_ACTION :
            WPRINT_SFLASH_INFO( ( "[QUERY_ACTION] Latest register status = %d\n", *( ( int* ) data ) ) );
            break;
        default :
            WPRINT_SFLASH_INFO( ( "!!! Warning, use wrong mode for %s !!!\n", __FUNCTION__ ) );
            return SFLASH_MSG_ERROR;
            break;
    }
    return result;
}

static int sflash_set_quad_mode( sflash_handle_t* sflash_handle, onoff_action_t mode, uint32_t mask_bit )
{
    uint32_t data = 0;
    uint32_t mask = mask_bit;

    if ( mode == TURN_OFF )
    {
        data &= ~( mask );
    }
    else
    {
        data |= ( mask );
    }
    return sflash_handle->capabilities->action->status_register( sflash_handle, &mask, &data, SETUP_ACTION );
}

static int sflash_set_block_protect( sflash_handle_t* sflash_handle, block_protect_action_t mode, uint32_t mask_bit, uint32_t* map_bit )
{
    uint32_t data = 0;
    uint32_t mode_to_size = 0;
    uint32_t mask = mask_bit;

    BLOCK_PROTECT_MODE_TO_SIZE( mode, mode_to_size );
    if ( sflash_handle->capabilities->total_size < ( mode_to_size * KBYTE ) )
    {
        WPRINT_SFLASH_INFO( ( "WARNING!! You want to protect size of %d bytes is large than Sflash total size ( %d bytes )\n", ( int )( mode_to_size * KBYTE ), ( int )sflash_handle->capabilities->total_size ) );
        WPRINT_SFLASH_INFO( ( "Executing Protect ALL!\n" ) );
        BLOCK_PROTECT_SIZE_TO_MODE( ( int )(sflash_handle->capabilities->total_size / KBYTE), mode );
    }

    data = ( ( uint32_t* ) map_bit )[ mode ];

    return sflash_handle->capabilities->action->status_register( sflash_handle, &mask, &data, SETUP_ACTION );
}

static int sflash_query_block_protect( sflash_handle_t* sflash_handle, uint32_t* protect_level, uint32_t mask_bit, uint32_t* map_bit )
{
    int result;
    uint32_t i;
    uint32_t data = 0;
    uint32_t mask = mask_bit;

    result = sflash_handle->capabilities->action->status_register( sflash_handle, &mask, &data, QUERY_ACTION );
    if (result != SFLASH_MSG_OK)
    {
        WPRINT_SFLASH_INFO( ( "!!! [%s] - Query fail !!!\n", __FUNCTION__ ) );
    }
    else
    {
        for( i = 0; i < BLOCK_PRTOECT_MAX_ENUM; i++ )
        {
            if ( ( ( uint32_t* ) map_bit )[ i ] == ( data & mask ) )
            {
                *protect_level = i;
                return SFLASH_MSG_OK;;
            }
        }
    }
    return SFLASH_MSG_ERROR;
}

static int sflash_set_write_enable( sflash_handle_t* sflash_handle, onoff_action_t mode )
{
    int result;

    if ( mode == TURN_OFF )
    {
        result = sflash_generic_command( sflash_handle, SFLASH_WRITE_DISABLE, NULL );
    }
    else
    {
        result = sflash_generic_command( sflash_handle, SFLASH_WRITE_ENABLE, NULL );
    }
    result = sflash_wait_busy_done( sflash_handle );
    return result;
}

static int sflash_reset( sflash_handle_t* sflash_handle )
{
    int result;

    result = sflash_generic_command( sflash_handle, SFLASH_RESET_ENABLE, NULL );
    if ( result != SFLASH_MSG_OK )
    {
        return result;
    }
    result = sflash_generic_command( sflash_handle, SFLASH_RESET, NULL );
    return result;
}

static wiced_bool_t sflash_is_status_register_bit_set( sflash_handle_t* sflash_handle, uint32_t mask_bit )
{
    int result;
    uint32_t data = 0;
    uint32_t mask = mask_bit;

    result = sflash_handle->capabilities->action->status_register( sflash_handle, &mask, &data, QUERY_ACTION );
    if ( result != SFLASH_MSG_OK )
    {
        WPRINT_SFLASH_INFO( ( "!!! [%s] - Query fail!\n", __FUNCTION__ ) );
    }
    else
    {
        if ( ( data & mask ) != 0 )
        {
            return WICED_TRUE;
        }
    }
    return WICED_FALSE;
}

static int sflash_wait_busy_done( sflash_handle_t* sflash_handle )
{
    uint32_t start_time, end_time;
    start_time = host_rtos_get_time();
    while ( sflash_handle->capabilities->action->is_busy( sflash_handle ) == WICED_TRUE )
    {
        end_time = host_rtos_get_time();
        if ( end_time - start_time > MAX_TIMEOUT_FOR_FLASH_BUSY )
        {
            WPRINT_SFLASH_INFO( ( "Sflash always busy over %ld ms ! ( default : %d ms)\n", ( end_time - start_time ), MAX_TIMEOUT_FOR_FLASH_BUSY ) );
            return SFLASH_MSG_ERROR;
        }
    }
    return SFLASH_MSG_OK;
}

static uint32_t sflash_get_command_data_length( sflash_command_t command )
{
    if ( ( command == SFLASH_WRITE_STATUS_REGISTER ) || ( command == SFLASH_READ_STATUS_REGISTER) ||
         ( command == SFLASH_READ_STATUS_REGISTER2) || ( command == SFLASH_WRITE_ENH_VOLATILE_REGISTER ) ||
         ( command == SFLASH_READ_ENH_VOLATILE_REGISTER ) )
    {
        return 1;
    }
    else if ( command == SFLASH_READID_JEDEC_ID )
    {
        return 4;
    }
    return 0;
}

static void sflash_deinit_by_stage( sflash_handle_t* sflash_handle, int8_t init_stage )
{
    switch ( init_stage )
    {
        case WICED_SFLASH_INIT_STAGE_CLEAR_ALL:
        case WICED_SFLASH_INIT_STAGE_2:
            /* clear sflash_handle */
            WPRINT_SFLASH_INFO(("de-initialize sflash_handle\n"));
            memset( sflash_handle, 0, sizeof( sflash_handle_t ) );
        case WICED_SFLASH_INIT_STAGE_1:
            /* de-initialize HW */
            WPRINT_SFLASH_INFO(("de-initialize HW\n"));
            spi_layer_deinit();
            break;
        default:
            /* de-initialize sflash with wrong init stage */
            WPRINT_SFLASH_INFO(("de-initialize sflash with wrong init stage! (%d)\n", init_stage));
            break;
    }
}

/*********************************
 * Sflash driver public function *
 *********************************/
int bcm4390x_sflash_init( sflash_handle_t* sflash_handle, void* peripheral_id, sflash_write_allowed_t write_allowed_in )
{
    int result;
    uint32_t sflash_status_id = 0;

    UNUSED_PARAMETER( peripheral_id );
    UNUSED_PARAMETER( write_allowed_in );

    /* Init stage 0, Initialize HW */
    spi_layer_init();

    /* Init stage 1, Read Sflash ID and Get sflash capability */
    if ( ( ( result = sflash_read_id( sflash_handle, &sflash_status_id ) ) != SFLASH_MSG_OK ) ||
            ( ( result = sflash_get_capability( sflash_handle, sflash_status_id ) ) != SFLASH_MSG_OK ) )
    {
        WPRINT_SFLASH_INFO( ( "!!! [%s] - Get Sflash ID fail!!!\n", __FUNCTION__ ) );
        WPRINT_SFLASH_INFO( ( "!!! Waring, Not found Sflash (0x%lx) in support list !!! \n", sflash_status_id ) );
        /* Initialize sflash fail, reset :
         * 1. HW */
        sflash_deinit_by_stage( sflash_handle, WICED_SFLASH_INIT_STAGE_1);
        goto init_fail;
    }

    /* Init stage 2, If Sflash support Quad mode, then enable it.*/
    if ( sflash_handle->capabilities->speed_advance->high_speed_read_dummy_cycle > 0 )
    {
        result = sflash_handle->capabilities->action->set_quad_mode( sflash_handle, TURN_ON );
        if (result != SFLASH_MSG_OK)
        {
            WPRINT_SFLASH_INFO( ( "!!! [%s] - enable Quad mode, fail!!!\n", __FUNCTION__ ) );
            /* Initialize sflash fail, reset :
             * 1. HW
             * 2. sflash_handle */
            sflash_deinit_by_stage( sflash_handle, WICED_SFLASH_INIT_STAGE_2);
            goto init_fail;
        }
    }

init_fail :
    return result;
}

int bcm4390x_sflash_deinit( sflash_handle_t* sflash_handle )
{
    sflash_deinit_by_stage( sflash_handle, WICED_SFLASH_INIT_STAGE_CLEAR_ALL );

    /* To be implement... */
    return SFLASH_MSG_OK;
}

int bcm4390x_sflash_read_data( sflash_handle_t* sflash_handle, uint32_t device_address, void* rx_data, uint32_t data_len, read_data_mode_t mode )
{
    int result = SFLASH_MSG_OK;
    char* rx_data_ptr = ( char* ) rx_data;
    sflash_command_t command;

    switch ( mode )
    {
        case AUTO_SPEED_READ_DATA :
            if ( sflash_handle->capabilities->speed_advance->high_speed_read_dummy_cycle > 0 )
            {
                command = ( sflash_handle->capabilities->speed_advance->high_speed_read_x4io_support == WICED_TRUE ? SFLASH_X4IO_READ : SFLASH_QUAD_READ );
            }
            else if ( sflash_handle->capabilities->speed_advance->fast_read_dummy_cycle > 0 )
            {
                command = SFLASH_FAST_READ;
            }
            else
            {
                command = SFLASH_READ;
            }
            WPRINT_SFLASH_INFO( ( "Speed : AUTO_SPEED_READ_DATA (0x%x)\n", command ) );
            break;
        case HIGH_SPEED_READ_DATA :
            command = ( sflash_handle->capabilities->speed_advance->high_speed_read_x4io_support == WICED_TRUE ? SFLASH_X4IO_READ : SFLASH_QUAD_READ );
            WPRINT_SFLASH_INFO( ( "Speed : HIGH_SPEED_READ_DATA (0x%x)\n", command ) );
            break;
        case FAST_READ_DATA :
            command = SFLASH_FAST_READ;
            WPRINT_SFLASH_INFO( ( "Speed : FAST_READ_DATA (0x%x)\n", command ) );
            break;
        case NORMAL_READ_DATA :
            command = SFLASH_READ;
            WPRINT_SFLASH_INFO( ( "Speed : NORMAL_READ_DATA (0x%x)\n", command ) );
            break;
        default :
            WPRINT_SFLASH_INFO( ( "!!! Wrong READ SPEED mode !!!\n" ) );
            return SFLASH_MSG_ERROR;
            break;
    };

    while ( data_len > 0 )
    {
        uint32_t read_size = data_len;

        result = spi_sflash_send_command( sflash_handle, command, device_address, rx_data_ptr, &read_size );

        if ( result != SFLASH_MSG_OK )
        {
            //exit the while loop when encounter error.
            break;
        }

        data_len -= read_size;
        rx_data_ptr += read_size;
        device_address += read_size;
        WPRINT_SFLASH_INFO( ( "done!\n" ) );
    }

    return result;
}

int bcm4390x_sflash_write_data( sflash_handle_t* sflash_handle, uint32_t device_address, void* tx_data, uint32_t data_len, write_data_mode_t mode )
{
    int result = SFLASH_MSG_OK;
    char* tx_data_ptr = ( char* ) tx_data;
    sflash_command_t command;

    if ( sflash_handle->write_allowed != SFLASH_WRITE_ALLOWED )
    {
        WPRINT_SFLASH_INFO( ( "!!! Warning, You do not have permission to write data. (%s) !!!\n", __FUNCTION__ ) );
        return SFLASH_MSG_ERROR;
    }

    switch ( mode )
    {
        case AUTO_SPEED_WRITE_DATA :
            if ( sflash_handle->capabilities->speed_advance->high_speed_write_support == WICED_TRUE )
            {
                command = ( sflash_handle->capabilities->speed_advance->high_speed_write_x4io_support == WICED_TRUE ? SFLASH_X4IO_WRITE : SFLASH_QUAD_WRITE );
            }
            else
            {
                command = SFLASH_WRITE;
            }
            WPRINT_SFLASH_INFO( ( "Speed : AUTO_SPEED_WRITE_DATA (0x%x)\n", command ) );
            break;
        case HIGH_SPEED_WRITE_DATA :
            command = ( sflash_handle->capabilities->speed_advance->high_speed_write_x4io_support == WICED_TRUE ? SFLASH_X4IO_WRITE : SFLASH_QUAD_WRITE );
            WPRINT_SFLASH_INFO( ( "Speed : HIGH_SPEED_WRITE_DATA (0x%x)\n", command ) );
            break;
        case NORMAL_WRITE_DATA :
            command = SFLASH_WRITE;
            WPRINT_SFLASH_INFO( ( "Speed : NORMAL_WRITE_DATA (0x%x)\n", command ) );
            break;
        default :
            WPRINT_SFLASH_INFO( ( "!!! Wrong WRITE SPEED mode !!!\n" ) );
            return SFLASH_MSG_ERROR;
            break;
    };

    while ( data_len > 0)
    {
        uint32_t max_page_size = sflash_handle->capabilities->max_page_size;
        uint32_t write_size;

        write_size = ( data_len >= max_page_size )? max_page_size : data_len;
        /* All transmitted data must not go beyond the end of the current page in a write */
        write_size = MIN( max_page_size - ( device_address % max_page_size ), write_size );

        result = sflash_handle->capabilities->action->set_write_enable( sflash_handle, TURN_ON );
        if (result != SFLASH_MSG_OK)
        {
            //exit the while loop when encounter error.
            break;
        }
        result = spi_sflash_send_command( sflash_handle, command, device_address, tx_data_ptr, &write_size );
        if (result != SFLASH_MSG_OK)
        {
            //exit the while loop when encounter error.
            break;
        }
        result = sflash_wait_busy_done( sflash_handle );
        if (result != SFLASH_MSG_OK)
        {
            //exit the while loop when encounter error.
            break;
        }

        data_len -= write_size;
        tx_data_ptr += write_size;
        device_address += write_size;

        WPRINT_SFLASH_INFO( ( "done!\n" ) );
    }

    return result;
}

int bcm4390x_sflash_erase_data( sflash_handle_t* sflash_handle, uint32_t device_address, erase_data_mode_t mode )
{
    int result = SFLASH_MSG_OK;
    uint32_t data_length = 0;
    sflash_command_t command;

    if ( sflash_handle->write_allowed != SFLASH_WRITE_ALLOWED )
    {
        WPRINT_SFLASH_INFO( ( "!!! Warning, You do not have permission to erase data. (%s) !!!\n", __FUNCTION__ ) );
        return SFLASH_MSG_ERROR;
    }

    switch ( mode )
    {
        case ERASE_ALL :
            command = SFLASH_CHIP_ERASE;
            WPRINT_SFLASH_INFO( ( "ERASE : all chip erase !\n" ) );
            break;
        case ERASE_BLOCK :
            command = SFLASH_64K_ERASE;
            WPRINT_SFLASH_INFO( ( "ERASE : erase 64KB on address(0x%lx) !\n", device_address ) );
            break;
        case ERASE_SECTOR :
            command = SFLASH_4K_ERASE;
            WPRINT_SFLASH_INFO( ( "ERASE : erase 4KB on address(0x%lx) !\n", device_address ) );
            break;
        default :
            WPRINT_SFLASH_INFO( ( "!!! Wrong ERASE mode !!!\n" ) );
            return SFLASH_MSG_ERROR;
            break;
    };

    result = sflash_handle->capabilities->action->set_write_enable( sflash_handle, TURN_ON );
    if (result != SFLASH_MSG_OK)
    {
        goto erase_fail;
    }
    if ( mode == ERASE_ALL )
    {
        result = sflash_generic_command( sflash_handle, command, NULL );
    }
    else
    {
        result = spi_sflash_send_command( sflash_handle, command, device_address, NULL, &data_length );
    }
    if (result != SFLASH_MSG_OK)
    {
        goto erase_fail;
    }
    result = sflash_wait_busy_done( sflash_handle );
    if (result != SFLASH_MSG_OK)
    {
        goto erase_fail;
    }
    WPRINT_SFLASH_INFO( ( "done!\n" ) );

erase_fail:

    return result;
}

