/*----------------------------------------------------------------------------*/
/* Copyright 2018-2021 NXP                                                    */
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
* Generic phDriver Component of Reader Library Framework.
* $Author$
* $Revision$
* $Date$
*
*/

#ifndef BOARD_NRF52840_PNEV5190B_H_
#define BOARD_NRF52840_PNEV5190B_H_





/******************************************************************
 * Board Pin/Gpio configurations
 ******************************************************************/
/* Pin configuration format : Its a 32 bit format where every byte represents a field as shown below.
 * | Byte3 | Byte2 | Byte1      | Byte0 |
 * |  --   |  --   | GPIO/PORT  | PIN   |
 * */
#define PHDRIVER_PIN_RESET  29 
#define PHDRIVER_PIN_IRQ    3   

/* For 5190 busy is same as IRQ */
#define PHDRIVER_PIN_BUSY    PHDRIVER_PIN_IRQ 
#define PHDRIVER_PIN_DWL      -1/**< Download pin, Pin4, GPIO9 */

 

/******************************************************************
 * PIN Pull-Up/Pull-Down configurations.
 ******************************************************************/
#define PHDRIVER_PIN_RESET_PULL_CFG    PH_DRIVER_PULL_UP
#define PHDRIVER_PIN_IRQ_PULL_CFG      PH_DRIVER_PULL_DOWN
#define PHDRIVER_PIN_NSS_PULL_CFG      PH_DRIVER_PULL_UP

#define PHDRIVER_PIN_BUSY_PULL_CFG     PH_DRIVER_PULL_UP
#define PHDRIVER_PIN_DWL_PULL_CFG      PH_DRIVER_PULL_UP


/******************************************************************
 * IRQ PIN NVIC settings
 ******************************************************************/

#define PIN_IRQ_TRIGGER_TYPE      PH_DRIVER_INTERRUPT_RISINGEDGE

/*****************************************************************
 * Front End Reset logic level settings
 ****************************************************************/
#define PH_DRIVER_SET_HIGH            1          /**< Logic High. */
#define PH_DRIVER_SET_LOW             0          /**< Logic Low. */
#define RESET_POWERDOWN_LEVEL         PH_DRIVER_SET_LOW
#define RESET_POWERUP_LEVEL           PH_DRIVER_SET_HIGH

/*****************************************************************
 * SPI Configuration
 ****************************************************************/


#define PHDRIVER_PIN_SSEL    44  /**< Slave select pin, pin28, GPIO9 */
#endif /* BOARD_NRF52840_PNEV5190B_H_ */
