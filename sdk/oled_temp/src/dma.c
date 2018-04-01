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
int CheckData(void)
{
	u32 *RxPacket;
	u32 *TxPacket;
	int Index = 0;

	RxPacket = (u32 *) RX_BUFFER_BASE;
	TxPacket = (u32 *) TX_BUFFER_BASE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
//	Xil_DCacheInvalidateRange((u32)RxPacket, MAX_PKT_LEN_RCV);

	xil_printf("Data sent: \r\n");
	for(Index = 0; Index < MAX_PKT_LEN_WORDS_SEND; Index++) {
		xil_printf("0x%X ", (unsigned int)TxPacket[Index]);
	}
	xil_printf("\r\n");
	xil_printf("Data received: \r\n");
	for(Index = 0; Index < MAX_PKT_LEN_WORDS_RCV; Index++) {
		xil_printf("0x%X ", (unsigned int)RxPacket[Index]);
	}
	xil_printf("\r\n");

	return XST_SUCCESS;
}
