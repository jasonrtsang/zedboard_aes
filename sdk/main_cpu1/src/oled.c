/*
 * oled.c
 *
 *  Created on: Mar 18, 2018
 *      Author: jason
 */

#include "common.h"
#include "ZedboardOLED.h"

/*****************************************************************************/
/**
*
* Clear and print OLED with menu or message
*
* @param    char* printLines[]        : Lines to display
*           int numOfLines            : Lines within printLines
*                                       the title of the menu
*           int index                 : Cursor position, set to 0 when printing
*                                       just a message
*           bool menu                 : Menu display
*
* @return   None
*
* @note     Expects each line to be 16 char aligned
*           Menu option lines are 2 space offset at front for the cursor
*           First Item in menu buffer is menu title
*
**/
/*****************************************************************************/
void _oled_refresh_oled(char* printLines[], int numOfLines, int index) {

	clear();
	if(index < numOfLines) {
		print_message(printLines[index], 0);
	}
	if(index+1 < numOfLines) {
		print_message(printLines[index+1], 1);
	}
	if(index+2 < numOfLines) {
		print_message(printLines[index+2], 2);
	}
	if(index+3 < numOfLines) {
		print_message(printLines[index+3], 3);
	}
}

/*****************************************************************************/
/**
*
* Clears the OLED screen
*
* @param    None
*
* @return   None
*
* @note     None
*
**/
/*****************************************************************************/
void oled_clear(void) {
	clear();
}

/*****************************************************************************/
/**
*
* Print message to OLED screen
*
* @param    char* printLines[]        : Message to print
*
* @return   None
*
* @note     Expects each line to be 16 char aligned
* 			Expects 4 lines in input
*
**/
/*****************************************************************************/
void oled_print_screen(char* printLines[]) {
	_oled_refresh_oled(printLines, 4, 0);
}

/*****************************************************************************/
/**
*
* Print line to OLED screen
*
* @param    char* printLine[]         : Line to print
*
* @return   None
*
* @note     Expects line to be 16 char aligned
*
**/
/*****************************************************************************/
void oled_print_line(char* printLine, int index) {
	print_message(printLine, index);
}
