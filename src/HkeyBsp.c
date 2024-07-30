/*****************************************************************************
Copyright (C) 2017-2024, U-Tec.com. All rights reserved
File name: HkeyBsp.c
Description: write here the file function
Author: Joe<joe@u-tec.com>
Date: 04/05/24 11:00:16
*****************************************************************************/

#include <stdint.h>
#include <phDriver.h>
#include "phApp_Init.h"
#include <unistd.h>

#include <ph_Status.h>
#include <phhalHw.h>
#include <phpalI14443p3a.h>
#include <ph_RefDefs.h>
#include <phNxpNfcRdLib_Config.h>
//linux 用反斜杠
#include "phpalI14443p3a_Sw_Int.h"
#include "HKeyBsp.h"
// #include "NxpNfcRdLib\comps\phpalI14443p3a\src\Sw\phpalI14443p3a_Sw_Int.h"
/*=============================================================================
 Defines
 =============================================================================
*/

/*=============================================================================
 Globals
 =============================================================================
*/

/*=============================================================================
 Externs
 =============================================================================
*/
extern void *pspalI14443p4;

/* =============================================================================
 HkeyBsp.c module APIs
 =============================================================================
*/
extern bool DetectIsCpuCard(void);
#define PH_ERR_PN5190_SUCCESS                                ((phStatus_t)0x0000U) 
#define PHHAL_HW_PN5190_INSTR_TRANSMIT_RF_DATA                              0x08U  
typedef struct
{
    uint8_t bCmd;               /**< In_Param - Command code*/
    uint8_t bQueue;             /**< In_Param - Flag to denote that Command is queuable*/
    uint8_t * pTxDataBuff;      /**< In_Param - Pointer to the Tx-Buffer*/
    uint16_t wTxDataLength;     /**< In_Param - No of Bytes in Tx-Buffer */
    uint8_t * pAddnData;        /**< In_Param - Points to additional data to be appended to Tx_Buffer;
                                     applicable for some commands like write eeprom, Exchange_RF_Data*/
    uint16_t wAddnDataLen;      /**< In_Param - Length of the additional data */
    uint8_t ** ppRxBuffer;      /**< InOut_Param - Double pointer to send/retrieve, pointer to RxData in RxBuffer.
                                     For RF data and Read_EEprom response, the pointer to buffer in which data is
                                     to be sent is sent. */
    uint16_t * pRxLength;       /**< Out_Param - Pointer to Rx-length*/
}phhalHw_InstMngr_CmdParams_t;

#define SEND                    0x01U
#define QUEUE                   0x02U
#define QUEUE_N_SEND            0x03U
#define PHHAL_HW_PN5190_INSTR_EXCHANGE_RF_DATA                              0x0AU  /**< Read RX data from internal RF reception buffer. */
#define PHHAL_HW_PN5190_INT_SPI_WRITE                   0x7FU /**< Direction Bit0 -> 0 - Write; 1 - Read. */
phStatus_t phhalHw_Pn5190_Send(
    phhalHw_Pn5190_DataParams_t *pDataParams,       /**< [In] Pointer to Pn5190 Hal layer's parameter structure. */
    uint8_t * pTxBuffer,                            /**< [In] Pointer to Buffer containing data to be sent. */
    uint16_t wTxLength,                             /**< [In] Number of Byte to be sent from the Buffer. */
    uint8_t bCheckIRQ                               /**< [In] Check IRQ status before sending Command to Reader. */
    );

phStatus_t phhalHw_Pn5190_WaitIrq(
    phhalHw_Pn5190_DataParams_t * pDataParams,            /**<[In] DataParameter representing this layer. */
    uint32_t * pReceivedEvents                            /**<[InOut] Return received Events. */
    );
phStatus_t phOsal_EventClear(
    phOsal_Event_t * eventHandle,
    phOsal_EventOpt_t options,
    phOsal_EventBits_t FlagsToClear,
    phOsal_EventBits_t *pCurrFlags);

phStatus_t phhalHw_Pn5190_InstMngr_ReadRsp(
    phhalHw_Pn5190_DataParams_t *pDataParams,       /**< [In] Pointer to Pn5190 Hal layer's parameter structure. */
    phhalHw_InstMngr_CmdParams_t *pCmdParams        /**< [In] Pointer to Current Command Under-Process. */
    );


