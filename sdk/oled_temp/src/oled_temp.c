/*
 * oled_temp.c
 *
 *  Created on: Mar 17, 2018
 *      Author: jrtsang
 */
#include "ZedboardOLED.h"

#include <stdio.h>
#include "platform.h"
#include "xil_types.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xgpiops.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xscugic.h"

int main(void){
	static XGpio GPIOInstance_Ptr;
	int xStatus;
	u32 Readstatus=0,OldReadStatus=0;
	int exit_flag,choice,internal_choice;
	init_platform();

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

	while(1)
	{
	Readstatus = XGpio_DiscreteRead(&GPIOInstance_Ptr, 1);
	printf("0x%x\n", Readstatus);
	}


	clear();

	print_message("Texas A&M Qatar",0);
	print_message("ECEN Department",2);
	print_message(" ECEN449 ",3);


	return (1);

}


