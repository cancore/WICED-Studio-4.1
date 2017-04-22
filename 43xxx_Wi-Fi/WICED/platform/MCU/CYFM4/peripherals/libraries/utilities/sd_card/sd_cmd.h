/*************************************************************************************
* Copyright (C) 2013-2015, Cypress Semiconductor Corporation. All rights reserved.    
*                                                                                     
* This software, including source code, documentation and related                     
* materials ( "Software" ), is owned by Cypress Semiconductor                         
* Corporation ( "Cypress" ) and is protected by and subject to worldwide              
* patent protection (United States and foreign), United States copyright              
* laws and international treaty provisions. Therefore, you may use this               
* Software only as provided in the license agreement accompanying the                 
* software package from which you obtained this Software ( "EULA" ).                  
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,             
* non-transferable license to copy, modify, and compile the                           
* Software source code solely for use in connection with Cypress's                    
* integrated circuit products. Any reproduction, modification, translation,           
* compilation, or representation of this Software except as specified                 
* above is prohibited without the express written permission of Cypress.              
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO                                
* WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING,                                
* BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED                                        
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A                                     
* PARTICULAR PURPOSE. Cypress reserves the right to make                              
* changes to the Software without notice. Cypress does not assume any                 
* liability arising out of the application or use of the Software or any              
* product or circuit described in the Software. Cypress does not                      
* authorize its products for use in any products where a malfunction or               
* failure of the Cypress product may reasonably be expected to result in              
* significant property damage, injury or death ( "High Risk Product" ). By            
* including Cypress's product in a High Risk Product, the manufacturer                
* of such system or application assumes all risk of such use and in doing             
* so agrees to indemnify Cypress against all liability.                               
*/
/******************************************************************************/
/** \file sd_cmd.h
 **
 ** Headerfile for SD functions
 **
 ** History:
 **   - 2013-05-16  1.0  QXu  First version.
 **   - 2013-07-24  1.1  RQian  Modification.
 **   - 2015-01-28  1.3  RZh    Modify for FM universal PDL
 **
 ******************************************************************************/

#ifndef __SD_CMD_H__
#define __SD_CMD_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "sdif/sdif.h"

#if (defined(PDL_PERIPHERAL_SD_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \addtogroup SdGroup
 ******************************************************************************/
//@{

/******************************************************************************
 * Global definitions
 ******************************************************************************/
#define SDIF                (&SDIF0)

#define BLOCK_SZ            (512)
#define RETRY_TIME          (1000)


#define SD_ACMD_OFFSET      (0x40)
#define MMC_CMD_TAG         (0x80)
#define ARG_ACMD41_BUSY     (0x80000000)       
#define OCR_CAPACITY_MASK   (0x40000000)


#define SD_CMD0             (0)
#define SD_CMD1             (1)
#define SD_CMD2             (2)
#define SD_CMD3             (3)
#define SD_CMD6             (6)

#define SD_CMD7             (7)
#define SD_CMD8             (8)
#define SD_CMD9             (9)
#define SD_CMD10            (10)

#define SD_CMD12            (12)
#define SD_CMD13            (13)

#define SD_CMD16            (16)
#define SD_CMD17            (17)
#define SD_CMD18            (18)

#define SD_CMD24            (24)
#define SD_CMD25            (25)

#define SD_CMD28            (28)
#define SD_CMD29            (29)
#define SD_CMD30            (30)

#define SD_CMD32            (32)
#define SD_CMD33            (33)
#define SD_CMD38            (38)


#define SD_CMD42            (42)
#define SD_CMD52            (52)
#define SD_CMD53            (53)
#define SD_CMD55            (55)

#define MMC_CMD8            (8 |MMC_CMD_TAG)

#define SD_ACMD6            (SD_ACMD_OFFSET + 6)
#define SD_ACMD13           (SD_ACMD_OFFSET + 13)
#define SD_ACMD41           (SD_ACMD_OFFSET + 41)
#define SD_ACMD51           (SD_ACMD_OFFSET + 51)


