/*
 * gic.c
 *
 *  Created on: Mar 29, 2018
 *      Author: jason
 */

#include "common.h"

/*****************************************************************************/
/**
*
* ORIGINALLY FROM XGpioPs_IntrHandler (xgpios_intr.c), EDITTED TO REDUCE BOUNCE
*
* This function is the interrupt handler for GPIO interrupts.It checks the
* interrupt status registers of all the banks to determine the actual bank in
* which an interrupt has been triggered. It then calls the upper layer callback
* handler set by the function XGpioPs_SetBankHandler(). The callback is called
* when an interrupt
*
* @param	InstancePtr is a pointer to the XGpioPs instance.
*
* @return	None.
*
* @note		This function does not save and restore the processor context
*		such that the user must provide this processing.
*
******************************************************************************/
void My_XGpioPs_IntrHandler(XGpioPs *InstancePtr)
{
    u8 Bank;
    u32 IntrStatus;
    u32 IntrEnabled;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    for (Bank = 0U; Bank < XGPIOPS_MAX_BANKS; Bank++) {
        IntrStatus = XGpioPs_IntrGetStatus(InstancePtr, Bank);
        IntrEnabled = XGpioPs_IntrGetEnabled(InstancePtr, Bank);

        if ((IntrStatus != (u32)0) && (IntrEnabled != (u32)0)) {

            XGpioPs_IntrClear((XGpioPs *)InstancePtr, Bank,
                            (IntrStatus & IntrEnabled));
            InstancePtr->Handler(InstancePtr->
                         CallBackRef, Bank,
                         (IntrStatus & IntrEnabled));
        }
    }
}

void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio, u16 GpioIntrId)
{
        XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */
        Xil_ExceptionInit();

        /*
         * Initialize the interrupt controller driver so that it is ready to
         * use.
         */
        IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

        XScuGic_CfgInitialize(GicInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);

        /*
         * Connect the interrupt controller interrupt handler to the hardware
         * interrupt handling logic in the processor.
         */
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, GicInstancePtr);

        /*
         * Connect the device driver handler that will be called when an
         * interrupt for the device occurs, the handler defined above performs
         * the specific interrupt processing for the device.
         */
        XScuGic_Connect(GicInstancePtr, GpioIntrId, (Xil_ExceptionHandler)My_XGpioPs_IntrHandler, (void *)Gpio);

        //Enable  interrupts for all the pins in bank 0.
        XGpioPs_SetIntrTypePin(Gpio, pbsw, XGPIOPS_IRQ_TYPE_EDGE_RISING);

        //Set the handler for gpio interrupts.
        XGpioPs_SetCallbackHandler(Gpio, (void *)Gpio, (XGpioPs_Handler)IntrHandler);

        //Enable the GPIO interrupts of Bank 0.
        XGpioPs_IntrEnablePin(Gpio, pbsw);

        //Enable the interrupt for the GPIO device.
        XScuGic_Enable(GicInstancePtr, GpioIntrId);

        // Enable interrupts in the Processor.
        Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
}

void IntrHandler(void *CallBackRef, int Bank, u32 Status)
{
    XGpioPs *Gpioint = (XGpioPs *)CallBackRef;
    XGpioPs_IntrClearPin(Gpioint, pbsw);
    cancelFlag = true;
}
