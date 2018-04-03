/*
 * main.c
 *
 *  Created on: Apr 3, 2018
 *      Author: jrtsang
 */

#include "common.h"

/******************************* Definitions *********************************/
#define COMM_VAL     (*(volatile unsigned long *)(0xFFFF0000))
#define FILESIZE_VAL (*(volatile unsigned long *)(0xFFFF0004))

#define LED_DELAY 125000 // 0.25 seconds
XGpio gpioLeds;

typedef uint8_t bool;
#define true 1
#define false 0
/*****************************************************************************/

/*****************************************************************************/
/**
*
* Initialize all drivers
*
* @param    None
*
* @return   None
*
* @note     None
*
**/
/*****************************************************************************/
void _main_initialization(void) {

	// Init LEDs
	if(XGpio_Initialize(&gpioLeds, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) {
	#if UART_PRINT
			printf("UH OH: GPIO2 LEDS initialization failed\r\n");
	#endif
	};
	// Set LEDs as output, GPIO pins 2 on HW block
	XGpio_SetDataDirection(&gpioLeds, 2, 0x0000);

	// Inter-processor setup with CPU0
	COMM_VAL = 0;
	// Disable cache on OCM
	Xil_SetTlbAttributes(0xFFFF0000, 0x14de2); // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0
}

/*****************************************************************************/
/**
*
* Entrance to CPU0 Application
*
* @param    None
*
* @return   0 when exit
*
* @note     None
*
**/
/*****************************************************************************/
int main()
{
	_main_initialization();

	char* processingScreen[] = {"  Seconds Left  ",
							    "                ",
							    "  % Completed   ",
							    "                "};
	// Progress variables
	int secondsLeft;
	int percentageComplete;
	int percentageIncrements;
	uint8_t ledLocation;
	int ledLoopCounter;
	bool firstLoop;
	// Print line buffer
	char printBuf[16];
	// Toggle all LEDs on bootup
	XGpio_DiscreteWrite(&gpioLeds, 2, 0xFF);
	usleep(LED_DELAY);

	while(1){

		// Turn off all LEDs
		XGpio_DiscreteWrite(&gpioLeds, 2, 0x00);

		// Trap when processing not on
		while(COMM_VAL == 0){
			firstLoop = true;
		}
		// First kick off initializations
		if(firstLoop) {
			percentageComplete = 0;
			secondsLeft = FILESIZE_VAL / 0x280000; // 10mb A0 0000 takes 4 seconds = 20 0000 per second
			percentageIncrements = 100 / secondsLeft;
			ledLocation = 128; // 0b10000000
			ledLoopCounter = 0;
			firstLoop = false;
		}

		// Print progress screen
		oled_print_screen(processingScreen);
		snprintf(printBuf, sizeof(printBuf), "      ~ %i", secondsLeft);
		oled_print_line(printBuf, 1);
		snprintf(printBuf, sizeof(printBuf), "      ~ %i", percentageComplete);
		oled_print_line(printBuf, 3);

		// Loop 8 LEDS - 1 seconds loop
		while(COMM_VAL ==1 && ledLoopCounter < 8) {
			ledLocation = ledLocation >> 1;
			XGpio_DiscreteWrite(&gpioLeds, 2, ledLocation);
			usleep(LED_DELAY);
			ledLoopCounter++;
		}
		// Reset back to start
		ledLocation = 128;
		ledLoopCounter = 0;

		// Update progress variables
		secondsLeft--;
		percentageComplete += percentageIncrements;
	}

    return 0; // Shouldn't reach here
}
