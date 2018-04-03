/*
 *  main.c
 *
 *  Created on: Mar 17, 2018
 *      Author: jrtsang
 */
#include "common.h"



#include <stdio.h>
#include "xil_io.h"
#include "xil_mmu.h"
#include "xil_exception.h"
#include "xpseudo_asm.h"
#include "xscugic.h"



#define sev() __asm__("sev")
#define CPU1STARTADR 0xfffffff0
#define COMM_VAL  (*(volatile unsigned long *)(0xFFFF0000))







void getKeyValue(XGpio* gpioSwitches, uint8_t* switchKey) {
	/* Fixed keys and initialization vector (cbc) */
	const uint8_t keys[] = {
			0x72, 0x42, 0xf8, 0xeb, 0xe2, 0xca, 0x6c, 0x20, 0x6c, 0xd8, 0xdf, 0x1a, 0xcd, 0xe3, 0xfd, 0xe7,
			0x89, 0xb3, 0x6e, 0xae, 0x31, 0xa4, 0x73, 0x7a, 0xda, 0x5c, 0x4a, 0x41, 0x63, 0x33, 0x11, 0xbd,
			0xf3, 0x52, 0x2a, 0x20, 0x1a, 0xfd, 0x97, 0x3e, 0x13, 0x7a, 0xec, 0x26, 0x3f, 0x8d, 0xeb, 0xdf,
			0x18, 0x6e, 0xb8, 0x08, 0x46, 0x4a, 0xd0, 0x15, 0xbf, 0x81, 0xb7, 0xa2, 0x3b, 0x78, 0xc2, 0x44,
			0xdc, 0x4c, 0x81, 0x75, 0x22, 0x74, 0x07, 0xae, 0x75, 0x8c, 0x65, 0xd0, 0x89, 0x25, 0x72, 0x67,
			0xa3, 0x02, 0x2d, 0xdd, 0xdf, 0x45, 0x0d, 0x3c, 0xa3, 0x44, 0x49, 0x4a, 0xe4, 0x09, 0x15, 0x2c,
			0x36, 0xe4, 0xb2, 0x70, 0xcd, 0x70, 0xc2, 0x06, 0x8c, 0xf2, 0xf9, 0x95, 0x83, 0x0e, 0x92, 0xb3,
			0x4d, 0xe1, 0xab, 0x84, 0x86, 0x2e, 0x41, 0x1a, 0x31, 0x83, 0xf6, 0xbe, 0x6e, 0x5d, 0x25, 0xa6,
			0x4d, 0x43, 0xf6, 0x9b, 0x72, 0xe7, 0xb0, 0xeb, 0x7c, 0xd4, 0xbb, 0x5a, 0xa3, 0xd1, 0x3e, 0xac,
			0xa9, 0x30, 0x9b, 0x17, 0x1b, 0x53, 0xf6, 0x78, 0x41, 0xf8, 0xef, 0x79, 0x78, 0xe5, 0x5b, 0x0b,
			0x06, 0x79, 0x72, 0x9a, 0x0d, 0xa1, 0x45, 0xa1, 0xda, 0x79, 0x17, 0x33, 0x78, 0x33, 0xbf, 0x84,
			0xfe, 0xa8, 0x1a, 0x4b, 0x7c, 0xb0, 0x79, 0xa0, 0x64, 0x0f, 0xf3, 0x3d, 0xe6, 0x72, 0xf3, 0x1d,
			0x89, 0x30, 0xa1, 0x0e, 0x41, 0xb6, 0xd8, 0x6d, 0x95, 0xfd, 0xd7, 0x79, 0xf9, 0xfb, 0xc9, 0x39,
			0xd1, 0x9f, 0x1c, 0x99, 0x88, 0x3a, 0x1a, 0xcf, 0xc1, 0xbf, 0x93, 0x01, 0x26, 0xe2, 0x40, 0x17,
			0xc9, 0x10, 0x64, 0x5d, 0x1c, 0xc8, 0x39, 0xec, 0x9d, 0x9e, 0x1e, 0xff, 0x4a, 0xe1, 0xe8, 0xc7,
			0x16, 0xbe, 0xc2, 0xc3, 0x77, 0x15, 0x36, 0xc2, 0x8b, 0x29, 0xea, 0x73, 0x5b, 0x58, 0x25, 0x20,
			0xd7, 0xac, 0x69, 0x1d, 0xbb, 0x86, 0xb3, 0x47, 0xf8, 0xf1, 0x58, 0x12, 0xd1, 0x57, 0xec, 0x83,
			0xbc, 0x8a, 0x7e, 0xe9, 0x69, 0xf6, 0x2c, 0x86, 0x13, 0x0e, 0x13, 0x1c, 0xf4, 0x0d, 0xe6, 0x08,
			0xdd, 0xc9, 0xf7, 0xc0, 0x3c, 0xab, 0x0e, 0x11, 0x7c, 0x57, 0x8e, 0x36, 0x4c, 0x57, 0xbc, 0x1f,
			0x7c, 0xfe, 0x0a, 0x61, 0x57, 0x53, 0x83, 0x88, 0x7c, 0x70, 0x71, 0xe8, 0x5b, 0x5b, 0x0a, 0x67,
			0xfb, 0x47, 0xcf, 0x9a, 0x7d, 0x0e, 0x9c, 0x94, 0x52, 0x54, 0xaa, 0x3a, 0xf6, 0x82, 0xd2, 0xa8,
			0x44, 0xfa, 0x2d, 0x2d, 0x14, 0x56, 0x06, 0xcf, 0x21, 0x47, 0x75, 0xb2, 0x7c, 0x0a, 0xd7, 0x2c,
			0x96, 0xe4, 0x38, 0xec, 0xba, 0xcc, 0x69, 0x79, 0x80, 0xc3, 0x52, 0xab, 0x5c, 0x9e, 0x0f, 0x09,
			0xd2, 0x50, 0x4f, 0xe5, 0x86, 0xf1, 0xe9, 0xec, 0x39, 0x4b, 0x61, 0x93, 0xb6, 0x6e, 0x62, 0xc0,
			0x09, 0x49, 0x49, 0x0b, 0x53, 0xfb, 0x11, 0xb2, 0xe7, 0xf9, 0x0b, 0xe5, 0xff, 0xfb, 0x9c, 0x28,
			0x2f, 0xef, 0x79, 0x24, 0x38, 0x0f, 0xc7, 0x75, 0x92, 0x01, 0xf2, 0xa8, 0x14, 0xab, 0x4b, 0xd4,
			0xc2, 0x38, 0xed, 0xfa, 0x29, 0x27, 0xb9, 0xa5, 0x27, 0x3d, 0x14, 0x1a, 0xe5, 0xb7, 0x46, 0x84,
			0xf7, 0x39, 0x04, 0x23, 0x93, 0x19, 0x17, 0x47, 0x28, 0xea, 0x4c, 0x06, 0x35, 0xaf, 0x91, 0x0b,
			0xc7, 0x65, 0x89, 0xf1, 0xf0, 0x87, 0xc7, 0xfe, 0x71, 0x2f, 0x75, 0x99, 0xcc, 0x9f, 0x9b, 0x4a,
			0xd1, 0x2c, 0x1c, 0x5a, 0x80, 0x6c, 0x8c, 0x53, 0x84, 0xf2, 0x88, 0x99, 0x8b, 0xf5, 0x24, 0x8c,
			0xeb, 0xf3, 0x03, 0xe5, 0x61, 0x25, 0x34, 0xbd, 0xe3, 0x29, 0x27, 0x26, 0x9f, 0x41, 0x96, 0x74,
	};

	uint8_t dipValue;
	int i;

	dipValue = XGpio_DiscreteRead(gpioSwitches, 1);
	for (i = 0; i < 16; i++) {
		switchKey[i] = keys[dipValue+i*16];
    }
}


