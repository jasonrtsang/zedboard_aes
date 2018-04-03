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
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "xil_mmu.h"
#include "sleep.h"

#include "common.h"

#include "ZedboardOLED.h"

#define COMM_VAL    (*(volatile unsigned long *)(0xFFFF0000))
#define FILESIZE_VAL    (*(volatile unsigned long *)(0xFFFF0004))

#define LED_DELAY 125000 // 0.25 seconds

int main()
{
	XGpio gpioLeds;
	if(XGpio_Initialize(&gpioLeds, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) {
		printf("UH OH: GPIO2 LEDS initialization failed\r\n");
	};
	XGpio_SetDataDirection(&gpioLeds, 2, 0x0000);

	 //Disable cache on OCM
	Xil_SetTlbAttributes(0xFFFF0000,0x14de2);           // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0

	COMM_VAL = 0;

	XGpio_DiscreteWrite(&gpioLeds, 2, 0xFF);
	usleep(LED_DELAY);
	XGpio_DiscreteWrite(&gpioLeds, 2, 0x00);

	int counter = 0;
	bool firstLoop = false;
	int secondsLeft = 0;
	int percentageComplete = 0;
	int percentageIncrements = 0;

	char printBuf[16];

	char* processingScreen[] = {"  Seconds Left  ",
							    "                ",
							    "  % Completed   ",
							    "                "};

	while(1){

		XGpio_DiscreteWrite(&gpioLeds, 2, 0b00000000);

		while(COMM_VAL == 0){
			firstLoop = true;
		}
		if(firstLoop) {
			percentageComplete = 0;
			secondsLeft = FILESIZE_VAL/0x280000; // 10mb A0 0000 takes 4 seconds = 20 0000 per second
			percentageIncrements = 100 / secondsLeft;
			firstLoop = false;
		}

		print_screen(processingScreen);
		snprintf(printBuf, sizeof(printBuf), "      ~ %i", secondsLeft);
		print_message(printBuf, 1);
		snprintf(printBuf, sizeof(printBuf), "      ~ %i", percentageComplete);
		print_message(printBuf, 3);


		// Better loop this so when a cancel occurs it will check COMM_VAL right away

		// Loop 8 LEDS - 1 seconds loop
		XGpio_DiscreteWrite(&gpioLeds, 2, 0b10000000);
		usleep(LED_DELAY);
		XGpio_DiscreteWrite(&gpioLeds, 2, 0b01000000);
		usleep(LED_DELAY);
		XGpio_DiscreteWrite(&gpioLeds, 2, 0b00100000);
		usleep(LED_DELAY);
		XGpio_DiscreteWrite(&gpioLeds, 2, 0b00010000);
		usleep(LED_DELAY);
		XGpio_DiscreteWrite(&gpioLeds, 2, 0b00001000);
		usleep(LED_DELAY);
		XGpio_DiscreteWrite(&gpioLeds, 2, 0b00000100);
		usleep(LED_DELAY);
		XGpio_DiscreteWrite(&gpioLeds, 2, 0b00000010);
		usleep(LED_DELAY);
		XGpio_DiscreteWrite(&gpioLeds, 2, 0b00000001);
		usleep(LED_DELAY);
		secondsLeft--;
		percentageComplete += percentageIncrements;


	}

    return 0;
}
