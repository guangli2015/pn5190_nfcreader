/*----------------------------------------------------------------------------*/
/* Copyright 2016-2023 NXP                                                    */
/*                                                                            */
/* NXP Confidential. This software is owned or controlled by NXP and may only */
/* be used strictly in accordance with the applicable license terms.          */
/* By expressly accepting such terms or by downloading, installing,           */
/* activating and/or otherwise using the software, you are agreeing that you  */
/* have read, and that you agree to comply with and are bound by, such        */
/* license terms. If you do not agree to be bound by the applicable license   */
/* terms, then you may not retain, install, activate or otherwise use the     */
/* software.                                                                  */
/*----------------------------------------------------------------------------*/

/** \file
* Example Source for NfcrdlibEx1_DiscoveryLoop that uses the Discovery loop implementation.
* By default Discovery Loop will work as per NFC Forum Activity Specification v2.2
* which will configure the Reader in both POLL and LISTEN (only for Universal device)
* modes of discovery loop.Displays detected tag information(like UID, SAK, Product Type)
* and prints information when it gets activated as a target by an external Initiator/reader.
*
* By enabling "ENABLE_DISC_CONFIG" macro, few of the most common Discovery Loop configuration
* are been updated to values defined in this Example.
* By enabling "ENABLE_EMVCO_PROF", Discovery Loop will be configured as per EMVCo Polling
* specification else the Discovery Loop will still be configured to NFC Forum but user defined
* values as per this Application.
*
* NFC Forum Mode: Whenever multiple technologies are detected, example will select first
* detected technology to resolve. Example will activate device at index zero whenever multiple
* device is detected.
*
* For EMVCo profile, this example provide full EMVCo digital demonstration along with option to
* use different SELECT PPSE Commands.
*
* Please refer Readme.txt file for Hardware Pin Configuration, Software Configuration and steps to build and
* execute the project which is present in the same project directory.
*
* $Author$
* $Revision$ (v07.10.00)
* $Date$
*/
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <stdlib.h>
/**
* Reader Library Headers
*/
#include <phApp_Init.h>

/* Local headers */
#include <NfcrdlibEx1_DiscoveryLoop.h>
#include <NfcrdlibEx1_EmvcoProfile.h>
#include "HKeyBsp.h"
/*******************************************************************************
**   Definitions
*******************************************************************************/
#if 1
phacDiscLoop_Sw_DataParams_t       * pDiscLoop;       /* Discovery loop component */
phpalI14443p4_Sw_DataParams_t * ppalI14443p4;        /* PAL ISO I14443-4 component */
phpalI14443p4a_Sw_DataParams_t * ppalI14443p4a;      /* PAL ISO I14443-4A component */
phpalI14443p3a_Sw_DataParams_t * ppalI14443p3a;      /* PAL ISO I14443-A component */
phpalI14443p3b_Sw_DataParams_t * ppalI14443p3b;      /* PAL ISO I14443-B component */
phalTop_Sw_DataParams_t *       palTop;    
phpalFelica_Sw_DataParams_t * ppalFelica;            /* PAL Felica component */
phpalI14443p4mC_Sw_DataParams_t * ppalI14443p4mC;    /* PAL ISO I14443-4mC Target component */

/*The below variables needs to be initialized according to example requirements by a customer */
uint8_t  sens_res[2]     = {0x04, 0x00};              /* ATQ bytes - needed for anti-collision */
uint8_t  nfc_id1[3]      = {0xA1, 0xA2, 0xA3};        /* user defined bytes of the UID (one is hardcoded) - needed for anti-collision */
uint8_t  sel_res         = 0x40;
uint8_t  nfc_id3         = 0xFA;                      /* NFC3 byte - required for anti-collision */
uint8_t  poll_res[18]    = {0x01, 0xFE, 0xB2, 0xB3, 0xB4, 0xB5,
                                   0xB6, 0xB7, 0xC0, 0xC1, 0xC2, 0xC3,
                                   0xC4, 0xC5, 0xC6, 0xC7, 0x23, 0x45 };
/* VASUP-A Command: TCI must be set according to data received via MFi Program  */
static uint8_t demoEcpVasup[] = { 0x6A,    /* VASUP-A Command             */
                                  0x02,    /* Byte1  - Format: 2.0        */
                                  0xCB,    /* Byte2  - Terminal Info      */
                                  0x02,    /* Byte3  - Terminal Type      */
                                  0x04,    /* Byte4  - Terminal Subtype   */
                                  0x02,    /* Byte5  - TCI 1              */
                                  0x11,    /* Byte6  - TCI 2              */
                                  0x00,    /* Byte7  - TCI 3              */
                                  0xb0,0x2a,0x52,0x74,0xec,0x02,0x13,0x4d,  /* Reader Identifier */
};

static uint8_t expTransacSelectApp[] = { 0x00, 0xA4, 0x04, 0x00, 0x0c, 0xA0, 0x00, 0x00, 0x08, 0x58, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00};
uint8_t Atqb[14];
uint8_t AtqbLen;
uint8_t Ats[50];
uint8_t bCidEnabled;
uint8_t bCid;
uint8_t bNadSupported;
uint8_t bFwi;
uint8_t bFsdi;
uint8_t bFsci;
uint8_t bUid[10];
uint8_t bMoreCardsAvailable;
uint8_t bLength;
uint8_t bMbli;
uint8_t bSak;
uint8_t *pRxData;
uint16_t wRxDataLength;
uint8_t aAppBuffer[4100];
uint16_t wTxDataLength;
#ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
uint32_t aDiscTaskBuffer[DISC_DEMO_TASK_STACK];
#else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
#define aDiscTaskBuffer    NULL
#endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */

