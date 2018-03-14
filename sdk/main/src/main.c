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

/* SD Headers */
#include "ff.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xplatform_info.h"
#include "xsdps.h"

#include "xbasic_types.h"

/************************** Defines Prototypes *******************************/

/************************** Function Prototypes ******************************/
void prompt_file_input(char *fileName);
bool write_to_file(const char *sdFile, const uint8_t *writeBuf, const uint32_t writeSize);
bool read_from_file(const char *sdFile, uint8_t *readBuf, uint32_t *readSize);

/************************** Variable Definitions *****************************/
#define FILENAME_LIMIT 32
#define USERINPUT_LIMIT 29
#define TESTBIN_SIZE_16 16
#define TESTBIN_SIZE_64 64
static FIL fil; // Specified file input

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

    /* Fixed key and initialization vector (cbc) */
    const uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    const uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    /* Universally used variables */
    int i, menuChoice, modeChoice, testBinChoice;
    uint32_t fileSizeRead;

    const TCHAR *Path = "0:/"; // Base directory of SD
    static FATFS fatfs; // File system format

    char fileNameIn[FILENAME_LIMIT] = ""; // Specified file input name
    char fileNameOut[FILENAME_LIMIT]; // Output file name

    static uint8_t inputBuf[10*1024*1024] __attribute__ ((aligned(32))); // 10mb buffers [1024*1024 == 1mb, 1024 == 1kb]
    static uint8_t outputBuf[10*1024*1024] __attribute__ ((aligned(32)));

    struct AES_ctx ctx; // Context

    bool exitFlag = false; // Application closure

    /* Initialize platform */
    init_platform();

#ifdef MILESTONE5
    XGpio gpioSwitches;
    XGpio gpioLeds;

	uint8_t dipValue;
	int switch_count;
	int test_mask;

	// Lookup the config information and store it in the struct "GPIO_Config"