#define ACMD41_HCS          (1<<30)
#define ACMD41_VOLTAGE      (0x00FF8000)


typedef void func_ptr_evt_cbk( void * );

/**
 ******************************************************************************
 ** \brief SD output clock enum
 ******************************************************************************/
typedef enum _en_sdcmd_clk
{
    SdClk400K    = (400 * 1000),
    SdClk20M     = (20 * 1000 * 1000),
    SdClk25M     = (25 * 1000 * 1000),
    SdClk40M     = (40 * 1000 * 1000),
    SdClk50M     = (50 * 1000 * 1000),
}en_sdcmd_clk_t;

/**
 ******************************************************************************
 ** \brief SD card type
 ******************************************************************************/
typedef enum
{
    SdCardCapacityStand    = 0u,
    SdCardCapacityHigh     = 1u,
}en_sdcmd_cardcapacity_t;

/**
 ******************************************************************************
 ** \brief SD card line type enum
 ******************************************************************************/
typedef enum
{
    enSdcmdCmdLine      = 1u,
    enSdcmdDataLine     = 2u,

}en_sdcmd_regtype_t;

/**
 ******************************************************************************
 ** \brief SD command data I/O direction.
 ******************************************************************************/
typedef enum
{
    SdcmdDirWrite         = 0u,         
    SdcmdDirRead          = 1u,        
}en_sdcmd_rwdir_t;

/**
 ******************************************************************************
 ** \brief SD card response type.
 ******************************************************************************/
typedef enum _en_sdcmd_response
{
    SdCmdRspNone                = 0u,         
    SdCmdRspR2                  = 1u,
    SdCmdRspR3R4                = 2u,
    SdCmdRspR1NORMALR5R6R7      = 3u,
    SdCmdRspR1AUTO              = 4u,
    SdCmdRspR1BNORMAL           = 5u,
    SdCmdRspR1BAUTO             = 6u,
    SdCmdRspR5B                 = 7u,
}en_sdcmd_response_t;

/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO card status
 ******************************************************************************/
typedef enum _en_sdcmd_cardstatus
{
    SdCardStateIdle     = 0u,
    SdCardStateReady    = 1u,
    SdCardStateIdent    = 2u,
    SdCardStateStby     = 3u,
    SdCardStateTran     = 4u,
    SdCardStateData     = 5u,
    SdCardStateRcv      = 6u,
    SdCardStatePrg      = 7u,
    SdCardStateDis      = 8u,
    SdCardStateRev      = 9u,
}en_sdcmd_cardstatus_t;

/**
 ******************************************************************************
 ** \brief SD interrupt event enum
 ******************************************************************************/
typedef enum _en_sdcmd_event
{
    SdCmdEventCmdComplete       = 1u,          
    SdCmdEventTransferDone      = 2u,
    SdCmdEventReadRdy           = 3u,
    SdCmdEventWrRdy             = 4u,
    SdCmdEventAdmaComplete      = 5u,
}en_sdcmd_event_t;

/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO bus width mode
 ******************************************************************************/
typedef enum en_sdcmd_buswidth
{
    SdBusWidth1     = 0u,
    SdBusWidth4     = 1u,
    SdBusWidth8     = 2u,
}en_sdcmd_buswidth_t;

/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO command configuration structure
 ******************************************************************************/
typedef struct _stc_sdcmd_config
{
    en_sdcmd_buswidth_t     enBusWidth;        
    uint32_t                u32Clock;          
    boolean_t               bHighSpeedMode;     
    boolean_t               bUsingADMA;        
}stc_sdcmd_config_t;

/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO ADMA description table structure.
 ******************************************************************************/
typedef struct
{
    uint16_t u16AttrValid   :1;
    uint16_t u16AttrEnd     :1;
    uint16_t u16AttrInt     :1;
    uint16_t u16Reserved1   :1;
    uint16_t u16Act         :2;
    uint16_t u16Reserved2   :10;
    uint16_t u16Len;
    uint32_t u32addr;
} stc_sdcmd_adma_desc_tbl_t;

