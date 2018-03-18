/*
 * oled_temp.c
 *
 *  Created on: Mar 17, 2018
 *      Author: jrtsang
 */
#include "ZedboardOLED.h"
#include <unistd.h>

#include <stdio.h>
#include "platform.h"
#include "xil_types.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xgpiops.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xscugic.h"

#define NUMINLIST 6

typedef uint8_t bool;
#define true 1
#define false 0


/************************** Function Prototypes ******************************/
void refresh_oled (char* menuLine, char* printLines[], int numOfLines, int index);

void format_line(char inputLine[], char *outputLine, bool cursor);




int main(void){
	static XGpio gpioBtn;
	int btnClick, i = 0;

	char* string_fileSelection = "File Selection:";

	char* test_array[NUMINLIST];
	test_array[0] = "ALPHA";
	test_array[1] = "BETA";
	test_array[2] = "CORSA";
	test_array[3] = "DOOR";
	test_array[4] = "EHHHH";
	test_array[5] = "FFUCK";


/* START */
	print("##### Application Starts #####\r\n");
	/* Initialization */
	init_platform();
	/* D-pad buttons */
	if (XST_SUCCESS != XGpio_Initialize(&gpioBtn, XPAR_BTN_GPIO_AXI_DEVICE_ID)) {
		printf("UH OH: BTN5 GPIO initialization failed\r\n");
	}
	XGpio_SetDataDirection(&gpioBtn, 1, 1);

	/* Initial screen */
	clear();
	print_message("File Selection", 0);
	if (i < NUMINLIST) {
		print_message(test_array[i], 2);
	}
	if (i+1 < NUMINLIST) {
		print_message(test_array[i+1], 3);
	}
	if (i+2 < NUMINLIST) {
		print_message(test_array[i+2], 3);
	}

	while(1)
	{
		btnClick = XGpio_DiscreteRead(&gpioBtn, 1);
		switch (btnClick) {
			case 4: // left
//				clear();
//				print_message("left",0);
				break;
			case 16: // up
				clear();
				if(i > 0 && i < NUMINLIST) {
					i-=1;
				}
				refresh_oled(string_fileSelection, test_array, NUMINLIST, i);
				usleep(250000);
				break;
			case 8: // right
//				clear();
//				print_message("right",0);
				break;
			case 2: // down
				clear();
				if(i >= 0 && i < NUMINLIST-1) {
					i+=1;
				}
				refresh_oled(string_fileSelection, test_array, NUMINLIST, i);

				usleep(250000); // 0.25 seconds delay
				break;
			case 1: // center
//				clear();
//				print_message("center",0);
				break;
			default:
				break;
		}
	}
	return (1);
}

void refresh_oled (char* menuLine, char* printLines[], int numOfLines, int index) {

	char* cursorLine = printLines[index];
	char* printLine;
	char formattedLine[16];

	clear();

	print_message(menuLine, 0);

	if (index < numOfLines) {
		format_line(cursorLine, formattedLine, true);
		print_message(formattedLine, 1);
	}
	if (index+1 < numOfLines) {
		printLine = printLines[index+1];
		format_line(printLine, formattedLine, false);
		print_message(formattedLine, 2);
	}
	if (index+2 < numOfLines) {
		printLine = printLines[index+2];
		format_line(printLine, formattedLine, false);
		print_message(formattedLine, 3);
	}
}

void format_line(char inputLine[], char *outputLine, bool cursor) {
   int i = 0, length = 0;

   if (cursor) {
	   outputLine[0] = '>';
	   outputLine[1] = ' ';
   } else {
	   outputLine[0] = ' ';
	   outputLine[1] = ' ';
   }

   while (inputLine[length] != '\0') {
	   length++;
   }

   while (i < 14 && i < length) {
	   outputLine[i+2] = inputLine[i];
	   i++;
   }
   outputLine[i+2] = '\0';
}