phStatus_t  phhalHw_Pn5190_InstMngr_Transceive_fast(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_InstMngr_CmdParams_t *pCmdParams, uint16_t wTlv_Length)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phOsal_EventBits_t PH_MEMLOC_REM dwReceivedEvents;
    uint8_t     PH_MEMLOC_REM bAbortCMDExe = 0;

    uint16_t    PH_MEMLOC_REM wBuffrdDataLength = 0U;
    uint16_t    PH_MEMLOC_REM wBufferSize = 0U;
    uint8_t     PH_MEMLOC_REM * pTxBuffer = NULL;

    pTxBuffer = pDataParams->pTxBuffer;
    wBufferSize = pDataParams->wTxBufSize;
      

    /* Return stored length. */
    wBuffrdDataLength = pDataParams->wTxBufLen;


    pTxBuffer--;
    *pTxBuffer = PHHAL_HW_PN5190_INT_SPI_WRITE;

    phhalHw_Pn5190_Send(pDataParams, pTxBuffer, (wBuffrdDataLength + 1), PH_ON);
        //printf("Sbuf\n");
    // DEBUG_PRINTF("phhalHw_Pn5190_InstMngr_Transceive_fast 222222222222222");



    // status = phhalHw_Pn5190_InstMngr_SendBuffdData(pDataParams);

    /*Reset the TxBuffer*/
    pDataParams->wTxBufLen = 0U;
    pDataParams->wTxBufStartPos = 0U;

    status = phhalHw_Pn5190_WaitIrq(pDataParams, &dwReceivedEvents);
    phOsal_EventClear(&pDataParams->HwEventObj.EventHandle, E_OS_EVENT_OPT_NONE, E_PH_OSAL_EVT_RF, NULL);
    status = phhalHw_Pn5190_InstMngr_ReadRsp(pDataParams, pCmdParams);
    return status;
}


phStatus_t phhalHw_Pn5190_InstMngr_Exchange_fast(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_InstMngr_CmdParams_t *pCmdParams, uint8_t bExchAction)
{

    phStatus_t  PH_MEMLOC_REM  status = PH_ERR_SUCCESS;
    uint16_t    PH_MEMLOC_REM  wSnglTLVLenght = 0U;
    uint16_t  PH_MEMLOC_REM  wTxBufferLen = 0U;
    uint16_t  PH_MEMLOC_REM  wTxBufferSize = 0U;
    uint8_t   PH_MEMLOC_REM  * pTxBuffer = NULL;

    uint8_t   PH_MEMLOC_REM  bTxBuffOffset = 0U;
    uint8_t   PH_MEMLOC_REM  bCmd = pCmdParams->bCmd;
    uint8_t   PH_MEMLOC_REM * pData = pCmdParams->pTxDataBuff;
    uint16_t  PH_MEMLOC_REM  wDataLength = pCmdParams->wTxDataLength;

   

    pTxBuffer = pDataParams->pTxBuffer;
    wTxBufferSize = pDataParams->wTxBufSize;
      

    /* Return stored length. */
    wTxBufferLen = pDataParams->wTxBufLen;

    pTxBuffer[pDataParams->wTxBufStartPos++] = bCmd; /* Reserved 0th Location of buffer for Command code */

    bTxBuffOffset = 2;

    //DEBUG_PRINTF("phhalHw_Pn5190_InstMngr_Exchange_fast 444444444444");
    (void)memcpy(&pTxBuffer[pDataParams->wTxBufStartPos + bTxBuffOffset],
                pCmdParams->pAddnData, pCmdParams->wAddnDataLen);

    pDataParams->wTxFwDataLen =  pCmdParams->wAddnDataLen;

    //DEBUG_PRINTF("phhalHw_Pn5190_InstMngr_Exchange_fast 5555555555555");
    (void)memcpy(&pTxBuffer[pDataParams->wTxBufStartPos + pDataParams->wTxBufLen
                            + pDataParams->wTxFwDataLen + bTxBuffOffset],
                pData, wDataLength);

    pDataParams->wTxBufLen += wDataLength;
        

    pDataParams->wTxBufLen += pDataParams->wTxFwDataLen;
    /*In the last we are updating the length*/
    pTxBuffer[pDataParams->wTxBufStartPos++] = ((pDataParams->wTxBufLen & 0xff00U) >> 8);

    pTxBuffer[pDataParams->wTxBufStartPos++] = (pDataParams->wTxBufLen & 0x00ffU);

    /* this is the last packet to add for the RF_EXchange the*/
    pDataParams->wTxBufLen += pDataParams->wTxBufStartPos;

    pDataParams->wTxFwDataLen = 0;

    // DEBUG_PRINTF("phhalHw_Pn5190_InstMngr_Exchange_fast 3333");
    status = phhalHw_Pn5190_InstMngr_Transceive_fast(pDataParams, pCmdParams, wSnglTLVLenght);

    return status;
}

