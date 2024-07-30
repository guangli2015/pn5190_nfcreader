/*----------------------------------------------------------------------------*/
/* Copyright 2021 NXP                                                         */ 
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

 /** @file
 * Freertos OSAL Component of Reader Library Framework.
 * $Author:: NXP $
 * $Revision$
 */
#include <phOsal.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>


/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */

/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */

/* *****************************************************************************************************************
 * Type Definitions
 * ***************************************************************************************************************** */

/* *****************************************************************************************************************
 * Global and Static Variables
 * Total Size: NNNbytes
 * ***************************************************************************************************************** 
static volatile uint8_t gbWaitTimedOut;
static volatile uint32_t gdwEvents[PH_OSAL_CONFIG_MAX_NUM_EVENTS];
static volatile uint32_t gdwEventBitMap;
*/


 static struct k_event  my_event_queue[5];

 static bool            eventqueue_use_flag[5]={false};
/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
//static phStatus_t phOsal_NullOs_GetFreeIndex(uint32_t * dwFreeIndex, uint32_t dwBitMap, uint32_t dwMaxLimit);

static phStatus_t phOsal_NullOs_ReturnUnsupportedCmd(void);
/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
phStatus_t phOsal_Init(void)
{
   // gbWaitTimedOut = 0;
   // gdwEventBitMap = 0;

   // return phOsal_InitTickTimer(&phOsal_NullOsSysTickHandler);
   return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_EventCreate(phOsal_Event_t *eventHandle, pphOsal_EventObj_t eventObj)
{
  /*uint32_t bEventIndex = 0;

    if ((eventHandle == NULL) || (eventObj == NULL))
    {
        return PH_OSAL_ADD_COMPCODE(PH_OSAL_ERROR, PH_COMP_OSAL);
    }

    PH_OSAL_CHECK_SUCCESS(phOsal_NullOs_GetFreeIndex(&bEventIndex, gdwEventBitMap, PH_OSAL_CONFIG_MAX_NUM_EVENTS));

    gdwEvents[bEventIndex] = 0;

    gdwEventBitMap |= (1 << bEventIndex);
    *eventHandle = (phOsal_Event_t)(&gdwEvents[bEventIndex]);
    eventObj->EventHandle = (phOsal_Event_t)(&gdwEvents[bEventIndex]);
    eventObj->dwEventIndex = bEventIndex;
*/

	uint8_t index;

     if ((eventHandle == NULL) || (eventObj == NULL))
    {
        return PH_OSAL_ADD_COMPCODE(PH_OSAL_ERROR, PH_COMP_OSAL);
    }
	
#if 1
	for(index=0;index<5;index++)
		{	
			if(eventqueue_use_flag[index]==false)
				{
				
					break;
				}
		}

	if(index>=5)
		{
			return PH_OSAL_ADD_COMPCODE(PH_OSAL_ERROR, PH_COMP_OSAL);
		}
	eventqueue_use_flag[index]=true;
	k_event_init(&(my_event_queue[index]));
	*eventHandle = (phOsal_Event_t)(&(my_event_queue[index]));
	 eventObj->EventHandle = (phOsal_Event_t)(&(my_event_queue[index]));
	 eventObj->dwEventIndex = index;
#endif
    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_EventPend(volatile phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_Ticks_t ticksToWait,
    phOsal_EventBits_t FlagsToWait, phOsal_EventBits_t *pCurrFlags)
{
    phStatus_t status;
    status=PH_OSAL_SUCCESS;
#if 0
    if((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return PH_OSAL_ADD_COMPCODE(PH_OSAL_ERROR, PH_COMP_OSAL);
    }

    status = PH_OSAL_IO_TIMEOUT;

    /* Check whether infinite wait, if not config timer. */
    if (ticksToWait != PHOSAL_MAX_DELAY)
    {
        phOsal_StartTickTimer(ticksToWait);
    }

    while(1)
    {
        /* Enter Critical Section */
        phOsal_EnterCriticalSection();

        if ((((options & E_OS_EVENT_OPT_PEND_SET_ALL) && (((*((uint32_t *)(*eventHandle))) & FlagsToWait) == FlagsToWait))
            || ((!(options & E_OS_EVENT_OPT_PEND_SET_ALL)) && ((*((uint32_t *)(*eventHandle))) & FlagsToWait)))
            || (gbWaitTimedOut))
        {
            /* Exit Critical Section. */
            phOsal_ExitCriticalSection();
            if (gbWaitTimedOut != 0x01)
            {
                status = PH_OSAL_SUCCESS;
            }
            break;
        }

        /* Exit Critical Section. */
        phOsal_ExitCriticalSection();

        /* Wait for interrupts/events to occur */
        phOsal_Sleep();
    }

    /* Check whether infinite wait, if not config timer. */
    if (ticksToWait != PHOSAL_MAX_DELAY)
    {
        phOsal_StopTickTimer();
    }
    gbWaitTimedOut = 0;

    phOsal_EnterCriticalSection();
    if (pCurrFlags != NULL)
    {
        *pCurrFlags = (*((uint32_t *)(*eventHandle)));
    }

    if (options & E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT)
    {
        (*((uint32_t *)(*eventHandle))) &= (~(FlagsToWait & (*((uint32_t *)(*eventHandle)))));
    }
    phOsal_ExitCriticalSection();
#endif



	uint32_t  events;
    if((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return PH_OSAL_ADD_COMPCODE(PH_OSAL_ERROR, PH_COMP_OSAL);
    }

	if((options & E_OS_EVENT_OPT_PEND_SET_ALL))
	{	
		/*if(options & E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT)
			{
				events = k_event_wait_all((k_event *)(*eventHandle), FlagsToWait, true, K_TICKS(ticksToWait));
			}
		else*/
			
			events = k_event_wait_all((struct k_event *)(*eventHandle), FlagsToWait, false, K_TICKS(ticksToWait));
			
	}
	else
		{
			/*if(options & E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT)
			{
				events = k_event_wait((k_event *)(*eventHandle), FlagsToWait, true, K_TICKS(ticksToWait));
			}
			else*/
			
			events = k_event_wait((struct k_event *)(*eventHandle), FlagsToWait, false, K_TICKS(ticksToWait));
			
		}

    if (events == 0) {
        // printk("No input devices are available!");
         status = PH_OSAL_IO_TIMEOUT;
    } 
    else {
        status = PH_OSAL_SUCCESS;
    }
    
    if(options & E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT)
    {
        k_event_clear((struct k_event *)(*eventHandle),FlagsToWait);
    }
    if (pCurrFlags != NULL)
    	{
    	    *pCurrFlags = events;
    	}
    return PH_OSAL_ADD_COMPCODE(status, PH_COMP_OSAL);
}

phStatus_t phOsal_EventPost(phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_EventBits_t FlagsToPost,
    phOsal_EventBits_t *pCurrFlags)
{
    if((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return PH_OSAL_ADD_COMPCODE(PH_OSAL_ERROR, PH_COMP_OSAL);
    }
#if 0
    /* Enter Critical Section */
    phOsal_EnterCriticalSection();

    /* Set the events. */
    (*((uint32_t *)(*eventHandle))) |= FlagsToPost;

    if (pCurrFlags != NULL)
    {
        *pCurrFlags = (*((uint32_t *)(*eventHandle)));
    }

    /* Exit Critical Section */
    phOsal_ExitCriticalSection();

    phOsal_WakeUp();
#endif

	uint32_t event_posted;
	 k_event_post((struct k_event *)(*eventHandle), FlagsToPost);
	 if (pCurrFlags != NULL)
	   {
            event_posted=k_event_test((struct k_event *)(*eventHandle), 0xffffffff);
		   *pCurrFlags = event_posted;
	   }

    return PH_OSAL_SUCCESS;

    }

phStatus_t phOsal_EventClear(phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_EventBits_t FlagsToClear,
    phOsal_EventBits_t *pCurrFlags)
{
    if((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return PH_OSAL_ADD_COMPCODE(PH_OSAL_ERROR, PH_COMP_OSAL);
    }
#if 0
    /* Enter Critical Section */
    phOsal_EnterCriticalSection();

    if (pCurrFlags != NULL)
    {
        *pCurrFlags = (*((uint32_t *)(*eventHandle)));
    }

    (*((uint32_t *)(*eventHandle))) &= (~FlagsToClear);

    /* Exit Critical Section. */
    phOsal_ExitCriticalSection();
#endif
uint32_t event_posted;
	
	 if (pCurrFlags != NULL)
	   {
            event_posted=k_event_test((struct k_event *)(*eventHandle), 0xffffffff);
		   *pCurrFlags = event_posted;
	   }
	k_event_clear((struct k_event *)(*eventHandle), FlagsToClear);

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_EventGet(phOsal_Event_t * eventHandle, phOsal_EventBits_t *pCurrFlags)
{
#if 0
    if (pCurrFlags == NULL)
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }
    else
    {
        *pCurrFlags = (*((uint32_t *)(*eventHandle)));
        return PH_OSAL_SUCCESS;
    }
#endif
return PH_OSAL_SUCCESS;

}

phStatus_t phOsal_EventDelete(phOsal_Event_t * eventHandle)
{
#if 0
    if((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return PH_OSAL_ADD_COMPCODE(PH_OSAL_ERROR, PH_COMP_OSAL);
    }

    gdwEventBitMap &= ~(1 << (((pphOsal_EventObj_t)eventHandle)->dwEventIndex));

    memset(eventHandle, 0, sizeof(phOsal_TimerObj_t));
#endif
    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_TimerCreate(phOsal_Timer_t *timerHandle, pphOsal_TimerObj_t timerObj)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_TimerStart(phOsal_Timer_t * timerHandle)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_TimerStop(phOsal_Timer_t * timerHandle)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_TimerGetCurrent(phOsal_Timer_t * timerHandle, uint32_t * pdwGetElapsedTime)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_TimerModify(phOsal_Timer_t *timerHandle, pphOsal_TimerObj_t timerObj)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_TimerDelete(phOsal_Timer_t * timerHandle)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadSecureStack(uint32_t stackSizeInNum)
{
   return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadDelete(phOsal_Thread_t * threadHandle)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadCreate(phOsal_Thread_t *threadHandle, pphOsal_ThreadObj_t threadObj, pphOsal_StartFunc_t startFunc, void *arg)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadChangePrio(phOsal_Thread_t * threadHandle, uint32_t newPrio)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadExit(void)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadDelay(phOsal_Ticks_t ticksToSleep)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_SemCreate(phOsal_Semaphore_t *semHandle, pphOsal_SemObj_t semObj,phOsal_SemOpt_t opt)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_SemPend(phOsal_Semaphore_t * semHandle, phOsal_TimerPeriodObj_t timePeriodToWait)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_SemPost(phOsal_Semaphore_t * semHandle, phOsal_SemOpt_t opt)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_SemDelete(phOsal_Semaphore_t * semHandle)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_MutexCreate(phOsal_Mutex_t *mutexHandle, pphOsal_MutexObj_t mutexObj)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_MutexLock(phOsal_Mutex_t * mutexHandle, phOsal_TimerPeriodObj_t timePeriodToWait)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_MutexUnLock(phOsal_Mutex_t * mutexHandle)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_MutexDelete(phOsal_Mutex_t * mutexHandle)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

void phOsal_StartScheduler(void)
{
    return ;
}

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */
static phStatus_t phOsal_NullOs_ReturnUnsupportedCmd(void)
{
    return (PH_OSAL_UNSUPPORTED_COMMAND | PH_COMP_OSAL);
}
/*
static phStatus_t phOsal_NullOs_GetFreeIndex(uint32_t * dwFreeIndex, uint32_t dwBitMap, uint32_t dwMaxLimit)
{
    phStatus_t status;

    (*dwFreeIndex) = 0;

    while(((1 << (*dwFreeIndex)) & dwBitMap) && ((*dwFreeIndex) < dwMaxLimit))
    {
        (*dwFreeIndex)++;
    }

    if (*dwFreeIndex == dwMaxLimit)
    {
        status = (PH_OSAL_ERROR | PH_COMP_OSAL);
    }
    else
    {
        status = PH_OSAL_SUCCESS;
    }

    return status;
}
*/

