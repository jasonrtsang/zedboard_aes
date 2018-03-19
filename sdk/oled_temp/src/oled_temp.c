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

#define DEBOUNCE_DELAY 250000 // 0.25 seconds

typedef uint8_t bool;
#define true 1
#define false 0

enum DPAD {CENTER = 1, DOWN = 2, UP = 16, LEFT = 4, RIGHT = 8};
static XGpio gpioBtn;


/************************** Function Prototypes ******************************/
void refresh_oled (char* printLines[], int numOfLines, int index, bool menu);
void format_cursor(char inputLine[], char *outputLine);
int selection_screen(char* printLines[], int numOfLines);
void print_screen(char* menuLines[]);

int main(void){
	int choice = 0;
	bool startScreen = false;
	bool confirmation = false;

	/* ENSURE LENGTHS OF 16 PER LINE */
	char* mainMenu[] = {"Main Menu:      ",
						"  Reformat      ",
						"  Test bin      ",
						"  ECB mode      ",
						"  CBC mode      ",
						"  Exit          "};
	char* testBinMenu[] = {"Choose BIN size:",
			               "  16 bytes      ",
						   "  64 bytes      "};
	char* reformatConfirmation[] = {"Erase/ format SD",
							        "card to FATFS...",
									" Are you sure?  ",
									"Click < to abort"};

	char* ecbMenu[] = {"ECB action:     ",
			           "  Encrypt file  ",
					   "  Decrypt file  "};
	char* cbcMenu[] = {"ECB action:     ",
			           "  Encrypt file  ",
					   "  Decrypt file  "};

	char* fileMenu[] = {"Select file:    "};


	char* keyConfirmation[] = {"  Please enter  ",
							   "  AES key using ",
							   "  the switches  ",
							   "Click < to abort"};

	char* encryptConfirmation[] = {" File will now  ",
							       " be encrypted   ",
							       " Are you sure?  ",
							       "Click < to abort"};
	char* decryptConfirmation[] = {" File will now  ",
							       " be decrypted   ",
							       " Are you sure?  ",
							       "Click < to abort"};



	enum DPAD dpadClick;

/* START */
	print("##### Application Starts #####\n\n");
	/* Initialization */
	init_platform();
	/* D-pad buttons */
	if (XST_SUCCESS != XGpio_Initialize(&gpioBtn, XPAR_BTN_GPIO_AXI_DEVICE_ID)) {
		printf("UH OH: BTN5 GPIO initialization failed\r\n");
	}
	XGpio_SetDataDirection(&gpioBtn, 1, 1);

	while(1) {
		/* Start Screen */
		clear();
		print_message(" AES ENCRYPTION ", 1);
		print_message("  & DECRYPTION  ", 2);

		while(!startScreen) {
			dpadClick = XGpio_DiscreteRead(&gpioBtn, 1);
			if (dpadClick == CENTER ) {
				startScreen = true;
				usleep(DEBOUNCE_DELAY);
			}
		}
		startScreen = false;

main_menu:
		choice = selection_screen(mainMenu, sizeof(mainMenu)/4);
		switch (choice) {
			case 1: // Reformat
				confirmation = false;
				print_screen(reformatConfirmation);

				while(!confirmation) {
					dpadClick = XGpio_DiscreteRead(&gpioBtn, 1);
					if(dpadClick == CENTER) {
						confirmation = true;
						usleep(DEBOUNCE_DELAY);
					} else if(dpadClick == LEFT) {
						usleep(DEBOUNCE_DELAY);
						goto main_menu;
					}
				}
				/*
				 * REFORMAT CODE HERE
				 */
				goto main_menu;
			case 2: // Test BIN
				/*
				 * TEST.BIN CODE HERE
				 */
				choice = selection_screen(testBinMenu, sizeof(testBinMenu)/4);
				switch (choice) {
					case 1: // 16 bytes
						break;
					case 2: // larger size
						break;
					default:
						break;
				}
				goto main_menu;
			case 3: // ECB
				break;
			case 4: // CBC
				break;
			case 5: // Exit
				usleep(DEBOUNCE_DELAY);
				break;
			default:
				break;
		}
	}
	return 0;
}

void print_screen(char* menuLines[]) {
	refresh_oled(menuLines, 4, 0, false);
}

int selection_screen(char* menuLines[], int numOfLines) {
	int i = 1; // 1st line is menu title
	bool exitMenu = false;
	enum DPAD dpadClick;

	clear();
	refresh_oled(menuLines, numOfLines, i, true);

	while(!exitMenu)
	{
		dpadClick = XGpio_DiscreteRead(&gpioBtn, 1);
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
			default:
				break;
		}
	}

	return i;
}

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