#ifdef PH_OSAL_FREERTOS
const uint8_t bTaskName[configMAX_TASK_NAME_LEN] = {"DiscLoop"};
#else
const uint8_t bTaskName[] = {"DiscLoop"};
#endif /* PH_OSAL_FREERTOS */

/*******************************************************************************
**   Static Defines
*******************************************************************************/

/* This is used to save restore Poll Config.
 * If in case application has update/change PollCfg to resolve Tech
 * when Multiple Tech was detected in previous poll cycle
 */
static uint16_t bSavePollTechCfg;
static volatile uint8_t bInfLoop = 1U;

/*******************************************************************************
**   Prototypes
*******************************************************************************/
void *pspalI14443p3a;
void *pspalI14443p3b;
void *pspalI14443p4a;
void *pspalI14443p4;
void DiscoveryLoop_Demo(void  *pDataParams);
uint16_t NFCForumProcess(uint16_t wEntryPoint, phStatus_t DiscLoopStatus);

#ifdef ENABLE_DISC_CONFIG
static phStatus_t LoadProfile(phacDiscLoop_Profile_t bProfile);
#endif /* ENABLE_DISC_CONFIG */


#endif
/*******************************************************************************
**   Code
*******************************************************************************/

int main(void)
{
    //printk("hello nxp 5190@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    //printk("i am coming...............\n");
#ifdef PHDRIVER_nRF52840_PNEV5190B_BOARD
printk("PHDRIVER_nRF52840_PNEV5190B_BOARD...............\n");
#endif
#ifdef PH_OSAL_NULLOS
printk("PH_OSAL_NULLOS...............\n");
#endif
#ifdef PH_PLATFORM_HAS_ICFRONTEND
printk("PH_PLATFORM_HAS_ICFRONTEND...............\n");
#endif
    do
    {

        phStatus_t status = PH_ERR_INTERNAL_ERROR;
        phNfcLib_Status_t     dwStatus;
#ifdef PH_PLATFORM_HAS_ICFRONTEND
        phNfcLib_AppContext_t AppContext = {0};
#endif /* PH_PLATFORM_HAS_ICFRONTEND */

#ifndef PH_OSAL_NULLOS
        phOsal_ThreadObj_t DiscLoop;
#endif /* PH_OSAL_NULLOS */
       
        /* Perform Controller specific initialization. */
        phApp_CPU_Init();

        /* Perform OSAL Initialization. */
        (void)phOsal_Init();

        DEBUG_PRINTF("\n DiscoveryLoop Example: \n");

#ifdef PH_PLATFORM_HAS_ICFRONTEND
        status = phbalReg_Init(&sBalParams, sizeof(phbalReg_Type_t));
        CHECK_STATUS(status);

        AppContext.pBalDataparams = &sBalParams;
        dwStatus = phNfcLib_SetContext(&AppContext);
        CHECK_NFCLIB_STATUS(dwStatus);
#endif

        /* Initialize library */
        dwStatus = phNfcLib_Init();
        CHECK_NFCLIB_STATUS(dwStatus);
        if(dwStatus != PH_NFCLIB_STATUS_SUCCESS) break;

        /* Set the generic pointer */
        pHal = phNfcLib_GetDataParams(PH_COMP_HAL);
        pDiscLoop = phNfcLib_GetDataParams(PH_COMP_AC_DISCLOOP);


        ppalI14443p3a = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3A);
        ppalI14443p3b = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3B);
        ppalI14443p4 = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P4);
        ppalI14443p4a = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P4A);
        ppalFelica = phNfcLib_GetDataParams(PH_COMP_PAL_FELICA);
        palTop = phNfcLib_GetDataParams(PH_COMP_AL_TOP);

        pspalI14443p3a  = (void *)ppalI14443p3a;
        pspalI14443p3b  = (void *)ppalI14443p3b;
        pspalI14443p4a  = (void *)ppalI14443p4a;
        pspalI14443p4   = (void *)ppalI14443p4;

        /* Initialize other components that are not initialized by NFCLIB and configure Discovery Loop. */
        status = phApp_Comp_Init(pDiscLoop);
        CHECK_STATUS(status);
        if(status != PH_ERR_SUCCESS) break;

        /* Perform Platform Init */
        status = phApp_Configure_IRQ();
        CHECK_STATUS(status);
        if(status != PH_ERR_SUCCESS) break;
 #if 1
#ifndef PH_OSAL_NULLOS

        DiscLoop.pTaskName = (uint8_t *)bTaskName;
        DiscLoop.pStackBuffer = aDiscTaskBuffer;
        DiscLoop.priority = DISC_DEMO_TASK_PRIO;
        DiscLoop.stackSizeInNum = DISC_DEMO_TASK_STACK;
        phOsal_ThreadCreate(&DiscLoop.ThreadHandle, &DiscLoop, &DiscoveryLoop_Demo, pDiscLoop);

        phOsal_StartScheduler();

        DEBUG_PRINTF("RTOS Error : Scheduler exited. \n");
#else
        (void)DiscoveryLoop_Demo(pDiscLoop);
