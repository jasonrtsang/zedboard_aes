/*****************************************************************************
*
* main.c: AES - ECB and CBC Encryption and Decryption on ZedBoard bare-metal
*
* @auth   Jason Tsang
*
* @date   2018/02/08
*
******************************************************************************/

#include "aes.h"

/* Switch Headers */
#include "platform.h"
#include "xgpio.h"
#include "xil_exception.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xgpiops.h"

/* SD Headers */
#include "ff.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xplatform_info.h"
#include "xsdps.h"

/* DMA Headers */
#include "xaxidma.h"

#include "xbasic_types.h"

/************************** Defines Prototypes *******************************/
/*
* Device hardware build related constants.
 */

#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#define MEM_BASE_ADDR		(XPAR_PS7_DDR_0_S_AXI_BASEADDR + 0x10000000)

#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#define MAX_PKT_LEN_WORDS_SEND	9
#define MAX_PKT_LEN_SEND			MAX_PKT_LEN_WORDS_SEND*4

#define MAX_PKT_LEN_WORDS_RCV	4
#define MAX_PKT_LEN_RCV			MAX_PKT_LEN_WORDS_RCV*4



#define NUMBER_OF_TRANSFERS	1

/************************** Function Prototypes ******************************/
void prompt_file_input(char *fileName);
void byte_comparison(void);
void create_test_bin(void);
bool write_to_file(const char *sdFile, const uint8_t *writeBuf, const uint32_t writeSize);
bool read_from_file(const char *sdFile, uint8_t *readBuf, uint32_t *readSize);

void My_XGpioPs_IntrHandler(XGpioPs *InstancePtr);
static void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio, u16 GpioIntrId);
static void IntrHandler(void *CallBackRef, int Bank, u32 Status);

int XAxiDma_Init(u16 DeviceId);
int XAxiDma_SimplePollExample(u16 DeviceId);
int XAxiDma_send_aes_state(u32 *inputBuf_ptr, u32 *outputBuf_ptr, u8 *key, bool mode);
static int CheckData(u8 *inBuf, u8 *outBuf, int size_of_data_bytes);

/************************** Variable Definitions *****************************/
#define FILENAME_LIMIT 32
#define USERINPUT_LIMIT 29
#define TESTBIN_SIZE_16 16
#define TESTBIN_SIZE_64 64
static FIL fil; // Specified file input

#define DECRYPTION 0
#define ENCRYPTION 1

/* MIO51 BTN9 Setup*/
#define XPAR_AXI_XADC_0_DEVICE_ID 0

#define GPIO_DEVICE_ID      XPAR_XGPIOPS_0_DEVICE_ID
#define INTC_DEVICE_ID      XPAR_SCUGIC_SINGLE_DEVICE_ID
#define GPIO_INTERRUPT_ID   XPS_GPIO_INT_ID

#define pbsw 51

static XScuGic Intc; /* The Instance of the Interrupt Controller Driver */
static XGpioPs Gpio; /* The driver instance for GPIO Device. */
int toggle;//used to toggle the LED
static void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio, u16 GpioIntrId);
static void IntrHandler(void *CallBackRef, int Bank, u32 Status);

XAxiDma AxiDma;

