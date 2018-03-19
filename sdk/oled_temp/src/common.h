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


typedef uint8_t bool;
#define true 1
#define false 0

enum DPAD {CENTER = 1, DOWN = 2, UP = 16, LEFT = 4, RIGHT = 8};

#define DEBOUNCE_DELAY 250000 // 0.25 seconds

#define TESTBIN_SIZE_16 16
#define TESTBIN_SIZE_64 64




/************************** Function Prototypes ******************************/

/********************************** OLED *************************************/
void print_screen(char* printLines[]);
bool confirmation_screen(XGpio* gpioBtn, char* printLines[]);
int selection_screen(XGpio* gpioBtn, char* printLines[], int numOfLines);
void format_cursor(char inputLine[], char *outputLine);
void refresh_oled (char* printLines[], int numOfLines, int index, bool menu);

/*********************************** SD *************************************/
bool init_sd(FATFS* fatfs);
bool format_sd(void);
void create_test_bin(int choice);
bool write_to_file(const char *sdFile, const uint8_t *writeBuf, const uint32_t writeSize);
bool read_from_file(const char *sdFile, uint8_t *readBuf, uint32_t *readSize);
void list_all_files(void);


#endif /* SRC_COMMON_H_ */