/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO event flag count
 ******************************************************************************/
typedef struct _stc_sdcmd_event_flag
{
    uint8_t u8BuffReadReady;
    uint8_t u8BuffWriteReady;
    uint8_t u8CmdComplete;
    uint8_t u8TransComplete;
    uint8_t u8ADMAComplete;
    uint8_t u8CardInsert;
    uint8_t u8CardRemove;

    uint8_t u8CmdTimeOut;

    uint8_t u8CRCError;
    uint8_t u8TranTimeOut;
    uint8_t u8DataComplete;
    uint8_t u8TxTimeOut;
    uint8_t u8RxTimeOut;
}stc_sdcmd_event_flag_t;

/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO command data structure.
 ******************************************************************************/
typedef struct _stc_sdcmd_cmd
{
    uint32_t                u32CmdIdx;
    uint32_t                u32Arg;
    uint32_t                u32Rsp[4];
    en_sdcmd_response_t     enRspType;
    uint8_t                 *pu8Data;
    uint8_t                 u8Dir;
    uint32_t                u32BlockCnt;
    uint32_t                u32BlockSize;
    stc_sdcmd_event_flag_t  stcEvent;
}stc_sdcmd_cmd_t;

typedef struct _stc_sdcmd_scr
{
    uint32_t    CMD_SUPPORT             :2;
    uint32_t    RESERVE1                :9;
    uint32_t    EX_SECURITY             :4;
    uint32_t    SD_SPEC3                :1;
    uint32_t    SD_BUSWIDTHS            :4;
    uint32_t    SD_SECURITY             :3;
    uint32_t    DATA_STAT_AFTER_ERASE   :1;
    uint32_t    SD_SPEC                 :4;
    uint32_t    SCR_STCUCTURE           :4;

    uint32_t    RESERVE2; //swaped.
}stc_sdcmd_scr_t;


/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO card response data structure.
 ******************************************************************************/
typedef struct _stc_sdcmd_cardstatus
{
    uint32_t    RESERVE1            :3;
    uint32_t    AKE_SEQ_ERROR       :1;
    uint32_t    RESERVE2            :1;
    uint32_t    APP_CMD             :1;
    uint32_t    RESERVE3            :2;
    uint32_t    READYFORDATA        :1;
    uint32_t    CURRENT_STATE       :4;

    uint32_t    ERASE_RESET         :1;
    uint32_t    CARD_ECC_DISABLE    :1;
    uint32_t    WP_ERASE_SKIP       :1;
    uint32_t    CSD_OVERWRITE       :1;
    uint32_t    RESERVE4            :2;
    uint32_t    ERRORR              :1;
    uint32_t    CC_ERROR            :1;
    uint32_t    CARD_ECC_FAIL       :1;
    uint32_t    ILLEGAL_CMD         :1;
    uint32_t    COM_CRC_ERROR       :1;
    uint32_t    LOCK_UNLOCK_FAIL    :1;
    uint32_t    CARD_IS_LOCKED      :1;
    uint32_t    WP_VIOLATION        :1;
    uint32_t    ERASE_PARAM         :1;
    uint32_t    ERASE_SEQ_ERR       :1;
    uint32_t    BLOCK_LEN_ERR       :1;
    uint32_t    ADDRESS_ERROR       :1;
    uint32_t    OUT_OF_RANGE        :1;

}stc_sdcmd_cardstatus_t;

/**
 ******************************************************************************
 ** \brief CSD Register ver1.0(for Standard Capacity Card)
 ******************************************************************************/
