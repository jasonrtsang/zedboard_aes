/*
 * common.h
 *
 *  Created on: Mar 18, 2018
 *      Author: jason
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <stdio.h>
#include "xil_printf.h"
#include "platform_config.h"

#include "sleep.h"
#include <xgpio.h>
#include "xparameters.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "xil_mmu.h"

typedef uint8_t bool;
#define true 1
#define false 0

// CPU0
#define COMM_VAL     (*(volatile unsigned long *)(0xFFFF0000))
#define FILESIZE_VAL (*(volatile unsigned long *)(0xFFFF0004))

/********************************** OLED *************************************/
void oled_clear(void);
void oled_print_screen(char* printLines[]);
void oled_print_line(char* printLine, int index);

#endif /* SRC_COMMON_H_ */