int main(void){
	static XGpio gpioBtn;
	int choice = 0;

	const uint8_t iv_key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

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

	char* doneConfirmation[] = {"                ",
							    "      Done!     ",
							    "                ",
							    "                "};

	char* cancelConfirmation[] = {"                ",
							      "   Cancelled!   ",
							      "                ",
							      "                "};

	char* processingScreen[] = {"                ",
							    "   Processing   ",
							    "      ....      ",
							    "                "};


	static FATFS fatfs; // File system format



//#define CPU1ADDR 0x20000000
//#define CPU1ADDR_START 0XFFFFFFF0
//#define sev() __asm__("sev")
//
//Xil_SetTlbAttributes(0xFFFF0000, 0X14DE2);



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
	char** fileList;
	char** fileListMenu;
	int numOfFiles;
	/* Interrupt cancel button */
	static XScuGic Intc; /* The Instance of the Interrupt Controller Driver */
	static XGpioPs Gpio; /* The driver instance for GPIO Device. */

	/* Encryption states */
#if 0
    static uint8_t inputBuf[10*1024*1024] __attribute__ ((aligned(32))); // 10mb buffers [1024*1024 == 1mb, 1024 == 1kb]
    struct AES_ctx ctx; // Context
#endif
    uint32_t fileSizeRead;


	/* Switches and LEDs Setup*/
	XGpio gpioSwitches;
//	XGpio gpioLeds;
	uint8_t switchKey[16];

	if(XGpio_Initialize(&gpioSwitches, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) {
		printf("UH OH: GPIO SWS initialization failed\r\n");
	};
//	if(XGpio_Initialize(&gpioLeds, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) {
//		printf("UH OH: GPIO2 LEDS initialization failed\r\n");
//	};

	// Set the direction of the bits in the GPIO.
	// The lower (LSB) 8 bits of the GPIO are for the DIP Switches (inputs).
	// The upper (MSB) 8 bits of the GPIO are for the LEDs (outputs).
	XGpio_SetDataDirection(&gpioSwitches, 1, 0x00FF);
//	XGpio_SetDataDirection(&gpioLeds, 2, 0x0000);

	/* MIO51 BTN9 Setup*/
	XGpioPs_Config *GPIOConfigPtr;
	GPIOConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	if (XGpioPs_CfgInitialize(&Gpio, GPIOConfigPtr,GPIOConfigPtr->BaseAddr) != XST_SUCCESS) {
		 print("GPIO INIT FAILED\n\r");
		return XST_FAILURE;
	}

	// Set direction input pin
	XGpioPs_SetDirectionPin(&Gpio, pbsw, 0x0);
	SetupInterruptSystem(&Intc, &Gpio, GPIO_INTERRUPT_ID);

	/* DMA */
	static XAxiDma AxiDma;
	XAxiDma_Init(&AxiDma, DMA_DEV_ID);


//	Xil_Out32(CPU1ADDR_START, CPU1ADDR);
//	dmb();
//	sev();

welcome_screen:
	/* Start Screen */

	while(!confirmation_screen(&gpioBtn, welcomeConfirmation)) {
	}











	//Disable cache on OCM
	Xil_SetTlbAttributes(0xFFFF0000,0x14de2);           // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0


	COMM_VAL = 0;



	print("CPU0: writing startaddress for cpu1\n\r");
		Xil_Out32(CPU1STARTADR, 0x00200000);
		dmb(); //waits until write has finished

		print("CPU0: sending the SEV to wake up CPU1\n\r");
		sev();

	while(1){

		print("CPU0: Hello World CPU 0\n\r");
		COMM_VAL = 1;
		while(COMM_VAL == 1);

	}

































	while(1) {
		init_sd(NULL);
		choice = selection_screen(&gpioBtn, mainMenu, sizeof(mainMenu)/4);
		init_sd(&fatfs);
		cancelFlag = false;
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
ecb_file_encrypt:
						choice = selection_screen(&gpioBtn, fileListMenu, numOfFiles+1);
						if (choice > 0) {
							if(!confirmation_screen(&gpioBtn, keyConfirmation)) {
								goto ecb_file_encrypt;
							} else {
								// Get key value
								getKeyValue(&gpioSwitches, switchKey);
							}
							if(!confirmation_screen(&gpioBtn, encryptConfirmation)) {
								goto ecb_file_encrypt;
							} else {
								print_screen(processingScreen);
		                        /* Read the current specified file */
		                        fileSizeRead = 0;
		                        if(!read_from_file(fileList[choice-1], (u32*)TX_BUFFER_BASE, &fileSizeRead)) {
		                            break;
		                        }

		                        // Padding need to fix...
		                        int length = 16 * ((fileSizeRead + 15) / 16);

								u32 *outputBuf_ptr = (u32*)RX_BUFFER_BASE;
								u32 *inputBuf_ptr = (u32*)TX_BUFFER_BASE;
								for (int i = 0; i < length; i += AES_BLOCKLEN)
								{
									AES_Process(&AxiDma, switchKey, inputBuf_ptr, outputBuf_ptr, ENCRYPTION);
									inputBuf_ptr += AES_BLOCKLEN/4;
									outputBuf_ptr += AES_BLOCKLEN/4;
								    if (cancelFlag) {
										if(confirmation_screen(&gpioBtn, cancelConfirmation)) {
											cancelFlag = false;
											goto ecb_file_encrypt_end;
										}
								    }
								}
								/* Create output file */
								write_to_file(fileList[choice-1], (u32*)RX_BUFFER_BASE, fileSizeRead);
								if(!confirmation_screen(&gpioBtn, doneConfirmation)) {
									break;
								}
							}
						}
ecb_file_encrypt_end:
						free(fileList);
						free(fileListMenu);
						break;
					case 2: // Decrypt
						// Choice is the filename so do stuff
						fileList = list_all_files(&numOfFiles);
						fileListMenu = format_fileList(fileList, numOfFiles); // numOfFiles offset by 1
ecb_file_decrypt:
						choice = selection_screen(&gpioBtn, fileListMenu, numOfFiles+1);
						if (choice > 0) {
							if(!confirmation_screen(&gpioBtn, keyConfirmation)) {
								goto ecb_file_decrypt;
							} else {
								// Get key value
								getKeyValue(&gpioSwitches, switchKey);
							}
							if(!confirmation_screen(&gpioBtn, encryptConfirmation)) {
								goto ecb_file_decrypt;
							} else {
								print_screen(processingScreen);
#if 0
		                        /* Read the current specified file */
		                        fileSizeRead = 0;
		                        if(!read_from_file(fileList[choice-1], inputBuf, &fileSizeRead)) {
		                            break;
		                        }
		                        /* Init roundkeys and process */
								AES_init_ctx(&ctx, switchKey);
								if (!AES_ECB_decrypt_buffer(&ctx, inputBuf, fileSizeRead)) {
									printf("ECB decryption CANCELED\r\n");
									break;
								}
								/* Create output file */
								write_to_file(fileList[choice-1], inputBuf, fileSizeRead);
								if(!confirmation_screen(&gpioBtn, doneConfirmation)) {
									break;
								}
#endif
		                        /* Read the current specified file */
		                        fileSizeRead = 0;
		                        if(!read_from_file(fileList[choice-1], (u32*)TX_BUFFER_BASE, &fileSizeRead)) {
		                            break;
		                        }

		                        // Padding need to fix...
		                        int length = 16 * ((fileSizeRead + 15) / 16);

								u32 *outputBuf_ptr = (u32*)RX_BUFFER_BASE;
								u32 *inputBuf_ptr = (u32*)TX_BUFFER_BASE;
								for (int i = 0; i < length; i += AES_BLOCKLEN)
								{
									AES_Process(&AxiDma, switchKey, inputBuf_ptr, outputBuf_ptr, DECRYPTION);
									inputBuf_ptr += AES_BLOCKLEN/4;
									outputBuf_ptr += AES_BLOCKLEN/4;
								    if (cancelFlag) {
										if(confirmation_screen(&gpioBtn, cancelConfirmation)) {
											cancelFlag = false;
											goto ecb_file_encrypt_end;
										}
								    }
								}

								write_to_file(fileList[choice-1], (u32*)RX_BUFFER_BASE, fileSizeRead);
								if(!confirmation_screen(&gpioBtn, doneConfirmation)) {
									break;
								}
							}
						}
						free(fileList);
						free(fileListMenu);
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
				if(!confirmation_screen(&gpioBtn, doneConfirmation)) {
					break;
				}
				break;
			case 5: // Test BIN
				choice = selection_screen(&gpioBtn, testBinMenu, sizeof(testBinMenu)/4);
				// check if it failed, print on screen
				if (create_test_bin(choice)) {
					if(!confirmation_screen(&gpioBtn, doneConfirmation)) {
						break;
					}
				}
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