typedef struct _stc_sdcmd_csd_v1
{
    uint8_t     RESERVE1                : 2;
    uint8_t     FILE_FORMAT             : 2;
    uint8_t     TMP_WRITE_PROTECT       : 1;
    uint8_t     PERM_WRITE_PROTECT      : 1;
    uint8_t     COPY                    : 1;
    uint8_t     FILE_FORMAT_GRP         : 1;

    uint8_t     RESERVE2                : 5;
    uint8_t     WRITE_BL_PARTIAL        : 1;
    uint8_t     WRITE_BL_LEN1           : 2;

    uint8_t     WRITE_BL_LEN2           : 2;
    uint8_t     R2W_FACTOR              : 3;
    uint8_t     RESERVE3                : 2;
    uint8_t     WP_GRP_ENABLE           : 1;

    uint8_t     WP_GRP_SIZE             : 7;
    uint8_t     SECTOR_SIZE1            : 1;

    uint8_t     SECTOR_SIZE2            : 6;
    uint8_t     ERASE_BLK_EN            : 1;
    uint8_t     C_SIZE_MULTI1           : 1;

    uint8_t     C_SIZE_MULTI2           : 2;
    uint8_t     CDD_W_CURR_MAX          : 3;
    uint8_t     CDD_W_CURR_MIN          : 3;

    uint8_t     VDD_R_CURR_MAX          : 3;
    uint8_t     VDD_R_CURR_MIN          : 3;
    uint8_t     C_SIZE1                 : 2;

    uint8_t     C_SIZE2                 : 8;

    uint8_t     C_SIZE3                 : 2;
    uint8_t     RESERVED4               : 2;
    uint8_t     DSR_IMP                 : 1;
    uint8_t     READ_BLK_MISALIGH       : 1;
    uint8_t     WRITE_BLK_MISALIGN      : 1;
    uint8_t     READ_BL_PARTIAL         : 1;

    uint8_t     READ_BL_LEN             : 4;
    uint8_t     CCC1                    : 4;

    uint8_t     CCC2                    : 8;

    uint8_t     TRAN_SPEED              : 8;

    uint8_t     NSAC                    : 8;

    uint8_t     TAAC                    : 8;

    uint8_t     RESERVE5                : 6;
    uint8_t     CSD_STRUCTURE           : 2;

}stc_sdcmd_csd_v1_t;

/**
 ******************************************************************************
 ** \brief CSD Register ver2.0(for High Capacity Card)
 ******************************************************************************/
typedef struct _stc_sdcard_csd_v2
{
    // Byte0
    uint8_t     RESERVED1               : 2;
    uint8_t     FILE_FORMAT             : 2;
    uint8_t     TMP_WRITE_PROTECT       : 1;
    uint8_t     PERM_WRITE_PROTECT      : 1;
    uint8_t     COPY                    : 1;
    uint8_t     FILE_FORMAT_GRP         : 1;

    uint8_t     RESERVED2               : 5;
    uint8_t     WRITE_BL_PARTIAL        : 1;
    uint8_t     WRITE_BL_LEN1           : 2;
    // Byte2
    uint8_t     WRITE_BL_LEN2           : 2;
    uint8_t     R2W_FACTOR              : 3;
    uint8_t     RESERVED3               : 2;
    uint8_t     WP_GRP_ENABLE           : 1;

    uint8_t     WP_GRP_SIZE             : 7;
    uint8_t     SECTOR_SIZE1            : 1;
    //byte 4
    uint8_t     SECTOR_SIZE2            : 6;
    uint8_t     ERASE_BLK_EN            : 1;
    uint8_t     RESERVED4               : 1;

    //byte 5
    uint8_t     C_SIZE1                 : 8;

    uint8_t     C_SIZE2                 : 8;

    uint8_t     C_SIZE3                 : 6;
    uint8_t     RESERVED5               : 2;

    //byte 8
    uint8_t     RESERVED6               : 4;
    uint8_t     DSR_IMP                 : 1;
    uint8_t     READ_BLK_MISALIGH       : 1;
    uint8_t     WRITE_BLK_MISALIGN      : 1;
    uint8_t     READ_BL_PARTIAL         : 1;

    //byte9
    uint8_t     READ_BL_LEN             : 4;
    uint8_t     CCC1                    : 4;

    //byte 10
    uint8_t     CCC2                    : 8;

    //byte 11
    uint8_t     TRAN_SPEED              : 8;
    uint8_t     NSAC                    : 8;
    uint8_t     TAAC                    : 8;

    uint8_t     RESERVED7               : 6;
    uint8_t     CSD_STRUCTURE           : 2;

} stc_sdcmd_csd_v2_t;


