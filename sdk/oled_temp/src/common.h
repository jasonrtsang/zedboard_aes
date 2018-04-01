/*
 * common.h
 *
 *  Created on: Mar 18, 2018
 *      Author: jason
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include "xil_types.h"
#include "xgpio.h"
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xgpiops.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xscugic.h"

#include <unistd.h>


/* SD Headers */
#include "ff.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xplatform_info.h"
#include "xsdps.h"

#include "aes.h"


typedef uint8_t bool;
#define true 1
#define false 0

enum DPAD {CENTER = 1, DOWN = 2, UP = 16, LEFT = 4, RIGHT = 8};

#define DEBOUNCE_DELAY 125000 // 0.25 seconds

#define TESTBIN_SIZE_16 16
#define TESTBIN_SIZE_64 64


/* MIO51 BTN9 Setup*/
#define XPAR_AXI_XADC_0_DEVICE_ID 0

#define GPIO_DEVICE_ID      XPAR_XGPIOPS_0_DEVICE_ID
#define INTC_DEVICE_ID      XPAR_SCUGIC_SINGLE_DEVICE_ID
#define GPIO_INTERRUPT_ID   XPS_GPIO_INT_ID

#define pbsw 51


/************************** Function Prototypes ******************************/

/********************************** OLED *************************************/
void print_screen(char* printLines[]);
bool confirmation_screen(XGpio* gpioBtn, char* printLines[]);
int selection_screen(XGpio* gpioBtn, char* printLines[], int numOfLines);
char** format_fileList(char* inputFiles[], int numOfFiles);
void format_cursor(char inputLine[], char *outputLine);
void refresh_oled (char* printLines[], int numOfLines, int index, bool menu);

/*********************************** SD *************************************/
bool init_sd(FATFS* fatfs);
bool format_sd(void);
bool create_test_bin(int choice);
bool write_to_file(const char *sdFile, const uint8_t *writeBuf, const uint32_t writeSize);
bool read_from_file(const char *sdFile, uint8_t *readBuf, uint32_t *readSize);
char** list_all_files(int* numOfFiles);

/**************************** *** INTERRUPT *********************************/
void My_XGpioPs_IntrHandler(XGpioPs *InstancePtr);
void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio, u16 GpioIntrId);
void IntrHandler(void *CallBackRef, int Bank, u32 Status);


#endif /* SRC_COMMON_H_ */