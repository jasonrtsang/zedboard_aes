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
void refresh_oled (const char* menuLine, const char* printLines[], int numOfLines, int index);
void format_line(const char inputLine[], char *outputLine, bool cursor);
int selection_menu(const char* menuLine, const char* printLines[], int numOfLines);

int main(void){
	int i = 0, listLength = 0;
	bool nextMenu = false;
	bool confirmation = false;

	const char* mainMenu[LENGTH_MAINMENU] = {"Reformat", "Test bin", "ECB mode", "CBC mode", "Exit"};
	const char* title_mainMenu = "Main Menu:";

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

		while(!nextMenu) {
			dpadClick = XGpio_DiscreteRead(&gpioBtn, 1);
			if (dpadClick == CENTER ) {
				nextMenu = true;
				usleep(DEBOUNCE_DELAY);
			}
		}

main_menu:
		i = selection_menu(title_mainMenu, mainMenu, sizeof(mainMenu)/4);

		nextMenu = false;
		switch (i) {
			case 0: // Reformat
				confirmation = false;
				clear();
				print_message("Erase/ format SD", 0);
				print_message("card to FATFS...", 1);
				print_message(" Are you sure?  ", 2);
				print_message("Click < to abort", 3);

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
			case 1: // Test BIN
				confirmation = false;
				clear();
				const char* title_testBin = "Choose BIN size:";
				const char* testBin[2] = {"16 bytes", "64 bytes"};


				i = 0;
				nextMenu = false;
				listLength = 2;
				clear();
				refresh_oled(title_testBin, testBin, listLength, i);

				while(!nextMenu)
				{
					dpadClick = XGpio_DiscreteRead(&gpioBtn, 1);
					switch (dpadClick) {
						case CENTER:
							nextMenu = true;
							break;
						case DOWN:
							clear();
							if(i >= 0 && i < listLength-1) {
								i+=1;
							}
							refresh_oled(title_testBin, testBin, listLength, i);
							usleep(DEBOUNCE_DELAY);
							break;
						case UP:
							clear();
							if(i > 0 && i < listLength) {
								i-=1;
							}
							refresh_oled(title_testBin, testBin, listLength, i);
							usleep(DEBOUNCE_DELAY);
							break;
						case LEFT:
							usleep(DEBOUNCE_DELAY);
							goto main_menu;
						default:
							break;
					}
				}

				/*
				 * TEST.BIN CODE HERE
				 */
				nextMenu = false;
				switch (i) {
					case 0: // 16 bytes
						break;
					case 1: // larger size
						break;
					default:
						break;
				}
				goto main_menu;
			case 2: // ECB
				break;
			case 3: // CBC
				break;
			case 4: // Exit
				usleep(DEBOUNCE_DELAY);
				break;
			default:
				break;
		}






	}
	return (0);
}

int selection_menu(const char* menuLine, const char* menuLines[], int numOfLines) {
	int i = 0;
	bool nextMenu = false;
	enum DPAD dpadClick;

	clear();
	refresh_oled(menuLine, menuLines, numOfLines, i);

	while(!nextMenu)
	{
		dpadClick = XGpio_DiscreteRead(&gpioBtn, 1);
		switch (dpadClick) {
			case CENTER:
				nextMenu = true;
				break;
			case DOWN:
				clear();
				if(i >= 0 && i < numOfLines-1) {
					i+=1;
				}
				refresh_oled(menuLine, menuLines, numOfLines, i);
				usleep(DEBOUNCE_DELAY);
				break;
			case UP:
				clear();
				if(i > 0 && i < numOfLines) {
					i-=1;
				}
				refresh_oled(menuLine, menuLines, numOfLines, i);
				usleep(DEBOUNCE_DELAY);
				break;
			default:
				break;
		}
	}

	return i;
}


void refresh_oled (const char* menuLine, const char* printLines[], int numOfLines, int index) {

	char formattedLine[16];

	clear();

	print_message(menuLine, 0);

	if (index-1 >= 0 && index-1 < numOfLines) {
		format_line(printLines[index-1], formattedLine, false);
		print_message(formattedLine, 1);
	}
	if (index < numOfLines) {
		format_line(printLines[index], formattedLine, true);
		print_message(formattedLine, 2);
	}
	if (index+1 < numOfLines) {
		format_line(printLines[index+1], formattedLine, false);
		print_message(formattedLine, 3);
	}
}

void format_line(const char inputLine[], char *outputLine, bool cursor) {
   int i = 0, length = 0;

   if (cursor) {
	   outputLine[0] = '*';
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

