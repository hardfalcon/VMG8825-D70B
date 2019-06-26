/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !

                This is only an

               E X A M P L E  (vxWorks)

            how to setup and use the wrap of
         the VRX driver interrupt functions.

   ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !  */

/* ============================================================================

   HOW DOES IT WORK:
      Here we setup wrapper functions for the intConnect(), intEnable() and
      intDisable() functions.

      Note:
         For intConnect the driver calls the local MEI_IfxIntConnect()
         function. Here the wrapped intConnect() fucntion will be called and
         also a lock will be set to block further changes after the interrupt
         functions are in use.

      After setup this local interrupt functions the VRX driver will use
      it for interrupt handling.

      Remember:
         Within the local MEI_IfxIntConnect() the VRX driver calls
         the user-intConnect() function with the VRX driver origional data
         (Interrupt Service Routine (ISR) + ISR PARAMS). So the user-intConnect
         has here the chance to catch the driver ISR and parameter.

      The example MEI_MyIntConnect_Wrap() function catch and save
      the VRX driver internal Interrupt Service Routine (ISR) and
      the VRX driver internal ISR parameters for later use within the
      example-ISR MEI_MyInterruptVxWorks() and after this it registers the
      example ISR and the example params via intConnect() to the
      vxWorks system.

      So if an interrupt occured the example-ISR can do his user-specific stuff
      and additional the origional VRX driver ISR has to be called.

      SEE:
         MEI_IfxIntConnect(), MEI_MyInterruptVxWorks()

   ========================================================================= */


/* ============================================================================
   Description : This file contains the defines specific to the vxWorks
                 interrupt wrapper functions.
   Remarks     : !!!! this is only a example code how to setup and use the
                      interrupt wrap.
   ========================================================================= */

#warning THIS_IS_ONLY_AN_EXAMPLE_HOW_TO_WRAP_THE_INTERRUPT_FUNCTIONS

/* ============================================================================
   Inlcudes
   ========================================================================= */
#include <vxworks.h>
#include "intLib.h"
#include "logLib.h"
#include "stdio.h"

/* get the VRX driver interrupt wrapper functions */
#include "drv_mei_cpe_vxworks.h"
#include "mei_cpe_drv_examp_int_wrap_vxw.h"


/* ============================================================================
   Example: interrupt functions (vxWorks) - declaration
   ========================================================================= */
LOCAL void MEI_MyInterruptVxWorks(int ISRParams);
LOCAL STATUS MEI_MyIntConnect_Wrap( VOIDFUNCPTR *pIntVector,
                                      VOIDFUNCPTR pISRRoutine,
                                      int ISRParams);
LOCAL int MEI_MyIntEnable_Wrap(int IRQNum);
LOCAL int MEI_MyIntDisable_Wrap(int IRQNum);


/* ============================================================================
   Example: local varibales
   ========================================================================= */

/* flag to block printout after the first call */
int MEI_MyIntIsrPrintLock = 0;

/*
   Save the original intConnect data - support 8 IRQ's
   Note: MPC expects internal the IRQ * 2
         HW IRQ line 7  --> IRQ 14
*/
LOCAL MEI_MyIsrParams_t MEI_MyIsrParams[8] =
{
   {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0},
   {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}
};


/* ============================================================================
   Example: Local interrupt functions (vxWorks) - definition
   ========================================================================= */

/*
   Wrap the VRX original ISR
   - do own stuff and also call the origional VRX Drv ISR
*/
LOCAL void MEI_MyInterruptVxWorks(int ISRParams)
{
   MEI_MyIsrParams_t *pMyParams = (MEI_MyIsrParams_t *)ISRParams;

   if (!MEI_MyIntIsrPrintLock)
   {
      /* do printout only once */
      MEI_MyIntIsrPrintLock++;
      logMsg("INT wrap - My ISR\r\n");
   }

   /*
      pre: do own stuff here
   */

   /* call the original ISR */
   if (pMyParams->MEI_OrgDrvIsr != NULL)
   {
      pMyParams->MEI_OrgDrvIsr(pMyParams->MEI_OrgIsrParams);
   }
   else
   {
      logMsg("ERROR: INT wrap - missing original ISR\r\n");
   }

   /*
      post: do own stuff here
   */

   return;
}


