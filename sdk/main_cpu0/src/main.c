/*
 *  main.c

 *
 *  Created on: Mar 17, 2018
 *      Author: jrtsang
 */

#include "common.h"

/******************************* Definitions *********************************/
#define sev() __asm__("sev")

XAxiDma axiDma;
/*****************************************************************************/

/*****************************************************************************/
/**
*
* Initialize all drivers
*
* @param    None
*
* @return   None
*
* @note     None
*
**/
/*****************************************************************************/
void _main_initialization(void) {

	init_platform(); // UART, Caches
    oled_init(); // DPAD GPIO
    gic_init(); // GIC, BTN9 GPIO
    aes_init(); // DMA, Switches GPIO
    dma_init(&axiDma);
	sd_init(); // Mount SD

	// Inter-processor setup for CPU1
	COMM_VAL = 0;
	// Disable cache on OCM
	Xil_SetTlbAttributes(0xFFFF0000, 0x14de2); // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0
	Xil_Out32(0xFFFFFFF0, 0x00200000); // 0xFFFFFFF0 = CPU1STARTADDR
	dmb(); // Waits until write has finished
	sev(); // Send SEV to wake up CPU1
}

/*****************************************************************************/
/**
*
* Entrance to CPU0 Application
*
* @param    None
*
* @return   0 when exit
*
* @note     None
*
**/
/*****************************************************************************/
int main(void){
	/* ENSURE LENGTHS OF 16 PER LINE */
	char* welcomeConfirmation[] = {"                ",
								   " AES ENCRYPTION ",
								   "  & DECRYPTION  ",
								   "                "};
	char* mainMenu[] = {"Main Menu:      ",
						"  ECB mode      ",
						"  CBC mode      ",
						"  Ethernet mode ",
						"  Reformat SD   ",
						"  Quit          "};

	char* reformatConfirmation[] = {"Erase/ format SD",
							        "card to FATFS...",
									" Are you sure?  ",
									"Click < to abort"};

	char* ecbMenu[] = {"ECB action:     ",
			           "  Encrypt file  ",
					   "  Decrypt file  "};

	const uint8_t iv_key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	char* cbcMenu[] = {"CBC action:     ",
			           "  Encrypt file  ",
					   "  Decrypt file  "};

	char* doneConfirmation[] = {"                ",
							    "      Done!     ",
							    "                ",
							    "                "};

	char* cancelConfirmation[] = {"                ",
							      "   Cancelled!   ",
							      "                ",
							      "                "};

	char* failedConfirmation[] = {"    Uh oh...    ",
							      " something went ",
							      "    wrong...    ",
							      "                "};

	int choice;
	enum STATUS aesStatus;

#if UART_PRINT
	print("##### Application Starts #####\n\n");
#endif

    _main_initialization();

	// Start Screen
	while(!oled_confirmation_screen(welcomeConfirmation)) {
	}

	// Main Menu loop
	while(1) {
		// Reset flags
		COMM_VAL = 0;
		cancelFlag = false;
		// Main Menu selection
		choice = oled_selection_screen(mainMenu, sizeof(mainMenu)/4);
		switch (choice) {
			case 1: // ECB
ecb_menu:
				// ECB encryption or decryption
				choice = oled_selection_screen(ecbMenu, sizeof(ecbMenu)/4);
				switch (choice) {
					case 1: // Encrypt
						aesStatus = aes_sd_process_run(ENCRYPTION, &axiDma);
						break;
					case 2: // Decrypt
						aesStatus = aes_sd_process_run(DECRYPTION, &axiDma);
						break;
					default:
						aesStatus = EXIT;
						break;
				}
				switch(aesStatus) {
					case DONE:
						usleep(250000); // Prevent race case for OLED
						while(!oled_confirmation_screen(doneConfirmation));
						break;
					case FAILED:
						while(!oled_confirmation_screen(failedConfirmation));
						break;
					case BACK:
						goto ecb_menu;
						break;
					case CANCELLED:
						cancelFlag = false;
						usleep(250000); // Prevent race case for OLED
						while(!oled_confirmation_screen(cancelConfirmation));
						break;
					case EXIT:
					default:
						break;
				};
				break;
			case 2: // CBC
				break;
			case 3: // Ethernet
				ethernet_mode_run(&axiDma);
				break;
			case 4: // Reformat
				if(!oled_confirmation_screen(reformatConfirmation)) {
					break;
				}
				// check if it failed, print on screen
				sd_format();
				if(!oled_confirmation_screen(doneConfirmation)) {
					break;
				}
				break;
			case 5: // Exit
				oled_clear();
				return 0;
			default:
				break;
		}
	}
}