typedef struct _stc_sdcmd_ocr
{
    uint32_t u32Reserved1   :15;
    uint32_t u32V27_28      :1;
    uint32_t u32V28_29      :1;
    uint32_t u32V29_30      :1;
    uint32_t u32V30_31      :1;
    uint32_t u32V31_32      :1;
    uint32_t u32V32_33      :1;
    uint32_t u32V33_34      :1;
    uint32_t u32V34_35      :1;
    uint32_t u32V35_36      :1;
    uint32_t u32S18A        :1;
    uint32_t u32Reserved2   :5;
    uint32_t u32CCS         :1;
    uint32_t u32Busy        :1;
}stc_sdcmd_ocr_t;
/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO card infomation reading from card
 ******************************************************************************/
typedef struct _stc_sdcmd_info
{
    en_sdcmd_cardcapacity_t enCapacityType;           ///< High Or Standard.
    uint32_t                u32MaxSectorNum;          ///< maximum sector number of the SD card
    stc_sdcmd_ocr_t         stcOCR;                   ///< Operation Condition Register
    uint8_t                 CID[16];                  ///< Card IDentification (CID) register  CID[0-4]-- CID( 127:8)
    uint8_t                 CSD[16];                  ///< Card-Specific Data register CSD  CSD[0-4]--CSD (127:8)
    uint16_t                RCA;                      ///< Relative Card Address
} stc_sdcmd_info_t;

/**
 ******************************************************************************
 ** \brief SD/eMMC/SDIO configuration structure
 ******************************************************************************/
typedef struct _stc_sdcard_config
{
    en_sdcmd_buswidth_t     enBusWidth;                 ///< SD Bus Width
    uint32_t                u32Clock;                   ///< SD Clock line frequency
    boolean_t               bHighSpeedMode;             ///< High Speed Mode.
    boolean_t               bUsingADMA;                 ///< if using ADMA
    boolean_t               bDisableCardDetect;         ///< Disable Card detect
    stc_sdcmd_info_t        stcInfo;                    ///< Information from card.
    func_ptr_t              pfnCardIrqCb;               ///< Pointer to card interrupt callback function
    boolean_t               bTouchNvic;                 ///< TRUE: Enable NVIC, FALSE: Disable NVIC
    boolean_t               bNoCardDetectPin;           ///< TRUE: Card detect pin not connected
}stc_sdcard_config_t;


/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/******************************************************************************/
/* Global variable declarations ('extern', definition in C source)            */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
void usdelay( uint32_t u32Delay );

void SdcmdEventBufferReadRdyCbk( void );

void SdcmdEventBufferWriteRdyCbk( void );

void SdcmdEventCmdCompleteCbk( void );

void SdcmdEventTransCompleteCbk( void );

void SdcmdEventADMACompleteCbk( void );

void SdcmdEventCmdTimeOutCbk( void );

void SdcmdEventCardRemoveCbk( void );

void SdcmdEventCardInsertCbk( void );


en_result_t Sdcmd_SDChangeBusWidth( stc_sdcmd_info_t *pstcInfo,en_sdcmd_buswidth_t enWidth );

en_result_t Sdcmd_MMCChangeBusWidth( stc_sdcmd_info_t *pstcInfo, en_sdcmd_buswidth_t enWidth );

en_result_t SdCardChangeClock( uint32_t u32Freq );

en_result_t Sdcmd_SDEnableHighSpeed( boolean_t bEnable, uint32_t u32Clock );