/*
   Wrap the VxWorks intConnect routine.

   The VRX driver still calls this function with the
   original VRX ISR and PARAMS.

   So here we catch the origional VRX driver informations for later use !!!

*/
LOCAL STATUS MEI_MyIntConnect_Wrap( VOIDFUNCPTR *pIntVector,
                                      VOIDFUNCPTR pISRRoutine,
                                      int ISRParams)
{
   STATUS ret;
   int IrqIdx = ((int)pIntVector) / 2;

   if (IrqIdx > 8)
   {
      printf( "ERROR: INT Wrap the intConnect - invalid IRQ %d (max 16)\r\r",
              (int)pIntVector);

      return ERROR;
   }

   printf("INT Wrap - intConnect\r\n");

   /* save the original VRX ISR and call it later */
   MEI_MyIsrParams[IrqIdx].MEI_OrgDrvIsr    = pISRRoutine;
   MEI_MyIsrParams[IrqIdx].MEI_OrgIsrParams = ISRParams;

   /*
      pre: do own stuff here
   */

   /*
      now it is time for the intConnect with the own params and ISR
   */
   ret = intConnect( pIntVector,
                     MEI_MyInterruptVxWorks,
                     (int)&MEI_MyIsrParams[IrqIdx]);


   /*
      post: do own stuff here
   */


   return ret;
}


/*
   Wrap the VxWorks intEnable routine
*/
LOCAL int MEI_MyIntEnable_Wrap(int IRQNum)
{
   int ret;
   printf("INT Wrap - intEnable\r\n");

   /*
      pre: do own stuff here
   */


   ret = intEnable(IRQNum);

   /*
      post: do own stuff here
   */

   return ret;
}


/*
   Wrap the VxWorks intDisable routine
*/
LOCAL int MEI_MyIntDisable_Wrap(int IRQNum)
{
   int ret;
   printf("INT Wrap - intDisable\r\n");

   /*
      pre: do own stuff here
   */

   ret = intDisable(IRQNum);

   /*
      post: do own stuff here
   */

   return ret;
}


/* ============================================================================
   Example: Global interrupt functions (vxWorks) - definition
   ========================================================================= */

/*
   Setup the interrupt wrap functions.

   Note:
   - You should setup the interrupt wrapping at the beginning
     (best: after the MEI_DevCreate() call).
   - Once you have called the intConnect() function no changes within the
     interrupt wrapper functions are possible
   - The intConnect() will be called while initialisation.
     The ioctl(.., FIO_MEI_DEV_INIT, ..) does the IRQ assignment.

   Remark:
      When the VRX driver do the intConnect it will increment an internal
      lock variable. Changes of the interrupt wrap functions are
      only possible if these lock is 0.
      Means: after the first intConnect() the update of the wrapper functions
      is blocked.
*/

int MEI_MyIntWrapSetup()
{

   /* init interrupt wrapper functions */
   if (MEI_FctIntConnectSet( MEI_MyIntConnect_Wrap ) != OK)
   {
      printf("ERROR: setup IntWrapper - set intConnect\r\n");
      goto MEI_MY_INT_WRAP_SETUP_ERROR;
   }

   /* init interrupt wrapper functions */
   if (MEI_FctIntEnableSet( MEI_MyIntEnable_Wrap ) != OK)
   {
      printf("ERROR: setup IntWrapper - set intEnable\r\n");
      goto MEI_MY_INT_WRAP_SETUP_ERROR;
   }

   /* init interrupt wrapper functions */
   if (MEI_FctIntDisableSet( MEI_MyIntDisable_Wrap ) != OK)
   {
      printf("ERROR: setup IntWrapper - set intDisable\r\n");
      goto MEI_MY_INT_WRAP_SETUP_ERROR;
   }

   return 0;

MEI_MY_INT_WRAP_SETUP_ERROR:

   return -1;
}


