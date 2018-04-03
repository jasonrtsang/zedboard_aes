/*
 * common.h
 *
 *  Created on: Mar 18, 2018
 *      Author: jason
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include "xil_types.h"
#include "xgpio.h"
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xgpiops.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xscugic.h"

#include <unistd.h>


/* SD Headers */
#include "ff.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xplatform_info.h"
#include "xsdps.h"

/* DMA Header */
#include "xaxidma.h"

#include "aes.h"


typedef uint8_t bool;
#define true 1
#define false 0

enum DPAD {CENTER = 1, DOWN = 2, UP = 16, LEFT = 4, RIGHT = 8};

enum AESMODE {ENCRYPTION = 0, DECRYPTION = 1};

#define DEBOUNCE_DELAY 125000 // 0.25 seconds

#define TESTBIN_SIZE_16 16
#define TESTBIN_SIZE_64 64

/* MIO51 BTN9 Setup*/
#define XPAR_AXI_XADC_0_DEVICE_ID 0

#define GPIO_DEVICE_ID      XPAR_XGPIOPS_0_DEVICE_ID
#define INTC_DEVICE_ID      XPAR_SCUGIC_SINGLE_DEVICE_ID
#define GPIO_INTERRUPT_ID   XPS_GPIO_INT_ID

#define pbsw 51

/* DMA */
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#define MEM_BASE_ADDR		(XPAR_PS7_DDR_0_S_AXI_BASEADDR + 0x10000000)

#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x0605E100) // Receiving buffer start 100MB after Transfer Buffer
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x0BFBC200) // Receiving buffer ends at 100MB

#define MAX_PKT_LEN_WORDS_SEND	4
#define MAX_PKT_LEN_SEND			MAX_PKT_LEN_WORDS_SEND*4

#define MAX_PKT_LEN_WORDS_RCV	4
#define MAX_PKT_LEN_RCV			MAX_PKT_LEN_WORDS_RCV*4

#define NUMBER_OF_TRANSFERS	1


/************************** Function Prototypes ******************************/

/********************************** OLED *************************************/
void print_screen(char* printLines[]);
bool confirmation_screen(XGpio* gpioBtn, char* printLines[]);
int selection_screen(XGpio* gpioBtn, char* printLines[], int numOfLines);
char** format_fileList(char* inputFiles[], int numOfFiles);
void format_cursor(char inputLine[], char *outputLine);
void refresh_oled (char* printLines[], int numOfLines, int index, bool menu);

/*********************************** SD *************************************/
bool init_sd(FATFS* fatfs);
bool format_sd(void);
bool create_test_bin(int choice);
bool write_to_file(const char *sdFile, const uint32_t *writeBuf, const uint32_t writeSize);
bool read_from_file(const char *sdFile, uint32_t *readBuf, uint32_t *readSize);
char** list_all_files(int* numOfFiles);

/******************************** INTERRUPT *********************************/
void My_XGpioPs_IntrHandler(XGpioPs *InstancePtr);
void SetupInterruptSystem(XScuGic *GicInstancePtr, XGpioPs *Gpio, u16 GpioIntrId);
void IntrHandler(void *CallBackRef, int Bank, u32 Status);

/**********************************  DMA ************************************/
int XAxiDma_Init(XAxiDma* AxiDma, u16 DeviceId);
bool AES_Process(XAxiDma* AxiDma, u32 *inputBuf_ptr, u32 *outputBuf_ptr);
bool AES_Process_init(const uint8_t* key, enum AESMODE mode);

#endif /* SRC_COMMON_H_ */