phStatus_t phhalHw_Pn5190_Instr_TransmitRFData_fast(
    phhalHw_Pn5190_DataParams_t * pDataParams,
    uint16_t wOption,
    uint8_t bTxLastBits,
    uint8_t * pTxBuffer,
    uint16_t wTxBufferLength
    )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_PN5190_SUCCESS;
    phhalHw_InstMngr_CmdParams_t sCmdParams = {0U};
    uint8_t PH_MEMLOC_REM aAddnBuffer[2] = {0U};
    uint8_t PH_MEMLOC_REM bIndex = 0U;
    uint16_t   PH_MEMLOC_REM wRxLen = 0U;
    uint8_t *  PH_MEMLOC_REM pRxData = NULL;

  
    /* clear internal buffer if requested */
    pDataParams->wTxBufLen = 0U;
    pDataParams->wTxBufStartPos = 0U;
    pDataParams->wTxFwDataLen = 0U;

    pDataParams->bNonRF_Cmd = 0U;
    

    aAddnBuffer[bIndex++] = bTxLastBits;
    aAddnBuffer[bIndex++] = 0;

    sCmdParams.pAddnData = &aAddnBuffer[0];
    sCmdParams.wAddnDataLen = bIndex;

    /* Command Type */
    sCmdParams.bCmd = PHHAL_HW_PN5190_INSTR_TRANSMIT_RF_DATA;

    sCmdParams.bQueue = false;

    sCmdParams.pTxDataBuff = pTxBuffer;
    sCmdParams.wTxDataLength = wTxBufferLength;
    sCmdParams.pRxLength = &wRxLen;
    sCmdParams.ppRxBuffer = &pRxData;

    /* Send command */

    status = phhalHw_Pn5190_InstMngr_Exchange_fast(pDataParams, &sCmdParams, QUEUE_N_SEND);


    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}
#define PH_ERR_PN5190_TX_FAILURE                                           ((phStatus_t)0x1AU)  

phStatus_t phhalHw_Pn5190_Transmit_fast(
                                   phhalHw_Pn5190_DataParams_t * pDataParams,
                                   uint16_t wOption,
                                   uint8_t * pTxBuffer,
                                   uint16_t wTxLength
                                   )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bNumberOfValidBits = 0U;
    uint8_t     PH_MEMLOC_REM bSyncByte = 0xf0;
 
    bNumberOfValidBits = (uint8_t)pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXLASTBITS];


    status =  phhalHw_Pn5190_Instr_TransmitRFData_fast(pDataParams, wOption, bNumberOfValidBits, pTxBuffer, wTxLength);
    /* Reset TxLastBits */
    if (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXLASTBITS] != 0)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, PHHAL_HW_CONFIG_TXLASTBITS, 0x00));
    }


    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