//	gpioSwLed_Config = XGpio_LookupConfig(XPAR_SW_LED_GPIO_AXI_DEVICE_ID);

	if(XGpio_Initialize(&gpioSwitches, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) { // AXI GPIO
	        printf("UH OH: GPIO SW0 initialization failed\r\n");
	};
	if(XGpio_Initialize(&gpioLeds, XPAR_SW_LED_GPIO_AXI_DEVICE_ID) != XST_SUCCESS) { // AXI GPIO
	        printf("UH OH: GPIO SW0 initialization failed\r\n");
	};

	// Set the direction of the bits in the GPIO.
	// The lower (LSB) 8 bits of the GPIO are for the DIP Switches (inputs).
	// The upper (MSB) 8 bits of the GPIO are for the LEDs (outputs).
	XGpio_SetDataDirection(&gpioSwitches, 1, 0x00FF);
	XGpio_SetDataDirection(&gpioLeds, 2, 0x0000);

    uint8_t switchKey[16];
#endif // MILESTONE5



    /* Main application */
    while(!exitFlag) {


    	// Go around in a loop for ever
		while (1)
		{
			// Read from the GPIO to determine the position of the DIP switches
			dipValue = XGpio_DiscreteRead(&gpioSwitches, 1);
			// Write the value back to the LEDS
			XGpio_DiscreteWrite(&gpioLeds, 2, dipValue);
		}



        printf("\r\n\r\n");
        printf("~~~~~~~~~~~~~~~~ AES Encryption/ Decryption ~~~~~~~~~~~~~~~\r\n");
        printf(" SELECT the operation from the below menu \r\n");
        printf(" TOGGLE 'SW0' high for verbose output\r\n");
        printf(" NOTE There is a single fixed key for this application\r\n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~ Menu Starts ~~~~~~~~~~~~~~~~~~~~~~~\r\n");
        printf(" Press '1' to Format SD card\r\n");
        printf(" Press '2' to Create TEST.BIN file\r\n");
        printf(" Press '3' to Specify current file\r\n");
        printf(" Press '4' to Enter CBC mode submenu\r\n");
        printf(" Press '5' to Enter ECB mode submenu\r\n");
        printf(" Press '6' to Byte compare two files\r\n");
        printf(" Press any other key to exit\r\n");
        printf("~~~~~~~~~~~~~~~~~~~~~~~~ Menu Ends ~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
        printf(" Current File: ");
        for(i = 0; i < FILENAME_LIMIT; i++) {
            printf("%c", fileNameIn[i]);
        }
        printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
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
                break;

            /* Current file input */
            case '3':
                prompt_file_input(fileNameIn);
                break;

            /* CBC */
            case '4':
                printf("\r\n\r\n");
                printf("~~~~~~~~~~~~~~~~~~~~~~~ CBC Mode ~~~~~~~~~~~~~~~~~~~~~~~\r\n");
                printf(" Press '1' to Run encryption\r\n");
                printf(" Press '2' to Run decryption\r\n");
                printf(" Press any other key to return to main menu\r\n");
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
                /* Poll on mode selection */
                modeChoice = inbyte();
                printf(" Selection : %c\r\n\r\n", modeChoice);

                /* Read the current specified file */
                fileSizeRead = 0;
                if(!read_from_file(fileNameIn, inputBuf, &fileSizeRead)) {
                    break;
                }

                switch(modeChoice) {
                    case '1': /* Encrypt */
                        printf("Starting CBC encrypt...\r\n");
                        printf("> Name encrypted file output\r\n");
                        prompt_file_input(fileNameOut);
                        AES_init_ctx_iv(&ctx, key, iv);
                        AES_CBC_encrypt_buffer(&ctx, inputBuf, fileSizeRead, 0);
                        printf("Writing encrypted file to SD card...\r\n");
                        /* Create output file */
                        write_to_file(fileNameOut, inputBuf, fileSizeRead);
                        printf("Done!\r\n");
                        break;
                    case '2': /* Decrypt */
                        printf("Starting CBC decrypt...\r\n");
                        printf("> Name decrypted file output\r\n");
                        prompt_file_input(fileNameOut);
                        AES_init_ctx_iv(&ctx, key, iv);
                        AES_CBC_decrypt_buffer(&ctx, inputBuf, fileSizeRead, 0);
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
            case '5':
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

                /* Read the current specified file */
                fileSizeRead = 0;
                if (!read_from_file(fileNameIn, inputBuf, &fileSizeRead)) {
                    break;
                }

                switch(modeChoice) {
                    case '1': /* Encrypt */
                        printf("Starting ECB encrypt...\r\n");
                        printf("> Name encrypted file output\r\n");
                        prompt_file_input(fileNameOut);


#ifdef MILESTONE5



#endif // MILESTONE5

                        AES_init_ctx(&ctx, key);

#ifdef MILESTONE2
                        uint32_t *baseaddr_p = (uint32_t *)XPAR_KEY_EXPANSION_0_S00_AXI_BASEADDR;
                        /* Input key */
                        uint32_t input_0 = key[3] | key[2] << 8 | key[1] << 16 | key[0] << 24;
                        *(baseaddr_p+0) = input_0;
                        uint32_t input_1 = key[7] | key[6] << 8 | key[5] << 16 | key[4] << 24;
                        *(baseaddr_p+1) = input_1;
                        uint32_t input_2 = key[11] | key[10] << 8 | key[9] << 16 | key[8] << 24;
                        *(baseaddr_p+2) = input_2;
                        uint32_t input_3 = key[15] | key[14] << 8 | key[13] << 16 | key[12] << 24;
                        *(baseaddr_p+3) = input_3;
                        *(baseaddr_p+48) = ENCRYPTION;

                        /* Output round keys */
                        j = 0;
                        for (i = 4; i < 48; i++) {
                            ctx_block.RoundKey[j++] = (*(baseaddr_p+i) >> 24) & 0xFF;
                            ctx_block.RoundKey[j++] = (*(baseaddr_p+i) >> 16) & 0xFF;
                            ctx_block.RoundKey[j++] = (*(baseaddr_p+i) >> 8) & 0xFF;
                            ctx_block.RoundKey[j++] = *(baseaddr_p+i) & 0xFF;
                        }

                        /* Print IP block vs SW implementation */
                        printf("\nHW Round Keys\n");
                        for (i = 0; i < 176; i++) {
                            printf("%.2x ", ctx_block.RoundKey[i]);
                        }
                        printf("\n\nSW Round Keys\n");
                        for (i = 0; i < 176; i++) {
                            printf("%.2x ", ctx.RoundKey[i]);
                        }
                        printf("\n");

                        /* Diff SW and HW to validate */
                        for (i = 0; i < 176; i++) {
                            if (ctx.RoundKey[i] != ctx_block.RoundKey[i]) {
                                printf("UH OH: SW Key Expansion does not match that of HW\r\n");
                            }
                        }
#endif // MILESTONE2

                        AES_ECB_encrypt_buffer(&ctx, inputBuf, fileSizeRead, 0);
                        printf("Writing encrypted file to SD card...\r\n");
                        /* Create output file */
                        write_to_file(fileNameOut, inputBuf, fileSizeRead);
                        printf("Done!\r\n");
                        break;
                    case '2': /* Decrypt */
                        printf("Starting ECB decrypt...\r\n");
                        printf("> Name decrypted file output\r\n");
                        prompt_file_input(fileNameOut);
                        AES_init_ctx(&ctx, key);

#ifdef MILESTONE3
                        uint32_t *baseaddr_p = (uint32_t *)XPAR_KEY_EXPANSION_0_S00_AXI_BASEADDR;
                        /* Input key */
                        uint32_t input_0 = key[3] | key[2] << 8 | key[1] << 16 | key[0] << 24;
                        *(baseaddr_p+0) = input_0;
                        uint32_t input_1 = key[7] | key[6] << 8 | key[5] << 16 | key[4] << 24;
                        *(baseaddr_p+1) = input_1;
                        uint32_t input_2 = key[11] | key[10] << 8 | key[9] << 16 | key[8] << 24;
                        *(baseaddr_p+2) = input_2;
                        uint32_t input_3 = key[15] | key[14] << 8 | key[13] << 16 | key[12] << 24;
                        *(baseaddr_p+3) = input_3;
                        *(baseaddr_p+48) = ENCRYPTION;

                        /* Output round keys */
                        j = 0;
                        for (i = 4; i < 48; i++) {
                            ctx_block.RoundKey[j++] = (*(baseaddr_p+i) >> 24) & 0xFF;
                            ctx_block.RoundKey[j++] = (*(baseaddr_p+i) >> 16) & 0xFF;
                            ctx_block.RoundKey[j++] = (*(baseaddr_p+i) >> 8) & 0xFF;
                            ctx_block.RoundKey[j++] = *(baseaddr_p+i) & 0xFF;
                        }

                        /* Print IP block vs SW implementation */
                        printf("\nHW Round Keys\n");
                        for (i = 0; i < 176; i++) {
                            printf("%.2x ", ctx_block.RoundKey[i]);
                        }
                        printf("\n\nSW Round Keys\n");
                        for (i = 0; i < 176; i++) {
                            printf("%.2x ", ctx.RoundKey[i]);
                        }
                        printf("\n");

                        /* Diff SW and HW to validate */
                        for (i = 0; i < 176; i++) {
                            if (ctx.RoundKey[i] != ctx_block.RoundKey[i]) {
                                printf("UH OH: SW Key Expansion does not match that of HW\r\n");
                            }
                        }
#endif // MILESTONE3

                        AES_ECB_decrypt_buffer(&ctx, inputBuf, fileSizeRead, 0);
                        printf("Writing decrypted file to SD card...\r\n");
                        /* Create output file */
                        write_to_file(fileNameOut, inputBuf, fileSizeRead);
                        printf("Done!\r\n");
                        break;
                    default:
                        break;
                }
                break;

            /* Byte file comparision */
            case '6':
                printf("Starting byte comparison...\r\n");
                char firstFile[FILENAME_LIMIT] = "", secondFile[FILENAME_LIMIT] = "";
                uint32_t firstFileSize = 0, secondFileSize = 0;
                bool different = false;

                printf("> 1st file\n\r");
                prompt_file_input(firstFile);
                printf("> 2nd file\n\r");
                prompt_file_input(secondFile);

                /* Open both files */
                if(!read_from_file(firstFile, inputBuf, &firstFileSize) || !read_from_file(secondFile, outputBuf, &secondFileSize)) {
                    break;
                }

                /* Size verification */
                if (firstFileSize != secondFileSize) {
                    printf("UH OH: Their file sizes don't match\r\n");
                } else {
                    /* Data verification */
                    printf("Verifying content...\r\n");
                    for(i = 0; i < firstFileSize; i++){
                       if(inputBuf[i] != outputBuf[i]){
                           different = true;
                           printf("UH OH: THEY LIKE DONT MATCH CONTENT AT ALL\r\n");
                           break;
                       }
                    }
                }
                if(!different) {
                    printf("Yup, Both files match!\r\n");
                    printf("Done!\r\n");
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
