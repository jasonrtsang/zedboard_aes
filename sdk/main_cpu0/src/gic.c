/*
 * gic.c
 *
 *  Created on: Mar 29, 2018
 *      Author: jason
 */

#include "common.h"

/******************************* Definitions *********************************/
#define BTN9 51 // MIO51 Push button
#define INTC_DEVICE_ID    XPAR_SCUGIC_SINGLE_DEVICE_ID
#define GPIO_INTERRUPT_ID XPS_GPIO_INT_ID

bool cancelFlag = false;
/*****************************************************************************/

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
**/
/*****************************************************************************/
void _XGpioPs_IntrHandler_EDIT(XGpioPs *InstancePtr)
{
    u8 Bank;
    u32 IntrStatus;
    u32 IntrEnabled;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    for (Bank = 0U; Bank < XGPIOPS_MAX_BANKS; Bank++) {
        IntrStatus = XGpioPs_IntrGetStatus(InstancePtr, Bank);
        // Use enabled status as well
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

/*****************************************************************************/
/**
*
* Interrupt handler for AES Process cancel interrupt BTN9
*
* @param    None
*
* @return   true if successful, false otherwise
*
* @note     None
*
**/
/*****************************************************************************/
void _gic_cancel_interrupt_handler(void *callBackRef, int bank, u32 status)
{
    XGpioPs *gpioIntr = (XGpioPs *)callBackRef;
    XGpioPs_IntrClearPin(gpioIntr, BTN9);
    // Set global cancel flag
    cancelFlag = true;
}

/*****************************************************************************/
/**
*
* Setup Gic controller and AES Process cancel interrupt BTN9
*
* @param    None
*
* @return   true if successful, false otherwise
*
* @note     None
*
**/
/*****************************************************************************/
bool gic_cancel_init(void)
{
	static XScuGic gicInstancePtr; // Gic Interrupt controller instance
	XScuGic_Config *intcConfig; // Interrupt controller for BTN9
	static XGpioPs gpioBtn; // Instance for BTN9
	XGpioPs_Config *gpioConfigPtr; // Configuration for BTN9

	/* MIO51 BTN9 Setup*/
	gpioConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	if(XGpioPs_CfgInitialize(&gpioBtn, gpioConfigPtr, gpioConfigPtr->BaseAddr) != XST_SUCCESS) {
#if UART_PRINT
		 print("MIO51 BTN9 INIT FAILED\n\r");
#endif
		 return false;
	}
	// Set direction of BTN9 as input
	XGpioPs_SetDirectionPin(&gpioBtn, BTN9, 0x0);

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	intcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

	XScuGic_CfgInitialize(&gicInstancePtr, intcConfig, intcConfig->CpuBaseAddress);

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, &gicInstancePtr);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	XScuGic_Connect(&gicInstancePtr, GPIO_INTERRUPT_ID, (Xil_ExceptionHandler)_XGpioPs_IntrHandler_EDIT, (void *)&gpioBtn);

	// Enable  interrupts for all the pins in bank 0
	XGpioPs_SetIntrTypePin(&gpioBtn, BTN9, XGPIOPS_IRQ_TYPE_EDGE_RISING);

	// Set the handler for GPIO interrupts
	XGpioPs_SetCallbackHandler(&gpioBtn, (void *)&gpioBtn, (XGpioPs_Handler)_gic_cancel_interrupt_handler);

	// Enable the GPIO interrupts of Bank 0
	XGpioPs_IntrEnablePin(&gpioBtn, BTN9);

	// Enable the interrupt for the GPIO device
	XScuGic_Enable(&gicInstancePtr, GPIO_INTERRUPT_ID);

	// Enable interrupts in the Processor
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);

	return true;
}
