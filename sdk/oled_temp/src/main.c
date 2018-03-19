/*
 *  main.c
 *
 *  Created on: Mar 17, 2018
 *      Author: jrtsang
 */
#include "common.h"

///************************** Function Prototypes ******************************/
//void refresh_oled (char* printLines[], int numOfLines, int index, bool menu);
//void format_cursor(char inputLine[], char *outputLine);
//int selection_screen(char* printLines[], int numOfLines);
//void print_screen(char* menuLines[]);

int main(void){
	static XGpio gpioBtn;
	int choice = 0;

	/* ENSURE LENGTHS OF 16 PER LINE */
	char* welcomeConfirmation[] = {"                ",
								   " AES ENCRYPTION ",
								   "  & DECRYPTION  ",
								   "                "};
	char* mainMenu[] = {"Main Menu:      ",
						"  List files    ",
						"  ECB mode      ",
						"  CBC mode      ",
						"  Reformat      ",
						"  Test bin      ",
						"  Ethernet mode ",
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

	char* cbcMenu[] = {"CBC action:     ",
			           "  Encrypt file  ",
					   "  Decrypt file  "};

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

	static FATFS fatfs; // File system format

/* START */
	print("##### Application Starts #####\n\n");
	/* Initialization */
	init_platform();
	/* D-pad buttons */
	if (XST_SUCCESS != XGpio_Initialize(&gpioBtn, XPAR_BTN_GPIO_AXI_DEVICE_ID)) {
		printf("UH OH: BTN5 GPIO initialization failed\r\n");
	}
	XGpio_SetDataDirection(&gpioBtn, 1, 1);
	/* SD card */
	init_sd(&fatfs);
	char** fileList;
	char** fileListMenu;
	int numOfFiles;

welcome_screen:
	/* Start Screen */

	while(!confirmation_screen(&gpioBtn, welcomeConfirmation)) {
	}

	while(1) {
		choice = selection_screen(&gpioBtn, mainMenu, sizeof(mainMenu)/4);
		switch (choice) {
			case 1:
				fileList = list_all_files(&numOfFiles);
				fileListMenu = format_fileList(fileList, numOfFiles); // numOfFiles offset by 1
file_list:
				choice = selection_screen(&gpioBtn, fileListMenu, numOfFiles+1);
				// Hack to disable center button and only have back button exit
				if(choice > 0) {
					goto file_list;
				}
				free(fileList);
				free(fileListMenu);
				break;
			case 2: // ECB
				choice = selection_screen(&gpioBtn, ecbMenu, sizeof(ecbMenu)/4);
				switch (choice) {
					case 1: // Encrypt
						// Choice is the filename so do stuff
						fileList = list_all_files(&numOfFiles);
						fileListMenu = format_fileList(fileList, numOfFiles); // numOfFiles offset by 1
ecb_file_selection:
						choice = selection_screen(&gpioBtn, fileListMenu, numOfFiles+1);
						if (choice > 0) {
							if(!confirmation_screen(&gpioBtn, keyConfirmation)) {
								goto ecb_file_selection;
							}
							if(!confirmation_screen(&gpioBtn, encryptConfirmation)) {
								goto ecb_file_selection;
							}
						}
						free(fileList);
						free(fileListMenu);

						break;
					case 2: // Decrypt
						break;
					default:
						break;
				}
				break;
			case 3: // CBC
				break;
			case 4: // Reformat
				if(!confirmation_screen(&gpioBtn, reformatConfirmation)) {
					break;
				}
				// check if it failed, print on screen
				format_sd();
				break;
			case 5: // Test BIN
				choice = selection_screen(&gpioBtn, testBinMenu, sizeof(testBinMenu)/4);
				// check if it failed, print on screen
				create_test_bin(choice);
				break;
			case 6: // Ethernet
				break;
			case 7: // Exit
				usleep(DEBOUNCE_DELAY);
				goto welcome_screen;
			default:
				break;
		}
	}
	return 0;
}
