/*----------------------------------------------------------------------------*/
/* Copyright 2017-2022 NXP                                                    */
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

/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */

#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "phDriver.h"
#include "BoardSelection.h"




/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */


/* *****************************************************************************************************************
 * Type Definitions
 * ***************************************************************************************************************** */
#define PN5190_NODE		DT_INST(0, nxp_pn5190)
/* *****************************************************************************************************************
 * Global and Static Variables
 * Total Size: NNNbytes
 * ***************************************************************************************************************** */


static pphDriver_TimerCallBck_t pPitTimerCallBack;
static volatile uint8_t dwTimerExp;
static struct gpio_callback int_cb_data;
static const struct gpio_dt_spec reset_gpio = GPIO_DT_SPEC_GET(PN5190_NODE, boardreset_gpios);
static const struct gpio_dt_spec irq_gpio = GPIO_DT_SPEC_GET(PN5190_NODE, irq_gpios);
static struct gpio_callback irq_cb_data;
K_EVENT_DEFINE(my_event);
/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
void isr_callback(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	k_event_post(&my_event, 0x1);
	printk("###");

}

void my_timer_handler(struct k_timer *dummy)
{
      if(NULL != pPitTimerCallBack)
    	{
    		pPitTimerCallBack();
    	}
}

K_TIMER_DEFINE(my_timer, my_timer_handler, NULL);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
phStatus_t phDriver_TimerStart(phDriver_Timer_Unit_t eTimerUnit, uint32_t dwTimePeriod, pphDriver_TimerCallBck_t pTimerCallBack)
{

  

	pPitTimerCallBack = pTimerCallBack;
	switch(eTimerUnit)
	{
		case PH_DRIVER_TIMER_SECS:
				  k_timer_start(&my_timer, K_SECONDS(dwTimePeriod), K_NO_WAIT);
				  break;
		case PH_DRIVER_TIMER_MILLI_SECS:
				   k_timer_start(&my_timer, K_MSEC(dwTimePeriod), K_NO_WAIT);
				   break;
		case PH_DRIVER_TIMER_MICRO_SECS:
				 k_timer_start(&my_timer, K_USEC(dwTimePeriod), K_NO_WAIT);
				   break;
				   default:
				   printk("phDriver_TimerStart eTimerUnit err\n");
				   break;

	}


   	if(NULL == pTimerCallBack)
    	{
    		k_timer_status_sync(&my_timer);
    	}
    return PH_DRIVER_SUCCESS;
}

phStatus_t phDriver_TimerStop(void)
{
	k_timer_stop(&my_timer);


    return PH_DRIVER_SUCCESS;
}

phStatus_t phDriver_PinConfig(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Pin_Config_t *pPinConfig)
{
    //gpio_pin_config_t sGpioConfig;
   // uint8_t bPinNum;
    //uint8_t bPortGpio;
	//struct device *dev_gpioX;
int ret;
    //port_interrupt_t eInterruptType;
    /*port_pin_config_t sPinConfig =
    {
        .pullSelect = kPORT_PullDisable,
        .slewRate = kPORT_FastSlewRate,
        .passiveFilterEnable = kPORT_PassiveFilterDisable,
        .openDrainEnable = kPORT_OpenDrainDisable,
        .driveStrength = kPORT_HighDriveStrength,
        .mux = kPORT_MuxAsGpio,
        .lockRegister = kPORT_UnlockRegister
    };*/

    if((ePinFunc == PH_DRIVER_PINFUNC_BIDIR) || (pPinConfig == NULL))
    {
        return PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }

	if (!gpio_is_ready_dt(&reset_gpio)) {
		printk("Error: reset_gpio device %s is not ready\n",
		       reset_gpio.port->name);
		return PH_DRIVER_ERROR | PH_COMP_DRIVER;;
	}
	if (!gpio_is_ready_dt(&irq_gpio)) {
		printk("Error: irq_gpio device %s is not ready\n",
		       irq_gpio.port->name);
		return PH_DRIVER_ERROR | PH_COMP_DRIVER;;
	}
    /* Extract the Pin, Gpio, Port details from dwPinNumber
	if(dwPinNumber<32)
		{
    		bPinNum = (uint8_t)(dwPinNumber);
    		bPortGpio = (uint8_t)(0);
		}
	else if(dwPinNumber>=32)
		{
			bPinNum = (uint8_t)(dwPinNumber-32);
    		bPortGpio = (uint8_t)(1);
		} */
    switch(dwPinNumber)
	{
		case PHDRIVER_PIN_RESET:
			gpio_pin_configure_dt(&reset_gpio, GPIO_OUTPUT);
			gpio_pin_set_dt(&reset_gpio, pPinConfig->bOutputLogic);
			break;
		case PHDRIVER_PIN_IRQ:
			gpio_pin_configure_dt(&irq_gpio, GPIO_INPUT);
			ret = gpio_pin_interrupt_configure_dt(&irq_gpio,
					      GPIO_INT_EDGE_TO_ACTIVE);
			if (ret != 0) {
				return PH_DRIVER_ERROR | PH_COMP_DRIVER;;
			}
			gpio_init_callback(&irq_cb_data, isr_callback, BIT(irq_gpio.pin));
			gpio_add_callback(irq_gpio.port, &irq_cb_data);
			break;
				

	}

    return PH_DRIVER_SUCCESS;
}

uint8_t phDriver_PinRead(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc)
{

		int value = 0;
		if(PHDRIVER_PIN_IRQ == dwPinNumber)
		{     
			value = gpio_pin_get_dt(&irq_gpio);
			value = (value < 0) ? 0 : value;
		}
	return value;

  
}

phStatus_t phDriver_IRQPinPoll(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Interrupt_Config_t eInterruptType)
{
    uint8_t    bGpioState = 0;

    if ((eInterruptType != PH_DRIVER_INTERRUPT_RISINGEDGE) && (eInterruptType != PH_DRIVER_INTERRUPT_FALLINGEDGE))
    {
        return PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }

    if (eInterruptType == PH_DRIVER_INTERRUPT_FALLINGEDGE)
    {
        bGpioState = 1;
    }
	if(PHDRIVER_PIN_IRQ == dwPinNumber)
		{     
			//while(phDriver_PinRead(dwPinNumber, ePinFunc) == bGpioState);
printk("phDriver_IRQPinPoll_1\n");
					 k_event_wait(&my_event, 0x007, false,K_FOREVER);
					k_event_clear(&my_event,0x1);
					printk("phDriver_IRQPinPoll_2\n");
		}

    return PH_DRIVER_SUCCESS;
}

void phDriver_PinWrite(uint32_t dwPinNumber, uint8_t bValue)
{
 


		if(PHDRIVER_PIN_RESET == dwPinNumber)
		{     
				gpio_pin_set_dt(&reset_gpio,bValue);
		}

}

void phDriver_PinClearIntStatus(uint32_t dwPinNumber)
{

}

phStatus_t phDriver_IRQPinRead(uint32_t dwPinNumber)
{
	phStatus_t bGpioVal = false;

	bGpioVal = phDriver_PinRead(dwPinNumber, PH_DRIVER_PINFUNC_INPUT);

	return bGpioVal;
}




void phDriver_EnterCriticalSection(void)
{
  //  __disable_irq();
}

void phDriver_ExitCriticalSection(void)
{
 //   __enable_irq();
}