#endif
#endif
    } while(0);

   // while(bInfLoop); /* Comes here if initialization failure or scheduler exit due to error */
    while(1);
    return 0;
}
#if 1

/**
* \brief Configure Mode
* This function configures HAL and PAL layers to ISO Mode.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t Configure_Mode(void)
{
    phStatus_t wStatus;

    /* Configure HAL to ISO mode */
    wStatus = phhalHw_SetConfig(
        pHal,
        PHHAL_HW_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Configure I14443-A PAL to ISO mode */
    wStatus = phpalI14443p3a_SetConfig(
        pspalI14443p3a,
        PHPAL_I14443P3A_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Configure I14443-B PAL to ISO mode */
    wStatus = phpalI14443p3b_SetConfig(
        pspalI14443p3b,
        PHPAL_I14443P3B_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Configure I14443-4A PAL to ISO mode */
    wStatus = phpalI14443p4a_SetConfig(
        pspalI14443p4a,
        PHPAL_I14443P4A_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Configure I14443-4 PAL to ISO mode */
    wStatus = phpalI14443p4_SetConfig(
        pspalI14443p4,
        PHPAL_I14443P4_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Return Status */
    return wStatus;
}
/**
* This function demonstrates the usage of discovery loop.
* The discovery loop can run with default setting Or can be configured as demonstrated and
* is used to detects and reports the NFC technology type.
* \param   pDataParams      The discovery loop data parameters
* \note    This function will never return
*/
void DiscoveryLoop_Demo(void  *pDataParams)
{
 phStatus_t    status, statustmp ,wStatus;
    uint16_t      wEntryPoint;
      phacDiscLoop_Sw_DataParams_t * pDataParamsSw = (phacDiscLoop_Sw_DataParams_t *)pDataParams;
    phacDiscLoop_Profile_t bProfile = PHAC_DISCLOOP_PROFILE_UNKNOWN;

    /* This call shall allocate secure context before calling any secure function,
     * when FreeRtos trust zone is enabled.
     * */
    phOsal_ThreadSecureStack( 512 );

#ifdef ENABLE_DISC_CONFIG

#ifndef ENABLE_EMVCO_PROF
    bProfile = PHAC_DISCLOOP_PROFILE_NFC;
#else
    bProfile = PHAC_DISCLOOP_PROFILE_EMVCO;
#endif
    /* Load selected profile for Discovery loop */
#if 0
    LoadProfile(bProfile);
#endif
#endif /* ENABLE_DISC_CONFIG */
 wStatus = Configure_Mode();
        CHECK_STATUS(wStatus);
#ifdef NXPBUILD__PHHAL_HW_TARGET
    /* Initialize the setting for Listen Mode */
    status = phApp_HALConfigAutoColl();
    CHECK_STATUS(status);
#endif /* NXPBUILD__PHHAL_HW_TARGET */

    /* Get Poll Configuration */
    status = phacDiscLoop_GetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_PAS_POLL_TECH_CFG, &bSavePollTechCfg);
    CHECK_STATUS(status);

    /* Start in poll mode */
    wEntryPoint = PHAC_DISCLOOP_ENTRY_POINT_POLL;
    status = PHAC_DISCLOOP_LPCD_NO_TECH_DETECTED;

    /* Switch off RF field */
    statustmp = phhalHw_FieldOff(pHal);
    CHECK_STATUS(statustmp);

    while(1)
    {
        /* Before polling set Discovery Poll State to Detection , as later in the code it can be changed to e.g. PHAC_DISCLOOP_POLL_STATE_REMOVAL*/
        statustmp = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
        CHECK_STATUS(statustmp);

#if !defined(ENABLE_EMVCO_PROF) && defined(PH_EXAMPLE1_LPCD_ENABLE)

#ifdef NXPBUILD__PHHAL_HW_RC663
        if (wEntryPoint == PHAC_DISCLOOP_ENTRY_POINT_POLL)
#else
        /* Configure LPCD */
        if ((status & PH_ERR_MASK) == PHAC_DISCLOOP_LPCD_NO_TECH_DETECTED)
#endif
        {
            status = phApp_ConfigureLPCD();
            CHECK_STATUS(status);
        }

        /* Bool to enable LPCD feature. */
        status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_ENABLE_LPCD, PH_ON);
        CHECK_STATUS(status);
#endif /* PH_EXAMPLE1_LPCD_ENABLE*/

        /* Start discovery loop */
#if 0
        status = phacDiscLoop_Run(pDataParams, wEntryPoint);
#endif  
        status = phhalHw_Lpcd(pDataParamsSw->pHalDataParams);
         DEBUG_PRINTF (" ecp sent... \n");
            uint8_t recv_buffer[32];
    uint16_t recv_size = 32;
  hkey_bsp_send_ecp(demoEcpVasup, sizeof(demoEcpVasup));
  //k_msleep(10);
    hkey_bsp_exchange(expTransacSelectApp,  sizeof(expTransacSelectApp), recv_buffer,&recv_size);
   // for(int i=0;i<recv_size;i++)
   // {   DEBUG_PRINTF (" resp:%x ",recv_buffer[i]);   }
   
        if(bProfile == PHAC_DISCLOOP_PROFILE_EMVCO)
        {
#if defined(ENABLE_EMVCO_PROF)

            EmvcoProfileProcess(pDataParams, status);

#endif /* ENABLE_EMVCO_PROF */
        }
        else
        {
            wEntryPoint = NFCForumProcess(wEntryPoint, status);

            /* Set Poll Configuration */
            statustmp = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_PAS_POLL_TECH_CFG, bSavePollTechCfg);
            CHECK_STATUS(statustmp);

            /* Switch off RF field */
            statustmp = phhalHw_FieldOff(pHal);
            CHECK_STATUS(statustmp);

            /* Wait for field-off time-out */
            statustmp = phhalHw_Wait(pHal, PHHAL_HW_TIME_MICROSECONDS, 5100);
            CHECK_STATUS(statustmp);
        }
    }
}

/**
* \brief Type A detection
* This function performs Type A detection.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t DetectTypeA(void)
{
    phStatus_t wStatus;
    uint8_t bIndex;
    uint8_t cnt = 5;
    while (cnt--)
    {
        /* Apply Protocol Setting for Type A */
        wStatus = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);
        CHECK_STATUS(wStatus);

        /* RF Field ON */
        wStatus = phhalHw_FieldOn(pHal);
        if(wStatus != PH_ERR_SUCCESS)
        {
            wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 100);
            continue;
        }
        CHECK_STATUS(wStatus);
        wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 6);
        CHECK_STATUS(wStatus);

        /* Activate Layer 3A card */
        wStatus = phpalI14443p3a_ActivateCard(
            pspalI14443p3a,
            NULL,
            0x00,
            bUid,
            &bLength,
            &bSak,
            &bMoreCardsAvailable);
        if((bSak & 0x24) == 0x20)    //UID complete, PICC compliant with ISO/IEC 14443-4
        {
            DEBUG_PRINTF("found CPU card pSak=0x%x\n",bSak);
        }  
        //ESP_LOG_BUFFER_HEX_LEVEL("UID ", bUid, bLength, ESP_LOG_INFO);
        if (PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
        {
            /* Find CID to be used */
            for(bIndex = 0; bIndex < (bLength - 1); bIndex++)
            {
                if(bUid[bIndex] == 0xBB)
                {
                    bCid = bUid[bIndex + 1];
                    break;
                }
            }
            
            /* Send RATS */
            wStatus = phpalI14443p4a_Rats(
                pspalI14443p4a,
                PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE,    /* Fsdi */
                bCid,
                Ats);
            //ESP_LOG_BUFFER_HEX_LEVEL("RATS ", Ats,50, ESP_LOG_INFO);
            /* Find the baud rates which shall be used */
            if(bUid[0] == 0xAA)
            {
                /* Send PPS */
                (void)phpalI14443p4a_Pps(
                    pspalI14443p4a,
                    (bUid[1] & 0x03),
                    ((bUid[1] & 0x0C) >> 2));

            }

            if (PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
            {
                /* Get parameters from 4A */
                wStatus = phpalI14443p4a_GetProtocolParams(
                    pspalI14443p4a,
                    &bCidEnabled,
                    &bCid,
                    &bNadSupported,
                    &bFwi,
                    &bFsdi,
                    &bFsci);
                CHECK_STATUS(wStatus);

                if(bFsci > PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE)
                {
                    bFsci = PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE;
                }

                /* Apply parameters to layer 4 */
                wStatus = phpalI14443p4_SetProtocol(
                    pspalI14443p4,
                    bCidEnabled,
                    bCid,
                    0,
                    0,
                    bFwi,
                    bFsdi,
                    bFsci);
                CHECK_STATUS(wStatus);
            }
        }
        
        /* Return Status */
        return wStatus;
    }
    return wStatus;
}
bool DetectCpuTypeA(void)
{
    phStatus_t wStatus;
    uint8_t bIndex;
    uint8_t cnt = 5;
    while (cnt--)
    {
        /* Apply Protocol Setting for Type A */
        wStatus = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);
        CHECK_STATUS(wStatus);

        /* RF Field ON */
        wStatus = phhalHw_FieldOn(pHal);
        if(wStatus != PH_ERR_SUCCESS)
        {
            wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 100);
            continue;
        }
        CHECK_STATUS(wStatus);
        wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 6);
        CHECK_STATUS(wStatus);

        /* Activate Layer 3A card */
        wStatus = phpalI14443p3a_ActivateCard(
            pspalI14443p3a,
            NULL,
            0x00,
            bUid,
            &bLength,
            &bSak,
            &bMoreCardsAvailable);
        if((bSak & 0x24) == 0x20)    //UID complete, PICC compliant with ISO/IEC 14443-4
        {
            DEBUG_PRINTF("found CPU card pSak=0x%x\n",bSak);
        } 
        else
        {
            return false;
        }   
        //ESP_LOG_BUFFER_HEX_LEVEL("UID ", bUid, bLength, ESP_LOG_INFO);
        if (PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
        {
            /* Find CID to be used */
            for(bIndex = 0; bIndex < (bLength - 1); bIndex++)
            {
                if(bUid[bIndex] == 0xBB)
                {
                    bCid = bUid[bIndex + 1];
                    break;
                }
            }
            
            /* Send RATS */
            wStatus = phpalI14443p4a_Rats(
                pspalI14443p4a,
                PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE,    /* Fsdi */
                bCid,
                Ats);
            //ESP_LOG_BUFFER_HEX_LEVEL("RATS ", Ats,50, ESP_LOG_INFO);
            /* Find the baud rates which shall be used */
            if(bUid[0] == 0xAA)
            {
                /* Send PPS */
                (void)phpalI14443p4a_Pps(
                    pspalI14443p4a,
                    (bUid[1] & 0x03),
                    ((bUid[1] & 0x0C) >> 2));

            }

            if (PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
            {
                /* Get parameters from 4A */
                wStatus = phpalI14443p4a_GetProtocolParams(
                    pspalI14443p4a,
                    &bCidEnabled,
                    &bCid,
                    &bNadSupported,
                    &bFwi,
                    &bFsdi,
                    &bFsci);
                CHECK_STATUS(wStatus);

                if(bFsci > PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE)
                {
                    bFsci = PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE;
                }

                /* Apply parameters to layer 4 */
                wStatus = phpalI14443p4_SetProtocol(
                    pspalI14443p4,
                    bCidEnabled,
                    bCid,
                    0,
                    0,
                    bFwi,
                    bFsdi,
                    bFsci);
                CHECK_STATUS(wStatus);
            }
        }
        
        /* Return Status */
        return true;
    }
    return false;
}
bool DetectCpuTypeA2(void)
{
    phStatus_t wStatus;
    uint8_t bIndex;
    uint8_t cnt = 5;
    while (cnt--)
    {
        /* Apply Protocol Setting for Type A */
        // DEBUG_PRINTF("DetectCpuTypeA 1");
        // wStatus = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);
        // CHECK_STATUS(wStatus);
        // DEBUG_PRINTF("DetectCpuTypeA 2");
        // /* RF Field ON */
        // wStatus = phhalHw_FieldOn(pHal);
        // if(wStatus != PH_ERR_SUCCESS)
        // {
        //     DEBUG_PRINTF("DetectCpuTypeA delay");
        //     wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 100);
        //     continue;
        // // }
        // DEBUG_PRINTF("DetectCpuTypeA 2");
        // CHECK_STATUS(wStatus);
        // wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 6);
        // CHECK_STATUS(wStatus);
        /* Activate Layer 3A card */
        wStatus = phpalI14443p3a_ActivateCard(
            pspalI14443p3a,
            NULL,
            0x00,
            bUid,
            &bLength,
            &bSak,
            &bMoreCardsAvailable);

        if((bSak & 0x24) == 0x20)    //UID complete, PICC compliant with ISO/IEC 14443-4
        {
            DEBUG_PRINTF("found CPU card pSak=0x%x\n",bSak);
        } 
        else
        {
            return false;
        }   
        //ESP_LOG_BUFFER_HEX_LEVEL("UID ", bUid, bLength, ESP_LOG_INFO);
        if (PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
        {
            /* Find CID to be used */
            for(bIndex = 0; bIndex < (bLength - 1); bIndex++)
            {
                if(bUid[bIndex] == 0xBB)
                {
                    bCid = bUid[bIndex + 1];
                    break;
                }
            }
            /* Send RATS */
            wStatus = phpalI14443p4a_Rats(
                pspalI14443p4a,
                PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE,    /* Fsdi */
                bCid,
                Ats);
            //ESP_LOG_BUFFER_HEX_LEVEL("RATS ", Ats,50, ESP_LOG_INFO);
            /* Find the baud rates which shall be used */
            if(bUid[0] == 0xAA)
            {
                /* Send PPS */
                (void)phpalI14443p4a_Pps(
                    pspalI14443p4a,
                    (bUid[1] & 0x03),
                    ((bUid[1] & 0x0C) >> 2));

            }

            if (PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
            {
                /* Get parameters from 4A */
                wStatus = phpalI14443p4a_GetProtocolParams(
                    pspalI14443p4a,
                    &bCidEnabled,
                    &bCid,
                    &bNadSupported,
                    &bFwi,
                    &bFsdi,
                    &bFsci);
                CHECK_STATUS(wStatus);
                if(bFsci > PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE)
                {
                    bFsci = PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE;
                }
                /* Apply parameters to layer 4 */
                wStatus = phpalI14443p4_SetProtocol(
                    pspalI14443p4,
                    bCidEnabled,
                    bCid,
                    0,
                    0,
                    bFwi,
                    bFsdi,
                    bFsci);
                CHECK_STATUS(wStatus);
            }
        }
        
        /* Return Status */
        return true;
    }
    return false;
}




uint16_t NFCForumProcess(uint16_t wEntryPoint, phStatus_t DiscLoopStatus)
{
    phStatus_t    status;
    uint16_t      wTechDetected = 0;
    uint16_t      wNumberOfTags = 0;
    uint16_t      wValue;
    uint8_t       bIndex;
    uint16_t      wReturnEntryPoint;

    if(wEntryPoint == PHAC_DISCLOOP_ENTRY_POINT_POLL)
    {
        if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_MULTI_TECH_DETECTED)
        {
            DEBUG_PRINTF (" \n Multiple technology detected: \n");

            status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTechDetected);
            CHECK_STATUS(status);

            if(PHAC_DISCLOOP_CHECK_ANDMASK(wTechDetected, PHAC_DISCLOOP_POS_BIT_MASK_A))
            {
                DEBUG_PRINTF (" \tType A detected... \n");
              
              
                
            }
            if(PHAC_DISCLOOP_CHECK_ANDMASK(wTechDetected, PHAC_DISCLOOP_POS_BIT_MASK_B))
            {
                DEBUG_PRINTF (" \tType B detected... \n");
            }
            if(PHAC_DISCLOOP_CHECK_ANDMASK(wTechDetected, PHAC_DISCLOOP_POS_BIT_MASK_F212))
            {
                DEBUG_PRINTF (" \tType F detected with baud rate 212... \n");
            }
            if(PHAC_DISCLOOP_CHECK_ANDMASK(wTechDetected, PHAC_DISCLOOP_POS_BIT_MASK_F424))
            {
                DEBUG_PRINTF (" \tType F detected with baud rate 424... \n");
            }
            if(PHAC_DISCLOOP_CHECK_ANDMASK(wTechDetected, PHAC_DISCLOOP_POS_BIT_MASK_V))
            {
                DEBUG_PRINTF(" \tType V / ISO 15693 / T5T detected... \n");
            }

            /* Select 1st Detected Technology to Resolve*/
            for(bIndex = 0; bIndex < PHAC_DISCLOOP_PASS_POLL_MAX_TECHS_SUPPORTED; bIndex++)
            {
                if(PHAC_DISCLOOP_CHECK_ANDMASK(wTechDetected, (1 << bIndex)))
                {
                    /* Configure for one of the detected technology */
                    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_POLL_TECH_CFG, (1 << bIndex));
                    CHECK_STATUS(status);
                    break;
                }
            }

            /* Print the technology resolved */
            phApp_PrintTech((1 << bIndex));

            /* Set Discovery Poll State to collision resolution */
            status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_COLLISION_RESOLUTION);
            CHECK_STATUS(status);

            /* Restart discovery loop in poll mode from collision resolution phase */
            DiscLoopStatus = phacDiscLoop_Run(pDiscLoop, wEntryPoint);
        }

        if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_MULTI_DEVICES_RESOLVED)
        {
            /* Get Detected Technology Type */
            status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTechDetected);
            CHECK_STATUS(status);

            /* Get number of tags detected */
            status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NR_TAGS_FOUND, &wNumberOfTags);
            CHECK_STATUS(status);

            DEBUG_PRINTF (" \n Multiple cards resolved: %d cards \n",wNumberOfTags);
            phApp_PrintTagInfo(pDiscLoop, wNumberOfTags, wTechDetected);

            if(wNumberOfTags > 1)
            {
                /* Get 1st Detected Technology and Activate device at index 0 */
                for(bIndex = 0; bIndex < PHAC_DISCLOOP_PASS_POLL_MAX_TECHS_SUPPORTED; bIndex++)
                {
                    if(PHAC_DISCLOOP_CHECK_ANDMASK(wTechDetected, (1 << bIndex)))
                    {
                        DEBUG_PRINTF("\t Activating one card...\n");
                        status = phacDiscLoop_ActivateCard(pDiscLoop, bIndex, 0);
                        break;
                    }
                }

                if(((status & PH_ERR_MASK) == PHAC_DISCLOOP_DEVICE_ACTIVATED) ||
                        ((status & PH_ERR_MASK) == PHAC_DISCLOOP_PASSIVE_TARGET_ACTIVATED) ||
                        ((status & PH_ERR_MASK) == PHAC_DISCLOOP_MERGED_SEL_RES_FOUND))
                {
                    /* Get Detected Technology Type */
                    status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTechDetected);
                    CHECK_STATUS(status);

                    phApp_PrintTagInfo(pDiscLoop, 0x01, wTechDetected);
                }
                else
                {
                    PRINT_INFO("\t\tCard activation failed...\n");
                }
            }
            /* Switch to LISTEN mode after POLL mode */
        }
        else if (((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_NO_TECH_DETECTED) ||
                ((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_NO_DEVICE_RESOLVED))
        {
            /* Switch to LISTEN mode after POLL mode */
        }
        else if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_EXTERNAL_RFON)
        {
            /*
             * If external RF is detected during POLL, return back so that the application
             * can restart the loop in LISTEN mode
             */
        }
        else if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_MERGED_SEL_RES_FOUND)
        {
            DEBUG_PRINTF (" \n Device having T4T and NFC-DEP support detected... \n");

            /* Get Detected Technology Type */
            status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTechDetected);
            CHECK_STATUS(status);

            phApp_PrintTagInfo(pDiscLoop, 1, wTechDetected);

        /* Switch to LISTEN mode after POLL mode */
        }
        else if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_DEVICE_ACTIVATED)
        {
            DEBUG_PRINTF (" \n Card detected and activated successfully... \n");
            status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NR_TAGS_FOUND, &wNumberOfTags);
            CHECK_STATUS(status);

            /* Get Detected Technology Type */
            status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTechDetected);
            CHECK_STATUS(status);

            phApp_PrintTagInfo(pDiscLoop, wNumberOfTags, wTechDetected);

            /* Switch to LISTEN mode after POLL mode */
        }
        else if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_ACTIVE_TARGET_ACTIVATED)
        {
            DEBUG_PRINTF (" \n Active target detected... \n");

            /* Switch to LISTEN mode after POLL mode */
        }
        else if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_PASSIVE_TARGET_ACTIVATED)
        {
            DEBUG_PRINTF (" \n Passive target detected... \n");

            /* Get Detected Technology Type */
            status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTechDetected);
            CHECK_STATUS(status);

            phApp_PrintTagInfo(pDiscLoop, 1, wTechDetected);

            /* Switch to LISTEN mode after POLL mode */
        }
        else if ((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_LPCD_NO_TECH_DETECTED)
        {
            /* LPCD is succeed but no tag is detected. */
        }
        else
        {
            if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_FAILURE)
            {
                status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ADDITIONAL_INFO, &wValue);
                CHECK_STATUS(status);
                DEBUG_ERROR_PRINT(PrintErrorInfo(wValue));
            }
            else
            {
                DEBUG_ERROR_PRINT(PrintErrorInfo(status));
            }
        }

        /* Update the Entry point to LISTEN mode. */
        wReturnEntryPoint = PHAC_DISCLOOP_ENTRY_POINT_LISTEN;

    }
    else
    {
        if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_EXTERNAL_RFOFF)
        {
            /*
             * Enters here if in the target/card mode and external RF is not available
             * Wait for LISTEN timeout till an external RF is detected.
             * Application may choose to go into standby at this point.
             */
            status = phhalHw_EventConsume(pHal);
            CHECK_STATUS(status);

            status = phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_RFON_INTERRUPT, PH_ON);
            CHECK_STATUS(status);

            status = phhalHw_EventWait(pHal, LISTEN_PHASE_TIME_MS);
            if((status & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
            {
                wReturnEntryPoint = PHAC_DISCLOOP_ENTRY_POINT_POLL;
            }
            else
            {
                wReturnEntryPoint = PHAC_DISCLOOP_ENTRY_POINT_LISTEN;
            }
        }
        else
        {
            if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_ACTIVATED_BY_PEER)
            {
                DEBUG_PRINTF (" \n Device activated in listen mode... \n");
            }
            else if ((DiscLoopStatus & PH_ERR_MASK) == PH_ERR_INVALID_PARAMETER)
            {
                /* In case of Front end used is RC663, then listen mode is not supported.
                 * Switch from listen mode to poll mode. */
            }
            else
            {
                if((DiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_FAILURE)
                {
                    status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ADDITIONAL_INFO, &wValue);
                    CHECK_STATUS(status);
                    DEBUG_ERROR_PRINT(PrintErrorInfo(wValue));
                }
                else
                {
                    DEBUG_ERROR_PRINT(PrintErrorInfo(status));
                }
            }

            /* On successful activated by Peer, switch to LISTEN mode */
            wReturnEntryPoint = PHAC_DISCLOOP_ENTRY_POINT_POLL;
        }
    }
    return wReturnEntryPoint;
}

