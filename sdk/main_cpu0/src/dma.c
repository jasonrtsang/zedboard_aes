/*
 * dma.c
 *
 *  Created on: Apr 1, 2018
 *      Author: jrtsang
 */

#include "common.h"

/******************************* Definitions *********************************/
#define DMA_DEV_ID		       XPAR_AXIDMA_0_DEVICE_ID

#define MAX_PKT_LEN_WORDS_SEND 4
#define MAX_PKT_LEN_SEND       MAX_PKT_LEN_WORDS_SEND*4

#define MAX_PKT_LEN_WORDS_RCV  4
#define MAX_PKT_LEN_RCV        MAX_PKT_LEN_WORDS_RCV*4
/*****************************************************************************/

/*****************************************************************************/
/**
*
* Initializing DMA
*
* @param    XAxiDma* axiDma           : Pointer to AXI DMA
*
* @return   true if successful, false otherwise
*
* @note     None
*
**/
/*****************************************************************************/
bool dma_init(XAxiDma *axiDma)
{
	XAxiDma_Config *CfgPtr;
	int Status;

	// Initialize the XAxiDma device
	CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
	if(!CfgPtr) {
#if UART_PRINT
		printf("No config found for %d\r\n", DMA_DEV_ID);
#endif
		return false;
	}
	Status = XAxiDma_CfgInitialize(axiDma, CfgPtr);
	if(Status != XST_SUCCESS) {
#if UART_PRINT
		xil_printf("Initialization failed %d\r\n", Status);
#endif
		return false;
	}
	if(XAxiDma_HasSg(axiDma)){
#if UART_PRINT
		xil_printf("Device configured as SG mode \r\n");
#endif
		return false;
	}

	// Disable interrupts, using polling mode for DMA
	XAxiDma_IntrDisable(axiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(axiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	return true;
}

/*****************************************************************************/
/**
*
* Streaming cipher states to the IP block
*
* @param    XAxiDma* axiDma           : Pointer to AXI DMA
* 			uint32_t *inputBuf        : Input state to stream to IP
* 			uint32_t *outputBuf       : Output state from IP
*
* @return   true if successful, false otherwise
*
* @note     None
*
**/
/*****************************************************************************/
bool dma_aes_process_transfer(XAxiDma* axiDma, uint32_t *inputBuf, uint32_t *outputBuf) {

	// Flush the inputBuf before the DMA transfer, in case the Data Cache is enabled
	Xil_DCacheFlushRange((u32)inputBuf, MAX_PKT_LEN_SEND);

	if(XST_SUCCESS != XAxiDma_SimpleTransfer(axiDma,(u32) outputBuf, MAX_PKT_LEN_RCV, XAXIDMA_DEVICE_TO_DMA)) {
		return false;
	}

	if(XST_SUCCESS != XAxiDma_SimpleTransfer(axiDma,(u32) inputBuf, MAX_PKT_LEN_SEND, XAXIDMA_DMA_TO_DEVICE)) {
		return false;
	}

	while (XAxiDma_Busy(axiDma,XAXIDMA_DMA_TO_DEVICE));
	while (XAxiDma_Busy(axiDma,XAXIDMA_DEVICE_TO_DMA));

	// Invalidate outputBuf cache to receive data from DMA
	Xil_DCacheInvalidateRange((u32)outputBuf, MAX_PKT_LEN_RCV);

	return true;
}
