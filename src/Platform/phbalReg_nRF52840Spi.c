/*
*         Copyright (c), NXP Semiconductors Bangalore / India
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Generic phDriver Component of Reader Library Framework.
* $Author$
* $Revision$
* $Date$
*
* History:
*  RS: Generated 24. Jan 2017
*
*/
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <stddef.h>
#include <zephyr/types.h>
#include <zephyr/sys/util.h>
#include "phDriver.h"
#include "BoardSelection.h"


#define PN5190_NODE		DT_INST(0, nxp_pn5190)
#define PHBAL_REG_KINETIS_SPI_ID               0x0FU       /**< ID for Kinetis SPI BAL component */

#ifndef PHDRIVER_KSDK_SPI_POLLING
dspi_rtos_handle_t g_masterHandle;
#endif

#define RX_BUFFER_SIZE_MAX                     272U /* Receive Buffer size while exchange */

static void phbalReg_SpiInit(void);

#ifdef PERF_TEST
static uint32_t dwSpiBaudRate = PHDRIVER_KSDK_SPI_DATA_RATE;
#endif /* PERF_TEST */
static     uint8_t g_dummyBuffer[RX_BUFFER_SIZE_MAX];
#if 1
static const struct device *spi_dev = DEVICE_DT_GET(DT_BUS(PN5190_NODE));
/* SPI hardware configuration. */
static const struct spi_config spi_cfg =  {
	.frequency = DT_PROP(PN5190_NODE, spi_max_frequency),
	.operation = (SPI_OP_MODE_MASTER | SPI_WORD_SET(8) |
		      SPI_TRANSFER_MSB | SPI_LINES_SINGLE |
		      SPI_MODE_CPHA),
	.slave = DT_REG_ADDR(PN5190_NODE),
	.cs = {
		.gpio = SPI_CS_GPIOS_DT_SPEC_GET(PN5190_NODE),
		.delay = T_NCS_SCLK
	}
};
#endif
phStatus_t phbalReg_Init(
                         void * pDataParams,
                         uint16_t wSizeOfDataParams
                         )
{

    if ( (pDataParams == NULL) || (sizeof(phbalReg_Type_t) != wSizeOfDataParams))
    {
        return (PH_DRIVER_ERROR | PH_COMP_DRIVER);
    }

    ((phbalReg_Type_t *)pDataParams)->wId      = PH_COMP_DRIVER | PHBAL_REG_KINETIS_SPI_ID;
    ((phbalReg_Type_t *)pDataParams)->bBalType = PHBAL_REG_TYPE_SPI;
#if 0
	LOG_DBG("Initializing. SPI device: %s, CS GPIO: %s pin %d",
		spi_dev->name, spi_cfg.cs.gpio.port->name, spi_cfg.cs.gpio.pin);

	if (!device_is_ready(spi_cfg.cs.gpio.port)) {
		LOG_ERR("GPIO device %s is not ready!", spi_cfg.cs.gpio.port->name);

		return -ENXIO;
	}

	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI device %s is not ready!", spi_dev->name);
		return -ENXIO;
	}
#endif

    return PH_DRIVER_SUCCESS;
}

phStatus_t phbalReg_Exchange(
                             void * pDataParams,
                             uint16_t wOption,
                             uint8_t * pTxBuffer,
                             uint16_t wTxLength,
                             uint16_t wRxBufSize,
                             uint8_t * pRxBuffer,
                             uint16_t * pRxLength
                             )
{
    phStatus_t status = PH_DRIVER_SUCCESS;
    uint8_t * pRxBuf;
  //  status_t dspiStatus;
   // dspi_transfer_t g_masterXfer;
	int err;

#if 0
  //  memset(&g_masterXfer, 0, sizeof(dspi_transfer_t));


    if(pTxBuffer == NULL)
    {
        wTxLength = wRxBufSize;
        g_dummyBuffer[0] = 0xFF;
      //  pTxBuffer = g_dummyBuffer;
    }
	else
		{
			memcpy(g_dummyBuffer, pTxBuffer, wTxLength );
		}

	
	const struct spi_buf tx_bufs[] = {
		{.buf = g_dummyBuffer, .len = wTxLength}
	};
	const struct spi_buf_set tx = {
		.buffers = tx_bufs,
		.count = ARRAY_SIZE(tx_bufs)
	};

	
    if(pRxBuffer != NULL)
    {
        //pRxBuf = g_dummyBuffer;
        	const struct spi_buf rx_bufs[] = {
				{.buf = pRxBuffer, .len = wTxLength}
			};
			const struct spi_buf_set rx = {
				.buffers = rx_bufs,
				.count = ARRAY_SIZE(rx_bufs)
			};

			err = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
			if (err) {
				LOG_ERR("SPI reg read failed, err: %d.", err);
				
			}
    }
    else
    {
        //pRxBuf = pRxBuffer;
        err = spi_transceive(spi_dev, &spi_cfg, &tx, NULL);
			if (err) {
				LOG_ERR("SPI direct command failed, err: %d.", err);
				
			}
    }
	
    /* Set up the transfer 
    g_masterXfer.txData = pTxBuffer;
    g_masterXfer.rxData = pRxBuf;
    g_masterXfer.dataSize = wTxLength;
    g_masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;
	*/

    /* Start transfer 
#ifdef PHDRIVER_KSDK_SPI_POLLING
    dspiStatus = DSPI_MasterTransferBlocking(PHDRIVER_KSDK_SPI_MASTER, &g_masterXfer);
#else
    dspiStatus = DSPI_RTOS_Transfer(&g_masterHandle, &g_masterXfer);
#endif
	*/

    if (err)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    if (pRxLength != NULL)
    {
        *pRxLength = wTxLength;
    }
#endif
    return status;
}

phStatus_t phbalReg_SetConfig(
                              void * pDataParams,
                              uint16_t wConfig,
                              uint32_t dwValue
                              )
{

    return PH_DRIVER_SUCCESS;
}

phStatus_t phbalReg_GetConfig(
                              void * pDataParams,
                              uint16_t wConfig,
                              uint32_t * pValue
                              )
{

    return PH_DRIVER_SUCCESS;
}