#ifdef ENABLE_DISC_CONFIG
/**
* This function will load/configure Discovery loop with default values based on interested profile
 * Application can read these values from EEPROM area and load/configure Discovery loop via SetConfig
* \param   bProfile      Reader Library Profile
* \note    Values used below are default and is for demonstration purpose.
*/
static phStatus_t LoadProfile(phacDiscLoop_Profile_t bProfile)
{
    phStatus_t status = PH_ERR_SUCCESS;
    uint16_t   wPasPollConfig = 0;
    uint16_t   wActPollConfig = 0;
    uint16_t   wPasLisConfig = 0;
    uint16_t   wActLisConfig = 0;

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS
    wPasPollConfig |= PHAC_DISCLOOP_POS_BIT_MASK_A;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS
    wPasPollConfig |= PHAC_DISCLOOP_POS_BIT_MASK_B;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS
    wPasPollConfig |= (PHAC_DISCLOOP_POS_BIT_MASK_F212 | PHAC_DISCLOOP_POS_BIT_MASK_F424);
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEV_TAGS
    wPasPollConfig |= PHAC_DISCLOOP_POS_BIT_MASK_V;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_I18000P3M3_TAGS
    wPasPollConfig |= PHAC_DISCLOOP_POS_BIT_MASK_18000P3M3;
#endif

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_ACTIVE
    wActPollConfig |= PHAC_DISCLOOP_ACT_POS_BIT_MASK_106;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF212_P2P_ACTIVE
    wActPollConfig |= PHAC_DISCLOOP_ACT_POS_BIT_MASK_212;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF424_P2P_ACTIVE
    wActPollConfig |= PHAC_DISCLOOP_ACT_POS_BIT_MASK_424;
#endif

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_TARGET_PASSIVE
    wPasLisConfig |= PHAC_DISCLOOP_POS_BIT_MASK_A;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF212_TARGET_PASSIVE
    wPasLisConfig |= PHAC_DISCLOOP_POS_BIT_MASK_F212;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF424_TARGET_PASSIVE
    wPasLisConfig |= PHAC_DISCLOOP_POS_BIT_MASK_F424;
#endif

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_TARGET_ACTIVE
    wActLisConfig |= PHAC_DISCLOOP_POS_BIT_MASK_A;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF212_TARGET_ACTIVE
    wActLisConfig |= PHAC_DISCLOOP_POS_BIT_MASK_F212;
#endif
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF424_TARGET_ACTIVE
    wActLisConfig |= PHAC_DISCLOOP_POS_BIT_MASK_F424;
#endif

    if(bProfile == PHAC_DISCLOOP_PROFILE_NFC)
    {
        /* passive Bailout bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_BAIL_OUT, 0x00);
        CHECK_STATUS(status);

        /* Set Passive poll bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_POLL_TECH_CFG, wPasPollConfig);
        CHECK_STATUS(status);

        /* Set Active poll bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ACT_POLL_TECH_CFG, wActPollConfig);
        CHECK_STATUS(status);

        /* Set Passive listen bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_LIS_TECH_CFG, wPasLisConfig);
        CHECK_STATUS(status);

        /* Set Active listen bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ACT_LIS_TECH_CFG, wActLisConfig);
        CHECK_STATUS(status);

        /* reset collision Pending */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_COLLISION_PENDING, PH_OFF);
        CHECK_STATUS(status);

        /* whether anti-collision is supported or not. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ANTI_COLL, PH_ON);
        CHECK_STATUS(status);

        /* Poll Mode default state*/
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
        CHECK_STATUS(status);

#ifdef  NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS
        /* Device limit for Type A */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_DEVICE_LIMIT, 1);
        CHECK_STATUS(status);

        /* Passive polling Tx Guard times in micro seconds. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTA_VALUE_US, 5100);
        CHECK_STATUS(status);
#endif

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS
        /* Device limit for Type B */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DEVICE_LIMIT, 1);
        CHECK_STATUS(status);

        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTB_VALUE_US, 5100);
        CHECK_STATUS(status);
