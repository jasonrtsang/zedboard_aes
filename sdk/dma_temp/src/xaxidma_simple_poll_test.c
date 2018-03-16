#include <stdio.h>
#include "platform.h"
#include "xaxidma.h"
#include "xparameters.h"

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

int XAxiDma_SimplePollExample(u16 DeviceId);
static int CheckData(void);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;


int main()
{
	int Status;

    init_platform();
	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the poll example for simple transfer */
	Status = XAxiDma_SimplePollExample(DMA_DEV_ID);

	if (Status != XST_SUCCESS)
	{
		xil_printf("XAxiDma_SimplePollExample: Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("XAxiDma_SimplePollExample: Passed\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* The example to do the simple transfer through polling. The constant
* NUMBER_OF_TRANSFERS defines how many times a simple transfer is repeated.
*
* @param	DeviceId is the Device Id of the XAxiDma instance
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if error occurs
*
* @note		None
*
*
******************************************************************************/
int XAxiDma_SimplePollExample(u16 DeviceId)
{
	XAxiDma_Config *CfgPtr;
	int Status;
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;
	u32 *TxBufferPtr;
	u32 *RxBufferPtr;
//	u32 Value;

	TxBufferPtr = (u32 *)TX_BUFFER_BASE;
	RxBufferPtr = (u32 *)RX_BUFFER_BASE;

	u32 mode = 0xFFFFFFFF;
	u32 inputData[4] = {0x3ad77bb4, 0x0d7a3660, 0xa89ecaf3, 0x2466ef97};
	u32 key[4] = {0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c};

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

	// Set mode
	TxBufferPtr[0] = mode;

	// Set data
	TxBufferPtr[1] = inputData[0];
	TxBufferPtr[2] = inputData[1];
	TxBufferPtr[3] = inputData[2];
	TxBufferPtr[4] = inputData[3];

	// Set Key
	TxBufferPtr[5] = key[0];
	TxBufferPtr[6] = key[1];
	TxBufferPtr[7] = key[2];
	TxBufferPtr[8] = key[3];

//	for(Index = 0; Index < MAX_PKT_LEN_WORDS_SEND; Index ++) {
//			TxBufferPtr[Index] = (u32*)outputBuf[Index];
//	}
	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((u32)TxBufferPtr, MAX_PKT_LEN_SEND);

	for(Index = 0; Index < Tries; Index ++) {


		Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) RxBufferPtr,
					MAX_PKT_LEN_RCV, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) TxBufferPtr,
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

		Status = CheckData();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	}

	/* Test finishes successfully
	 */
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
static int CheckData(void)
{
	u32 *RxPacket;
	u32 *TxPacket;
	int Index = 0;

	RxPacket = (u32 *) RX_BUFFER_BASE;
	TxPacket = (u32 *) TX_BUFFER_BASE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((u32)RxPacket, MAX_PKT_LEN_RCV);

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