static phStatus_t phpalI14443p3a_ECP(phpalI14443p3a_Sw_DataParams_t *pDataParams, uint8_t *ecp_data, int ecp_len)
{
    phStatus_t PH_MEMLOC_REM statusTmp;
    phStatus_t PH_MEMLOC_REM Status;
    uint8_t PH_MEMLOC_REM cmd[1] = {0};
    uint8_t *PH_MEMLOC_REM pResp = NULL;
    uint16_t PH_MEMLOC_REM wRespLength = 0;
    uint16_t PH_MEMLOC_REM wRegister;

    /* Apply Protocol Setting for Type A */
    statusTmp = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);
    CHECK_STATUS(statusTmp);

    /* RF Field ON */
    //DEBUG_PRINTF("phpalI14443p3a_ECP 1111");
    statusTmp = phhalHw_FieldOn(pHal);
    if (statusTmp != PH_ERR_SUCCESS)
    {
        statusTmp = phhalHw_Wait(pHal, PHHAL_HW_TIME_MILLISECONDS, 100);
    }
    //获取卡id
    // if(DetectIsCpuCard() == false)
    // {
    //     return PH_ERR_UNKNOWN;
    // }
    //DEBUG_PRINTF("phpalI14443p3a_ECP 2222");
    /* Disable MIFARE Classic contactless IC Crypto1 */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
                                        pDataParams->pHalDataParams,
                                        PHHAL_HW_CONFIG_DISABLE_MF_CRYPTO1,
                                        PH_ON));

    /* Reset default data rates */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
                                        pDataParams->pHalDataParams,
                                        PHHAL_HW_CONFIG_TXDATARATE_FRAMING,
                                        PHHAL_HW_RF_DATARATE_106));
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
                                        pDataParams->pHalDataParams,
                                        PHHAL_HW_CONFIG_RXDATARATE_FRAMING,
                                        PHHAL_HW_RF_DATARATE_106));

    /* Set selection timeout */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
                                        pDataParams->pHalDataParams,
                                        PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,
                                        PHPAL_I14443P3A_SELECTION_TIME_US + PHPAL_I14443P3A_EXT_TIME_US));

    /* Switch off CRC */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_TXCRC, PH_ON));
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RXCRC, PH_ON));

    /* Only 7 bits are valid */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_TXLASTBITS, 7));
    // printf("debug at %s %d\n",__FILE__,__LINE__);
    for (int i = 0; i < 3; i++)
    {   
        //DEBUG_PRINTF("phpalI14443p3a_ECP 3333 %d", i);
        statusTmp = phhalHw_Pn5190_Transmit_fast(pDataParams->pHalDataParams, PH_TRANSMIT_DEFAULT, ecp_data, ecp_len);
        //phhalHw_Wait(pHal, PHHAL_HW_TIME_MILLISECONDS, 1);
        // DEBUG_PRINTF(" statusTmp %x", statusTmp);
    }

    return PH_ERR_SUCCESS;
}
/**
 * @brief exchange in 14443a
 *
 * @param send
 * @param send_len
 * @param recBuffer
 * @param receiveLen
 * @param timeout
 * @return int
 */
int hkey_bsp_exchange(const uint8_t *send, uint16_t send_len, uint8_t *recBuffer, uint16_t *receiveLen)
{
    uint8_t *pRxData;
    uint16_t wRxDataLength;
    // ESP_LOG_BUFFER_HEX_LEVEL("HKEY send", send, send_len, ESP_LOG_INFO);
    phStatus_t wStatus = phpalI14443p4_Exchange(
        pspalI14443p4,
        PH_EXCHANGE_DEFAULT,
        send,
        send_len,
        &pRxData,
        &wRxDataLength);
     //DEBUG_PRINTF("HKEY status %x\n", wStatus);
    // ESP_LOG_BUFFER_HEX_LEVEL("HKEY ACK", pRxData, wRxDataLength, ESP_LOG_INFO);
    
    memcpy(recBuffer, pRxData, wRxDataLength);
    *receiveLen = wRxDataLength;
    return wStatus == PH_ERR_SUCCESS ? 0 : -1;
}

extern phStatus_t DetectTypeA(void);
extern phStatus_t DetectCpuTypeA2(void);
extern bool DetectCpuTypeA(void);

/**
 * @brief Detect type1 card 
 *
 * @param ecp_data
 * @param ecp_len
 * @return int
 */
int hkey_bsp_send_ecp(uint8_t *ecp_data, int ecp_len)
{
    phStatus_t wStatus = PH_ERR_UNKNOWN;
    //DEBUG_PRINTF("HKEY poll");
    wStatus = phpalI14443p3a_ECP(pspalI14443p4, ecp_data, ecp_len);
    if(wStatus == PH_ERR_UNKNOWN)
    {
        DEBUG_PRINTF("PH_ERR_UNKNOWN");
        return PH_ERR_UNKNOWN;
    }
#if 1
    // it takes time for phone wakeup
    // phhalHw_Wait(pHal, PHHAL_HW_TIME_MILLISECONDS, 200);
    // check phone is wakeup and select
   // DEBUG_PRINTF("HKEY type 11");
    //判断是不是cpu卡
    bool CardStatus = DetectCpuTypeA2();
    if(CardStatus == false)
    {
        wStatus = PH_ERR_UNKNOWN;
    }
    else
    {
        wStatus = PH_ERR_SUCCESS;
    }
#endif
    //DEBUG_PRINTF("wStatus");
    return (wStatus == PH_ERR_SUCCESS) ? 0 : -1;
}
