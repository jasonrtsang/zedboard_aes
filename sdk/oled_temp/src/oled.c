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
* Print message to OLED screen
*
* @param    char* printLines[]        : Message to print
*
* @return   None
*
* @note     Expects each line to be 16 char aligned
* 			Expects 4 lines in input
*
******************************************************************************/
void print_screen(char* printLines[]) {
	refresh_oled(printLines, 4, 0, false);
}

/*****************************************************************************/
/**
*
* Print confirmation message to OLED screen, and wait for confirmation
*
* @param    XGpio* gpioBtn            : Pointer to D-pad GPIO instantiated
*           char* printLines[]        : Message to print
*
* @return   None
*
* @note     Expects each line to be 16 char aligned
* 			Expects 4 lines in input
*
******************************************************************************/
bool confirmation_screen(XGpio* gpioBtn, char* printLines[]) {
	enum DPAD dpadClick;

	print_screen(printLines);

	while(1) {
		dpadClick = XGpio_DiscreteRead(gpioBtn, 1);
		if(dpadClick == CENTER) {
			usleep(DEBOUNCE_DELAY);
			return true;
		} else if(dpadClick == LEFT) {
			usleep(DEBOUNCE_DELAY);
			return false;
		}
	}
}

/*****************************************************************************/
/**
*
* Print Menu to OLED screen, and manage selection refresh
*
* @param    XGpio* gpioBtn            : Pointer to D-pad GPIO instantiated
*           char* menuLines[]         : Menu message, expects 1st entry to be
*                                       the title of the menu
*           int numOfLines            : Number of items within menuLunes
*
* @return   int, index of the cursor within menuLines
*
* @note     None
*
******************************************************************************/
int selection_screen(XGpio* gpioBtn, char* menuLines[], int numOfLines) {
	int i = 1; // 1st line is menu title
	bool exitMenu = false;
	enum DPAD dpadClick;

	refresh_oled(menuLines, numOfLines, i, true);

	while(!exitMenu)
	{
		dpadClick = XGpio_DiscreteRead(gpioBtn, 1);
		switch (dpadClick) {
			case CENTER:
				exitMenu = true;
				break;
			case DOWN:
				clear();
				if(i >= 1 && i < numOfLines-1) {
					i+=1;
				}
				refresh_oled(menuLines, numOfLines, i, true);
				usleep(DEBOUNCE_DELAY);
				break;
			case UP:
				clear();
				if(i > 1 && i < numOfLines) {
					i-=1;
				}
				refresh_oled(menuLines, numOfLines, i, true);
				usleep(DEBOUNCE_DELAY);
				break;
			case LEFT:
				return 0;
			default:
				break;
		}
	}

	return i;
}

/*****************************************************************************/
/**
*
* Replaces first whitespace with * cursor
*
* @param    char inputLine[]         : Line content of cursor
*           char *outputLine         : Line formatted with cursor
*
* @return   None
*
* @note     Expects each line to be 16 char aligned
*
******************************************************************************/
char** format_fileList(char* inputFiles[], int numOfFiles) {
   int i;
   char* fileList_title = "Files on SD:    ";

   char** outputFiles = malloc((numOfFiles+1) * sizeof(char *));

   outputFiles[0] = malloc(sizeof(fileList_title) * sizeof(char));
   strcpy(outputFiles[0], fileList_title);

   for(i = 0; i < numOfFiles; i++) {
	   outputFiles[i+1] = malloc(sizeof(inputFiles[i]) * sizeof(char));
	   strcpy(outputFiles[i+1], " ");
	   strcpy(outputFiles[i+1]+1, " ");
	   strcpy(outputFiles[i+1]+2, inputFiles[i]);
   }

  return outputFiles;
}

/*****************************************************************************/
/**
*
* Replaces first whitespace with * cursor
*
* @param    char inputLine[]         : Line content of cursor
*           char *outputLine         : Line formatted with cursor
*
* @return   None
*
* @note     Expects each line to be 16 char aligned
*
******************************************************************************/
void format_cursor(char inputLine[], char *outputLine) {
   int i = 1, length = 0;

   outputLine[0] = '*';

   while (inputLine[length] != '\0') {
	   length++;
   }

   while (i < 15 && i < length) {
	   outputLine[i] = inputLine[i];
	   i++;
   }
}

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
******************************************************************************/
void refresh_oled(char* printLines[], int numOfLines, int index, bool menu) {
	char formattedLine[16];

	clear();
	if(menu) {
		print_message(printLines[0], 0);
		if (index-1 >= 1 && index-1 < numOfLines) {
			print_message(printLines[index-1], 1);
		}
		if (index < numOfLines) {
			format_cursor(printLines[index], formattedLine);
			print_message(formattedLine, 2);
		}
		if (index+1 < numOfLines) {
			print_message(printLines[index+1], 3);
		}
	} else {
		if (index < numOfLines) {
			print_message(printLines[index], 0);
		}
		if (index+1 < numOfLines) {
			print_message(printLines[index+1], 1);
		}
		if (index+2 < numOfLines) {
			print_message(printLines[index+2], 2);
		}
		if (index+3 < numOfLines) {
			print_message(printLines[index+3], 3);
		}

	}
}