/*****************************************************************************
*
* Main function
*
* @param    None
*
* @return   0 (sucessful) always
*
* @note     None
*
******************************************************************************/
int main(void)
{
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
	const uint8_t iv_key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    /* Universally used variables */
    int i, menuChoice, modeChoice;
    uint32_t fileSizeRead;
    int Index = 0;

    const TCHAR *Path = "0:/"; // Base directory of SD
    static FATFS fatfs; // File system format

    char fileNameIn[FILENAME_LIMIT] = ""; // Specified file input name
    char fileNameOut[FILENAME_LIMIT]; // Output file name

    static uint8_t inputBuf[10*1024*1024] __attribute__ ((aligned(32))); // 10mb buffers [1024*1024 == 1mb, 1024 == 1kb]
	static uint8_t outputBuf[10*1024*1024] __attribute__ ((aligned(32))); // 10mb buffers [1024*1024 == 1mb, 1024 == 1kb]

    struct AES_ctx ctx; // Context

    bool exitFlag = false; // Application closure

    /* Initialize platform */
    init_platform();

    /* Switches and LEDs Setup*/
    XGpio gpioSwitches;
    XGpio gpioLeds;
    uint8_t switchKey[16];
	uint8_t dipValue;

	if(XGpio_Initialize(&gpioSwitches, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) {
		printf("UH OH: GPIO SWS initialization failed\r\n");
	};
	if(XGpio_Initialize(&gpioLeds, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) {
		printf("UH OH: GPIO2 LEDS initialization failed\r\n");
	};

	// Set the direction of the bits in the GPIO.
	// The lower (LSB) 8 bits of the GPIO are for the DIP Switches (inputs).
	// The upper (MSB) 8 bits of the GPIO are for the LEDs (outputs).
	XGpio_SetDataDirection(&gpioSwitches, 1, 0x00FF);
	XGpio_SetDataDirection(&gpioLeds, 2, 0x0000);

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

    // Initialize the DMA
    XAxiDma_Init(DMA_DEV_ID);

    /* Main application */
    while(!exitFlag) {
    	fflush(stdin);
    	cancelFlag = false;
        printf("\r\n\r\n");
        printf("~~~~~~~~~~~~~~~~ AES Encryption/ Decryption ~~~~~~~~~~~~~~~\r\n");
        printf(" SELECT the operation from the below menu \r\n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~ Menu Starts ~~~~~~~~~~~~~~~~~~~~~~~\r\n");
        printf(" Press '1' to Format SD card\r\n");
        printf(" Press '2' to Create TEST.BIN file\r\n");
        printf(" Press '3' to Enter CBC mode submenu\r\n");
        printf(" Press '4' to Enter ECB mode submenu\r\n");
        printf(" Press any other key to exit\r\n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~ Menu Ends ~~~~~~~~~~~~~~~~~~~~~~~~\r\n");

        /* Poll on menu selection */
        menuChoice = inbyte();
        fflush(stdin);
        printf(" Selection : %c\r\n\r\n", menuChoice);

        /* Register SD volume, initialize device */
        if(f_mount(&fatfs, Path, 0) != FR_OK) {
            printf("UH OH: Something went wrong with mounting the SD card...\r\n");
            return XST_FAILURE;
        }

        /* Menu options */
        switch(menuChoice) {
            /* Format SD Card */
            case '1':
                printf("Formatting SD Card to FATFS..\r\n");
                if(f_mkfs(Path, 0, 0) != FR_OK) {
                    printf("UH OH: Something went wrong with formatting the SD card...\r\n");
                    printf("Is the SD card plugged in and not locked?\r\n");
                } else {
                    printf("Done!\r\n");
                }
                break;

            /* Create TEST.BIN */
            case '2':
            	create_test_bin();
            	break;

            /* CBC */
            case '3':
                printf("\r\n\r\n");
                printf("~~~~~~~~~~~~~~~~~~~~~~~ CBC Mode ~~~~~~~~~~~~~~~~~~~~~~~\r\n");
                printf(" Press '1' to Run encryption\r\n");
                printf(" Press '2' to Run decryption\r\n");
                printf(" Press any other key to return to main menu\r\n");
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
                /* Poll on mode selection */
                modeChoice = inbyte();
                printf(" Selection : %c\r\n\r\n", modeChoice);

                switch(modeChoice) {
                    case '1': /* Encrypt */
                        printf("Starting CBC encrypt...\r\n");
						printf("> Input file to encrypt\r\n");
                        prompt_file_input(fileNameIn);
                        /* Read the current specified file */
                        fileSizeRead = 0;
                        if(!read_from_file(fileNameIn, inputBuf, &fileSizeRead)) {
                            break;
                        }
                        printf("> Name encrypted file output\r\n");
                        prompt_file_input(fileNameOut);
                        AES_init_ctx_iv(&ctx, switchKey, iv_key);
                        AES_CBC_encrypt_buffer(&ctx, inputBuf, fileSizeRead);
                        printf("Writing encrypted file to SD card...\r\n");
                        /* Create output file */
                        write_to_file(fileNameOut, inputBuf, fileSizeRead);
                        printf("Done!\r\n");
                        break;
                    case '2': /* Decrypt */
                        printf("Starting CBC decrypt...\r\n");
						printf("> Input file to decrypt\r\n");
                        prompt_file_input(fileNameIn);
                        /* Read the current specified file */
                        fileSizeRead = 0;
                        if(!read_from_file(fileNameIn, inputBuf, &fileSizeRead)) {
                            break;
                        }
                        printf("> Name decrypted file output\r\n");
                        prompt_file_input(fileNameOut);
                        AES_init_ctx_iv(&ctx, switchKey, iv_key);
                        AES_CBC_decrypt_buffer(&ctx, inputBuf, fileSizeRead);
                        printf("Writing decrypted file to SD card...\r\n");
                        /* Create output file */
                        write_to_file(fileNameOut, inputBuf, fileSizeRead);
                        printf("Done!\r\n");
                        break;
                    default:
                        break;
                }
                break;
            /* ECB */
            case '4':
                printf("\r\n\r\n");
                printf("~~~~~~~~~~~~~~~~~~~~~~~ ECB Mode ~~~~~~~~~~~~~~~~~~~~~~~\r\n");
                printf(" Press '1' to Run encryption\r\n");
                printf(" Press '2' to Run decryption\r\n");
                printf(" Press any other key to return to main menu\r\n");
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
                /* Poll on mode selection */
                modeChoice = inbyte();
                fflush(stdin);
                printf(" Selection : %c\r\n\r\n", modeChoice);

                switch(modeChoice) {
                    case '1': /* Encrypt */
                        printf("Starting ECB encrypt...\r\n");
						printf("> Input file to encrypt\r\n");
                        prompt_file_input(fileNameIn);
                        /* Read the current specified file */
                        fileSizeRead = 0;
                        if(!read_from_file(fileNameIn, inputBuf, &fileSizeRead)) {
                            break;
                        }
                        printf("> Name encrypted file output\r\n");
                        prompt_file_input(fileNameOut);

                        printf("Enter encryption key via switches, press enter to continue...\r\n");
						inbyte();
						fflush(stdin);
						// Read from the GPIO to determine the position of the DIP switches
						dipValue = XGpio_DiscreteRead(&gpioSwitches, 1);
						// Write the value back to the LEDS
//						XGpio_DiscreteWrite(&gpioLeds, 2, dipValue);
						for (i = 0; i < 16; i++) {
							switchKey[i] = keys[dipValue+i*16];
                        }
						printf("Running ECB encryption...\r\n");
						


						// Let's print out the input data first
						xil_printf("Data to encrypt: \r\n");
						for(Index = 0; Index < fileSizeRead; Index++) {
							xil_printf("0x%X ", inputBuf[Index]);
						}
						xil_printf("\r\n");




						// Temp pointers that the for loop can move around as it wants
						u32 *outputBuf_ptr = (u32*)outputBuf;
						u32 *inputBuf_ptr = (u32*)inputBuf;
						
						for (i = 0; i < fileSizeRead; i += AES_BLOCKLEN)
						{
							XAxiDma_send_aes_state(inputBuf_ptr, outputBuf_ptr, &switchKey, false);
							inputBuf_ptr += AES_BLOCKLEN;
							outputBuf_ptr += AES_BLOCKLEN;
						}
						
						CheckData((u8*)inputBuf, (u8*)outputBuf, fileSizeRead);
						
						AES_init_ctx(&ctx, switchKey);
                        if (!AES_ECB_encrypt_buffer(&ctx, inputBuf, fileSizeRead)) {
                        	printf("ECB encryption CANCELED\r\n");
                        	break;
                        }

                        // Now let's see what the data looks like after we're done
                        // Let's print out the input data first
						xil_printf("Encrypted Data (SW): \r\n");
						for(Index = 0; Index < fileSizeRead; Index++) {
							xil_printf("0x%X ", inputBuf[Index]);
						}
						xil_printf("\r\n");
						

                        printf("Writing encrypted file to SD card...\r\n");
                        /* Create output file */
                        write_to_file(fileNameOut, inputBuf, fileSizeRead);
                        printf("Done!\r\n");
                        break;
                    case '2': /* Decrypt */
                        printf("Starting ECB decrypt...\r\n");
						printf("> Input file to decrypt\r\n");
                        prompt_file_input(fileNameIn);
                        /* Read the current specified file */
                        fileSizeRead = 0;
                        if(!read_from_file(fileNameIn, inputBuf, &fileSizeRead)) {
                            break;
                        }
                        printf("> Name decrypted file output\r\n");
                        prompt_file_input(fileNameOut);

                        printf("Enter encryption key via switches, press enter to continue...\r\n");
						inbyte();
						fflush(stdin);
						// Read from the GPIO to determine the position of the DIP switches
						dipValue = XGpio_DiscreteRead(&gpioSwitches, 1);
						// Write the value back to the LEDS
//						XGpio_DiscreteWrite(&gpioLeds, 2, dipValue);
						for (i = 0; i < 16; i++) {
							switchKey[i] = keys[dipValue+i*16];
                        }
						printf("Running ECB decryption...\r\n");
						AES_init_ctx(&ctx, switchKey);
                        if (!AES_ECB_decrypt_buffer(&ctx, inputBuf, fileSizeRead)) {
							printf("ECB decryption CANCELED\r\n");
							break;
						}
                        printf("Writing decrypted file to SD card...\r\n");
                        /* Create output file */
                        write_to_file(fileNameOut, inputBuf, fileSizeRead);
                        printf("Done!\r\n");
                        break;
                    default:
                        break;
                }
                break;

            /* Any other key */
            default:
                exitFlag = true;
                break;
        }
    }

    printf("\r\n#####\r\nBYE!\r\n#####\r\n");
    cleanup_platform();
    return 0;
}

/*****************************************************************************/
/**
*
* Create 16 or 64 bytes test binary
*
* @param    None
*
* @return   true if successful, false otherwise
*
* @note     None
*
******************************************************************************/
void create_test_bin(void)
{
	int i, testBinChoice;
	uint8_t outputBuf[64] __attribute__ ((aligned(32)));

    printf("\r\n\r\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~ TEST.BIN ~~~~~~~~~~~~~~~~~~~~~~~\r\n");
    printf(" Press '1' to Create 16 byte size\r\n");
    printf(" Press '2' to Create 64 byte size\r\n");
    printf(" Press any other key to return to main menu\r\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
    /* Poll on mode selection */
    testBinChoice = inbyte();
    fflush(stdin);
    printf(" Selection : %c\r\n\r\n", testBinChoice);

    const char SD_TestBin[] = "TEST.BIN";
    uint32_t fileSize; // in bytes
    printf("Creating TEST.BIN...\r\n");
    switch(testBinChoice) {
        case '1':
            fileSize = TESTBIN_SIZE_16;
            break;
        case '2':
            fileSize = TESTBIN_SIZE_64;
            break;
        default:
            break;
    }
    /* Populate TEST.BIN with incrementing index */
    for(i = 0; i < fileSize; i++) {
        outputBuf[i] = i;
    }
    if(!write_to_file(SD_TestBin, outputBuf, fileSize)) {
    	printf("Is the SD card plugged in and not locked?\r\n");
    } else {
    	printf("Done!\r\n");
    }
}

/*****************************************************************************/
/**
*
* Write a file to the SD card
*
* @param    const char *sdFile        : Filename
*           const uint8_t *writeBuf   : Content to be written
*           const uint32_t writeSize  : Size of content to be written
*
* @return   true if successful, false otherwise
*
* @note     None
*
******************************************************************************/
bool write_to_file(const char *sdFile, const uint8_t *writeBuf, const uint32_t writeSize)
{
    UINT numOfBytesWritten; // Not used
    /* Create file */
    if(f_open(&fil, sdFile, FA_CREATE_ALWAYS | FA_WRITE | FA_READ)) {
        printf("UH OH: Something went wrong with creating the file...\r\n");
        return false;
    }
    /* Pointer to beginning of file */
    if(f_lseek(&fil, 0)) {
        printf("UH OH: Something went wrong with getting start of file...\r\n");
        return false;
    }
    /* Write data to file */
    if(f_write(&fil, (const void*)writeBuf, writeSize, &numOfBytesWritten)) {
        printf("UH OH: Something went wrong with writing content of file...\r\n");
        return false;
    }
    /* Close file */
    if(f_close(&fil)) {
        return false;
    }
    return true;
}

/*****************************************************************************/
/**
*
* Read a file from the SD card
*
* @param    const char *sdFile  : Filename
*           uint8_t *readBuf    : Buffer to be read to
*           uint32_t readSize   : Size of content read
*
* @return   true if successful, false otherwise
*
* @note     None
*
******************************************************************************/
bool read_from_file(const char *sdFile, uint8_t *readBuf, uint32_t *readSize)
{
    UINT numOfBytesRead; // Not used
    /* Open the file */
    if(f_open(&fil, sdFile, FA_OPEN_EXISTING | FA_READ) != FR_OK) {
        printf("UH OH: Could not find file, please specify one\r\n");
        return false;
    }
    /* Set pointer to beginning of file */
    if(f_lseek(&fil, 0)) {
        printf("UH OH: Something went wrong with getting start of file...\r\n");
        return false;
    }
    /* Read data from file */
    if(f_read(&fil, (void*)readBuf, fil.fsize, &numOfBytesRead)) {
        printf("UH OH: Something went wrong with reading content of file...\r\n");
        return false;
    }
    /* Pass file size */
    *readSize = fil.fsize;
    /* Close file */
    if(f_close(&fil)) {
        return false;
    }
    return true;
}

/*****************************************************************************/
/**
*
* Read a file from the SD card
*
* @param    char *fileName. : File name output from prompt
*
* @return   None
*
* @note     Appends 0:/ to any user input
*
******************************************************************************/
void prompt_file_input(char *fileName)
{
    int i;
    char userInput[USERINPUT_LIMIT];

    printf("> Enter filename (must not exceed 29 characters) ...\n\r");
    /* Clear input array */
    for(i = 0; i < USERINPUT_LIMIT; i++) {
        userInput[i] = '\0';
    }
    /* Clear filename buffer */
    for(i = 0; i < FILENAME_LIMIT; i++) {
        fileName[i] = '\0';
    }
    /* Loop until return hit */
    i = 0;
    do {
        userInput[i] = inbyte();
        putchar(userInput[i]);
        fflush(stdout);
        /* Backspace */
        if(userInput[i] == '\b') {
            userInput[i] = '\0';
            userInput[i-1] = '\0';
            i = (i-2);
            putchar(' ');
            putchar('\b');
            fflush(stdout);
            if(i <- 1) {
                i =- 1;
            }
        }
        i++;
    } while(userInput[i-1] != '\r');
    /* Remove last \r char from userInput */
    userInput[i-1] = '\0';
    printf("\r\n");
    /* Append drive directory to start */
    fileName[0] = '0';
    fileName[1] = ':';
    fileName[2] = '/';
    for(i = 0; i < USERINPUT_LIMIT; i++) {
        if(userInput[i] != '\0') {
            fileName[FILENAME_LIMIT-USERINPUT_LIMIT+i] = userInput[i];
        } else {
            break;
        }
    }
}

/*****************************************************************************/
/**
*
* ORIGINALLY FROM XGpioPs_IntrHandler (xgpios_intr.c), EDITTED TO REDUCE BOUNCE
*
* This function is the interrupt handler for GPIO interrupts.It checks the
* interrupt status registers of all the banks to determine the actual bank in
* which an interrupt has been triggered. It then calls the upper layer callback
* handler set by the function XGpioPs_SetBankHandler(). The callback is called
* when an interrupt
*
* @param	InstancePtr is a pointer to the XGpioPs instance.
*
* @return	None.
*
* @note		This function does not save and restore the processor context
*		such that the user must provide this processing.
*
******************************************************************************/
void My_XGpioPs_IntrHandler(XGpioPs *InstancePtr)
{
    u8 Bank;
    u32 IntrStatus;
    u32 IntrEnabled;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    for (Bank = 0U; Bank < XGPIOPS_MAX_BANKS; Bank++) {
        IntrStatus = XGpioPs_IntrGetStatus(InstancePtr, Bank);
        IntrEnabled = XGpioPs_IntrGetEnabled(InstancePtr, Bank);

        if ((IntrStatus != (u32)0) && (IntrEnabled != (u32)0)) {

            XGpioPs_IntrClear((XGpioPs *)InstancePtr, Bank,
                            (IntrStatus & IntrEnabled));
            InstancePtr->Handler(InstancePtr->
                         CallBackRef, Bank,
                         (IntrStatus & IntrEnabled));
        }
    }
}

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio, u16 GpioIntrId)
{
        XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */
        Xil_ExceptionInit();

        /*
         * Initialize the interrupt controller driver so that it is ready to
         * use.
         */
        IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

        XScuGic_CfgInitialize(GicInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);

        /*
         * Connect the interrupt controller interrupt handler to the hardware
         * interrupt handling logic in the processor.
         */
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, GicInstancePtr);

        /*
         * Connect the device driver handler that will be called when an
         * interrupt for the device occurs, the handler defined above performs
         * the specific interrupt processing for the device.
         */
        XScuGic_Connect(GicInstancePtr, GpioIntrId, (Xil_ExceptionHandler)My_XGpioPs_IntrHandler, (void *)Gpio);

        //Enable  interrupts for all the pins in bank 0.
        XGpioPs_SetIntrTypePin(Gpio, pbsw, XGPIOPS_IRQ_TYPE_EDGE_RISING);

        //Set the handler for gpio interrupts.
        XGpioPs_SetCallbackHandler(Gpio, (void *)Gpio, IntrHandler);

        //Enable the GPIO interrupts of Bank 0.
        XGpioPs_IntrEnablePin(Gpio, pbsw);

        //Enable the interrupt for the GPIO device.
        XScuGic_Enable(GicInstancePtr, GpioIntrId);

        // Enable interrupts in the Processor.
        Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
    }

static void IntrHandler(void *CallBackRef, int Bank, u32 Status)
{
    XGpioPs *Gpioint = (XGpioPs *)CallBackRef;
    XGpioPs_IntrClearPin(Gpioint, pbsw);
    cancelFlag = true;
}

// Initialize DMA
int XAxiDma_Init(u16 DeviceId)
{
	XAxiDma_Config *CfgPtr;
	int Status;
//	int Tries = NUMBER_OF_TRANSFERS;
//	int Index;
//	u32 *TxBufferPtr;
//	u32 *RxBufferPtr;
//	u32 Value;

//	TxBufferPtr = (u32 *)TX_BUFFER_BASE;
//	RxBufferPtr = (u32 *)RX_BUFFER_BASE;

	/* Initialize the XAxiDma device.
	 */
	CfgPtr = XAxiDma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if(XAxiDma_HasSg(&AxiDma)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);
						
	return XST_SUCCESS;
}

int XAxiDma_send_aes_state(u32 *inputBuf_ptr, u32 *outputBuf_ptr, u8 *key, bool mode)
{
	int Status = 0;
	int Index = 0;
	
	// Need to create the 9x32 bit output to send
	u32 *combinedInput = malloc(sizeof(u32) * MAX_PKT_LEN_WORDS_SEND);
	if (mode)
	{
		// Set to decrypt
		memset(&(combinedInput[0]), 0xFF, sizeof(u32));
	}
	else
	{
		// Set to encrypt
		memset(&(combinedInput[0]), 0x00, sizeof(u32));
	}
	memcpy(&(combinedInput[1]), inputBuf_ptr, (sizeof(u32) * 4));
	memcpy(&(combinedInput[5]), key, (16));
	
	// We are going to see if the things I'm sending are correct
	xil_printf("Output buffer BEFORE: \r\n");
	for(Index = 0; Index < 4; Index++) {
		xil_printf("0x%X ", outputBuf_ptr[Index]);
	}
	xil_printf("\r\n");

	Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) outputBuf_ptr,
					MAX_PKT_LEN_RCV, XAXIDMA_DEVICE_TO_DMA);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) combinedInput,
				MAX_PKT_LEN_SEND, XAXIDMA_DMA_TO_DEVICE);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (XAxiDma_Busy(&AxiDma,XAXIDMA_DMA_TO_DEVICE)) {
			/* Wait */
	}
	while (XAxiDma_Busy(&AxiDma,XAXIDMA_DEVICE_TO_DMA)) {
			/* Wait */
	}

	// We are going to see if the things I'm sending are correct
	xil_printf("Output buffer AFTER: \r\n");
	for(Index = 0; Index < 4; Index++) {
		xil_printf("0x%X ", outputBuf_ptr[Index]);
	}
	xil_printf("\r\n");

//	Status = CheckData();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Looks like we need this
	Xil_DCacheInvalidateRange((u32)outputBuf_ptr, MAX_PKT_LEN_RCV);
	free(combinedInput);
	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* @param	None
*
* @return
*		- XST_SUCCESS if validation is successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int CheckData(u8 *inBuf, u8 *outBuf, int size_of_data_bytes)  // This needs to be changed to deal with the input/output buffer we are now using
{
//	u32 *RxPacket;
//	u32 *TxPacket;
	int Index = 0;

//	RxPacket = (u32 *) RX_BUFFER_BASE;
//	TxPacket = (u32 *) TX_BUFFER_BASE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
//	Xil_DCacheInvalidateRange((u32)outBuf, size_of_data_bytes);

	xil_printf("Data sent: \r\n");
	for(Index = 0; Index < size_of_data_bytes; Index++) {
		xil_printf("0x%X ", inBuf[Index]);
	}
	xil_printf("\r\n");
	xil_printf("Data received: \r\n");
	for(Index = 0; Index < size_of_data_bytes; Index++) {
		xil_printf("0x%X ", outBuf[Index]);
	}
	xil_printf("\r\n");

	return XST_SUCCESS;
}

