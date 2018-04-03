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

typedef uint8_t bool;
#define true 1
#define false 0

extern bool cancelFlag;

enum AESMODE {ENCRYPTION, DECRYPTION};
enum STATUS {DONE, FAILED, BACK, CANCELLED};

// DMA
#define MEM_BASE_ADDR  (XPAR_PS7_DDR_0_S_AXI_BASEADDR + 0x10000000)
#define TX_BUFFER_BASE (MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE (MEM_BASE_ADDR + 0x06500000) // Receiving buffer start 100MB after TX_BUFFER_BASE

// CPU1
#define COMM_VAL  (*(volatile unsigned long *)(0xFFFF0000))
#define FILESIZE_VAL    (*(volatile unsigned long *)(0xFFFF0004))

/********************************** OLED *************************************/
void   oled_init(void);
void   oled_print_screen(char* printLines[]);
bool   oled_confirmation_screen(char* printLines[]);
int    oled_selection_screen(char* printLines[], int numOfLines);
char** oled_format_fileList(char* inputFiles[], int numOfFiles);

/******************************** INTERRUPT *********************************/
bool gic_init(void);

/*********************************** SD *************************************/
bool   sd_init(FATFS* fatfs);
bool   sd_format(void);
bool   sd_write_to_file(const char *sdFile, const uint32_t *writeBuf, const uint32_t writeSize);
bool   sd_read_from_file(const char *sdFile, uint32_t *readBuf, uint32_t *readSize);
char** sd_list_all_files(int *numOfFiles);

/**********************************  DMA ************************************/
bool dma_init(XAxiDma *axiDma);
bool dma_aes_process_transfer(XAxiDma* axiDma, uint32_t *inputBuf, uint32_t *outputBuf);

/**********************************  AES ************************************/
void aes_init(void);
enum STATUS aes_sd_process_run(enum AESMODE mode);

#endif /* SRC_COMMON_H_ */
