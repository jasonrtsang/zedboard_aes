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

#define NUMINLIST 5

int main(void){
	static XGpio GPIOInstance_Ptr;
	int xStatus;
	u32 Readstatus=0,OldReadStatus=0;
	int exit_flag,choice,internal_choice;
	init_platform();

	char *test_array[6];
	char *print_index;

	test_array[0] = "ALPHA";
	test_array[1] = "BETA";
	test_array[2] = "CORSA";
	test_array[3] = "DOOR";
	test_array[4] = "EHHHH";
	test_array[5] = "FFUCK";


	int i;
	int wrapAround;

	print("##### Application Starts #####\n\r");
	print("\r\n");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Step-1 :AXI GPIO Initialization
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	xStatus = XGpio_Initialize(&GPIOInstance_Ptr,XPAR_BTN_GPIO_AXI_DEVICE_ID);
	if(XST_SUCCESS != xStatus)
	print("GPIO INIT FAILED\n\r");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Step-2 :AXI GPIO Set the Direction
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	XGpio_SetDataDirection(&GPIOInstance_Ptr, 1,1);

	clear();

	i = 0;

	if (i < NUMINLIST) {
		print_message(test_array[i], 1);
	}
	if (i+1 < NUMINLIST) {
		print_message(test_array[i+1], 2);
	}
	if (i+2 < NUMINLIST) {
		print_message(test_array[i+2], 3);
	}

	while(1)
	{
		Readstatus = XGpio_DiscreteRead(&GPIOInstance_Ptr, 1);
		switch (Readstatus) {
			case 4: // left
//				clear();
//				print_message("left",0);
				break;
			case 16: // up
				clear();
				if(i > 0 && i < NUMINLIST) {
					i-=1;
				}
				if (i < NUMINLIST) {
					print_message(test_array[i], 1);
				}
				if (i+1 < NUMINLIST) {
					print_message(test_array[i+1], 2);
				}
				if (i+2 < NUMINLIST) {
					print_message(test_array[i+2], 3);
				}
				usleep(250000);
				print_message("up",0);
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
				if (i < NUMINLIST) {
					print_index = "> " + test_array[i];
					print_message(test_array[i], 1);
				}
				if (i+1 < NUMINLIST) {
					print_message(test_array[i+1], 2);
				}
				if (i+2 < NUMINLIST) {
					print_message(test_array[i+2], 3);
				}
				usleep(250000); // 0.25 seconds delay
				print_message("down",0);
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