#endif

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS
        /* Device limit for Type F */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEF_DEVICE_LIMIT, 1);
        CHECK_STATUS(status);

        /* Guard time for Type F. This guard time is applied when Type F poll before Type B */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTFB_VALUE_US, 20400);
        CHECK_STATUS(status);

        /* Guard time for Type F. This guard time is applied when Type B poll before Type F */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTBF_VALUE_US, 15300);
        CHECK_STATUS(status);
#endif

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEV_TAGS
        /* Device limit for Type V (ISO 15693) */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_DEVICE_LIMIT, 1);
        CHECK_STATUS(status);

        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTV_VALUE_US, 5200);
        CHECK_STATUS(status);
#endif

#ifdef NXPBUILD__PHAC_DISCLOOP_I18000P3M3_TAGS
        /* Device limit for 18000P3M3 */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_18000P3M3_DEVICE_LIMIT, 1);
        CHECK_STATUS(status);

        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GT18000P3M3_VALUE_US, 10000);
        CHECK_STATUS(status);
#endif

        /* Discovery loop Operation mode */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_OPE_MODE, RD_LIB_MODE_NFC);
        CHECK_STATUS(status);
    }
    else if(bProfile == PHAC_DISCLOOP_PROFILE_EMVCO)
    {
        /* EMVCO */
        /* passive Bailout bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_BAIL_OUT, 0x00);
        CHECK_STATUS(status);

        /* passive poll bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_POLL_TECH_CFG, (PHAC_DISCLOOP_POS_BIT_MASK_A | PHAC_DISCLOOP_POS_BIT_MASK_B));
        CHECK_STATUS(status);

        /* Active Listen bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_LIS_TECH_CFG, 0x00);
        CHECK_STATUS(status);

        /* Active Listen bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ACT_LIS_TECH_CFG, 0x00);
        CHECK_STATUS(status);

        /* Active Poll bitmap config. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ACT_POLL_TECH_CFG, 0x00);
        CHECK_STATUS(status);

        /* Bool to enable LPCD feature. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ENABLE_LPCD, PH_OFF);
        CHECK_STATUS(status);

        /* reset collision Pending */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_COLLISION_PENDING, PH_OFF);
        CHECK_STATUS(status);

        /* whether anti-collision is supported or not. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_ANTI_COLL, PH_ON);
        CHECK_STATUS(status);

        /* Poll Mode default state*/
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
        CHECK_STATUS(status);

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS
        /* Device limit for Type A */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_DEVICE_LIMIT, 1);
        CHECK_STATUS(status);

        /* Passive polling Tx Guard times in micro seconds. */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTA_VALUE_US, 5100);
        CHECK_STATUS(status);

        /* Configure FSDI for the 14443P4A tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_FSDI, 0x08);
        CHECK_STATUS(status);

        /* Configure CID for the 14443P4A tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_CID, 0x00);
        CHECK_STATUS(status);

        /* Configure DRI for the 14443P4A tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_DRI, 0x00);
        CHECK_STATUS(status);

        /* Configure DSI for the 14443P4A tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_DSI, 0x00);
        CHECK_STATUS(status);
#endif

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS
        /* Device limit for Type B */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DEVICE_LIMIT, 1);
        CHECK_STATUS(status);

        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTB_VALUE_US, 5100);
        CHECK_STATUS(status);

        /* Configure AFI for the type B tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_AFI_REQ, 0x00);
        CHECK_STATUS(status);

        /* Configure FSDI for the type B tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_FSDI, 0x08);
        CHECK_STATUS(status);

        /* Configure CID for the type B tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_CID, 0x00);
        CHECK_STATUS(status);

        /* Configure DRI for the type B tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DRI, 0x00);
        CHECK_STATUS(status);

        /* Configure DSI for the type B tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DSI, 0x00);
        CHECK_STATUS(status);

        /* Configure Extended ATQB support for the type B tags */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_EXTATQB, 0x00);
        CHECK_STATUS(status);
#endif
        /* Configure reader library mode */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_OPE_MODE, RD_LIB_MODE_EMVCO);
        CHECK_STATUS(status);
    }
    else
    {
        /* Do Nothing */
    }
    return status;
}
#endif /* ENABLE_DISC_CONFIG */
#endif