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
	bool startScreen = false;
	bool confirmation = false;

	/* ENSURE LENGTHS OF 16 PER LINE */
	char* welcomeConfirmation[] = {"                ",
								   " AES ENCRYPTION ",
								   "  & DECRYPTION  ",
								   "                "};
	char* mainMenu[] = {"Main Menu:      ",
						"  Reformat      ",
						"  Test bin      ",
						"  ECB mode      ",
						"  CBC mode      ",
						"  List files    ",
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

	char* fileMenu[] = {"Select file:    ",
						"  temp1         ",
						"  temp2         "};


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

welcome_screen:
	/* Start Screen */
	print_screen(welcomeConfirmation);

	while(!startScreen) {
		dpadClick = XGpio_DiscreteRead(&gpioBtn, 1);
		if (dpadClick == CENTER ) {
			startScreen = true;
			usleep(DEBOUNCE_DELAY);
		}
	}
	startScreen = false;

	char** fileList;
	char** fileListMenu;
	int numOfFiles;

	while(1) {
		choice = selection_screen(&gpioBtn, mainMenu, sizeof(mainMenu)/4);
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
						break;
					}
				}
				// check if it failed, print on screen
				format_sd();
			case 2: // Test BIN
				choice = selection_screen(&gpioBtn, testBinMenu, sizeof(testBinMenu)/4);
				// check if it failed, print on screen
				create_test_bin(choice);
				break;
			case 3: // ECB
				choice = selection_screen(&gpioBtn, ecbMenu, sizeof(ecbMenu)/4);
				switch (choice) {
					case 1: // Encrypt
						// Choice is the filename so do stuff
						choice = selection_screen(&gpioBtn, fileMenu, sizeof(fileMenu)/4);

						if(!confirmation_screen(&gpioBtn, keyConfirmation)) {
							break;
						}
						if(!confirmation_screen(&gpioBtn, encryptConfirmation)) {
							break;
						}
						break;
					case 2: // Decrypt
						break;
					default:
						break;
				}
				break;
			case 4: // CBC
				break;
			case 5:
				fileList = list_all_files(&numOfFiles);
				fileListMenu = format_fileList(fileList, numOfFiles); // numOfFiles offset by 1
				choice = selection_screen(&gpioBtn, fileListMenu, numOfFiles+1);
				free(fileList);
				free(fileListMenu);
				break;
			case 6: // Exit
				usleep(DEBOUNCE_DELAY);
				goto welcome_screen;
			default:
				break;
		}
	}
	return 0;
}
