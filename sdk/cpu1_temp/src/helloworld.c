/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include <xgpio.h>


#include "xparameters.h"
#include <stdio.h>
#include "xil_io.h"
#include "xil_mmu.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "sleep.h"


#define INTC		    XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#define INTC_DEVICE_ID	XPAR_PS7_SCUGIC_0_DEVICE_ID
#define PL_IRQ_ID       XPS_IRQ_INT_ID

#define IRQ_PCORE_GEN_BASE  XPAR_IRQ_GEN_0_BASEADDR


#define COMM_VAL    (*(volatile unsigned long *)(0xFFFF0000))

/**
 * This typedef contains configuration information for the device driver.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID of device */
	u32 BaseAddress;	/**< Base address of the device */
} Pl_Config;


/**
 * The driver instance data. The user is required to allocate a
 * variable of this type.
 * A pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	Pl_Config Config;   /**< Hardware Configuration */
	u32 IsReady;		/**< Device is initialized and ready */
	u32 IsStarted;		/**< Device is running */
} XPlIrq;


extern u32 MMUTable;

int main()
{
	XGpio gpioLeds;


	 //Disable cache on OCM
	Xil_SetTlbAttributes(0xFFFF0000,0x14de2);           // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0


	print("CPU1: init_platform\n\r");


	 while(1){


		print("CPU1: Hello World CPU 1\n\r");
		COMM_VAL = 0;
		while(COMM_VAL == 0);

	 }




























    init_platform();

	if(XGpio_Initialize(&gpioLeds, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) {
		printf("UH OH: GPIO2 LEDS initialization failed\r\n");
	};
	XGpio_SetDataDirection(&gpioLeds, 2, 0x0000);
	while(1) {
		XGpio_DiscreteWrite(&gpioLeds, 2, 0x00);
		usleep(500000);
		XGpio_DiscreteWrite(&gpioLeds, 2, 0xFF);
		usleep(500000);
	}

    cleanup_platform();
    return 0;
}
