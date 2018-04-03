/*
 * dma.c
 *
 *  Created on: Apr 1, 2018
 *      Author: jrtsang
 */

#include "common.h"

// Initialize DMA
int XAxiDma_Init(XAxiDma* AxiDma, u16 DeviceId)
{
	XAxiDma_Config *CfgPtr;
	int Status;

	/* Initialize the XAxiDma device.
	 */
	CfgPtr = XAxiDma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if(XAxiDma_HasSg(AxiDma)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);

	return XST_SUCCESS;
}


bool AES_Process_init(const uint8_t* key, enum AESMODE mode) {

	uint32_t *aes_process_address = (uint32_t*)XPAR_AES_PROCESS_0_S00_AXI_BASEADDR;

	// Set Key with register write
	*(aes_process_address + 0) =  key[0] << 24 |  key[1] << 16 |  key[2] << 8 |  key[3];
	*(aes_process_address + 1) =  key[4] << 24 |  key[5] << 16 |  key[6] << 8 |  key[7];
	*(aes_process_address + 2) =  key[8] << 24 |  key[9] << 16 | key[10] << 8 | key[11];
	*(aes_process_address + 3) = key[12] << 24 | key[13] << 16 | key[14] << 8 | key[15];

	// Set mode with register write
	switch(mode) {
		case ENCRYPTION:
			*(aes_process_address + 8) = 0x00000000;
			break;
		case DECRYPTION:
			*(aes_process_address + 8) = 0xFFFFFFFF;
			break;
		default:
			return false;
			break;
	};
	return true;
}


bool AES_Process(XAxiDma* AxiDma, u32 *inputBuf_ptr, u32 *outputBuf_ptr) {

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((u32)inputBuf_ptr, MAX_PKT_LEN_SEND);

	if (XST_SUCCESS != XAxiDma_SimpleTransfer(AxiDma,(u32) outputBuf_ptr, MAX_PKT_LEN_RCV, XAXIDMA_DEVICE_TO_DMA)) {
		return XST_FAILURE;
	}

	if (XST_SUCCESS != XAxiDma_SimpleTransfer(AxiDma,(u32) inputBuf_ptr, MAX_PKT_LEN_SEND, XAXIDMA_DMA_TO_DEVICE)) {
		return XST_FAILURE;
	}

	while (XAxiDma_Busy(AxiDma,XAXIDMA_DMA_TO_DEVICE)) {
		/* Wait */
	}
	while (XAxiDma_Busy(AxiDma,XAXIDMA_DEVICE_TO_DMA)) {
		/* Wait */
	}

	Xil_DCacheInvalidateRange((u32)outputBuf_ptr, MAX_PKT_LEN_RCV);

	return true;
}