en_result_t Sdcmd_MMCEnableHighSpeed( boolean_t bEnable, uint32_t u32Clock );

en_result_t Sdcmd_Ops_GoIdle( void );                                                       // CMD0

en_result_t Sdcmd_Ops_AllSendCid( stc_sdcmd_info_t *pstcInfo, uint32_t u32Arg );            // CMD2

en_result_t Sdcmd_Ops_SendRelativeAddr( stc_sdcmd_info_t *pstcInfo, uint32_t u32Arg );      // CMD3

en_result_t Sdcmd_Ops_SwitchFunc( uint32_t u32Arg );            // CMD6

en_result_t Sdcmd_Ops_SelectCard( uint32_t u32RCA );            // CMD7

en_result_t Sdcmd_Ops_SendIfCond( uint32_t u32Arg );            // CMD8

en_result_t Sdcmd_Ops_SendCSD( stc_sdcmd_info_t *pstcInfo );    // CMD9

en_result_t Sdcmd_Ops_SendCid( void );                          // CMD10

en_result_t Sdcmd_Ops_SwitchVol( void );                        // CMD11

en_result_t Sdcmd_Ops_StopTran( void );                         // CMD12

en_sdcmd_cardstatus_t Sdcmd_Ops_SendStatus( stc_sdcmd_info_t *pstcInfo ); // CMD13

en_result_t Sdcmd_Ops_ReadSingleBlock( stc_sdcmd_info_t *pstcInfo,
                                       uint32_t u32StartBlock,
                                       uint8_t *pu8BuffOut );   // CMD17

en_result_t Sdcmd_Ops_ReadMultiBlock( stc_sdcmd_info_t *pstcInfo,
                                      uint32_t u32StartBlock,
                                      uint32_t u32BlockCnt,
                                      uint8_t *pu8BuffOut );    // CMD18

en_result_t Sdcmd_Ops_WriteSingleBlock( stc_sdcmd_info_t *pstcInfo,
                                       uint32_t u32StartBlock,
                                       uint8_t *pu8BuffIn );   // CMD24

en_result_t Sdcmd_Ops_WriteMultiBlock( stc_sdcmd_info_t *pstcInfo,
                                       uint32_t u32StartBlock,
                                       uint32_t u32BlockCnt,
                                       uint8_t *pu8BuffIn );   // CMD25

en_result_t Sdcmd_Ops_EraseStartAddr( stc_sdcmd_info_t *pstcInfo,
                                      uint32_t u32StartBlock
                                     );

en_result_t Sdcmd_Ops_EraseEndAddr( stc_sdcmd_info_t *pstcInfo,
                                    uint32_t u32StartBlock
                                   );

en_result_t Sdcmd_Ops_Erase( stc_sdcmd_info_t *pstcInfo );


en_result_t Sdcmd_Ops_SetBusWdith( stc_sdcmd_info_t *pstcInfo, uint32_t u32Arg );                               // ACMD6

en_result_t Sdcmd_Ops_SendOpCond( stc_sdcmd_info_t *pstcInfo, uint32_t u32Arg );    // ACMD41

en_result_t Sdcmd_Ops_SdSendOpCond( stc_sdcmd_info_t *pstcInfo, uint32_t u32Arg );

en_result_t Sdcmd_Ops_SendSCR( stc_sdcmd_info_t *pstcInfo, uint32_t u32RCA );

en_result_t MMCcmd_Ops_SendOpCond( stc_sdcmd_info_t *pstcInfo, uint32_t u32Arg );

en_result_t MMCcmd_Ops_SendExtCsd( stc_sdcmd_info_t *pstcInfo );

en_result_t MMCcmd_Ops_CmdSet( uint32_t u32Arg );

en_result_t Sdcmd_Init( stc_sdcmd_config_t *pstcCfg );

en_result_t Sdcmd_SendCmd(stc_sdcmd_cmd_t *pstcCmd);



//@} // SdGroup
#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_SD_ACTIVE))

#endif /* __SD_CMD_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/

