/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "lwip/err.h"
#include "lwip/tcp.h"
#if defined (__arm__) || defined (__aarch64__)
#include "xil_printf.h"
#endif

// Defines
#define MEM_BASE_ADDR		(XPAR_PS7_DDR_0_S_AXI_BASEADDR + 0x10000000)
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x06500000) // buffer ~100MB
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x0C900000) // buffer ~100MB

volatile int return_data_available = 0;

int transfer_data() {
	return 0;
}

void print_app_header()
{
	xil_printf("\n\r\n\r-----File Store Server ------\n\r");
	xil_printf("TCP packets will be echoed back\n\r");
}

typedef enum
{
		STATE_INITIAL = 0,
		STATE_TRANSFER_NOT_COMPLETE,
		STATE_TRANSFER_COMPLETE,
}file_transfer_state_E;

// Static variables
static u32 size_of_byte_stream = 0;
static u32 remaining_bytes = 0;
static u8 *last_write_location;

static u32 bytes_to_be_sent_remaining = 0;
u8* last_address_sent;

static file_transfer_state_E recv_file_state = STATE_INITIAL;
static file_transfer_state_E send_file_state = STATE_INITIAL;

static struct tcp_pcb *pcb;

err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err)
{
	u8 *inputBuf_ptr = (u8*)TX_BUFFER_BASE;
	u8 *outputBuf_ptr = (u8*)RX_BUFFER_BASE;
	/* do not read the packet if we are not in ESTABLISHED state */
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	/* indicate that the packet has been received */
	tcp_recved(tpcb, p->len);

	switch(recv_file_state)
	{
		case STATE_INITIAL:
			// Clear the buffer just for debug purposes
//			memset(inputBuf_ptr, 0xAD, 100);
			// Grab the information from the header (how big will the file be?)
			memcpy(&size_of_byte_stream, p->payload, sizeof(size_of_byte_stream));
			if (size_of_byte_stream > p->len - sizeof(size_of_byte_stream))
			{
				// We know to expect more packets, so get ready to come to this state machine again
				recv_file_state = STATE_TRANSFER_NOT_COMPLETE;
				// Now let's store that data
				memcpy(inputBuf_ptr, (p->payload + sizeof(size_of_byte_stream)), p->len - sizeof(size_of_byte_stream));
				remaining_bytes = size_of_byte_stream - (p->len - sizeof(size_of_byte_stream));
				// And we'll need to save the place in the buffer where to write the next bit of data
				last_write_location = inputBuf_ptr + (p->len - sizeof(size_of_byte_stream));
			}
			else
			{
				memcpy(inputBuf_ptr, (p->payload + sizeof(size_of_byte_stream)), p->len - sizeof(size_of_byte_stream));
				recv_file_state = STATE_TRANSFER_COMPLETE;
			}
			break;
		case STATE_TRANSFER_NOT_COMPLETE:
			memcpy(last_write_location, p->payload, p->len);
			remaining_bytes -= p->len;
			if (remaining_bytes == 0)
			{
				// We have gotten all the info!
				recv_file_state = STATE_TRANSFER_COMPLETE;
				//temporarily add NULL to end of buffer for printing purposes
				*(last_write_location + p->len) = 0x00;
			}
			else
			{
				// Keep receiving data, update buf pointer
				last_write_location += (p->len);
			}
			break;
		case STATE_TRANSFER_COMPLETE:
			// Fall through for now
		default:
			//stuff
			break;
	}

	if (recv_file_state == STATE_TRANSFER_COMPLETE)
	{
		xil_printf("Yay we've got the data, it reads:\n %s\n", inputBuf_ptr);
		recv_file_state = STATE_INITIAL;
		// now do stuff here like encrypt the data
		// for now we'll just copy the data to our RX_Buffer and send that back
		memcpy(outputBuf_ptr, &size_of_byte_stream, sizeof(size_of_byte_stream)); // Add the byte header
		memcpy(outputBuf_ptr + sizeof(size_of_byte_stream), inputBuf_ptr, size_of_byte_stream);
//		err = tcp_write(tpcb, outputBuf_ptr, size_of_byte_stream + sizeof(size_of_byte_stream), 0); // We are NOT copying the buffer to the tcp layer
//		if (err != ERR_OK)
//		{
//			xil_printf("There is an error trying to send data over TCP\n\r");
//		}
		return_data_available = 1;
	}


	// Now we need to decode the header


	/* echo back the payload */
	/* in this case, we assume that the payload is < TCP_SND_BUF */
//	if (tcp_sndbuf(tpcb) > p->len) {
//		err = tcp_write(tpcb, p->payload, p->len, 1);
//	} else
//		xil_printf("no space in tcp_sndbuf\n\r");

	/* free the received pbuf */
	pbuf_free(p);

	return ERR_OK;
}

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;

	/* set the receive callback for this connection */
	tcp_recv(newpcb, recv_callback);
	// Let's hold onto this connection
	pcb = newpcb;

	/* just use an integer number indicating the connection id as the
	   callback argument */
	tcp_arg(newpcb, (void*)(UINTPTR)connection);

	/* increment for subsequent accepted connections */
	connection++;

	return ERR_OK;
}


// Contains the buffer to be sent, the first 32 bits are the size of the rest of the buffer
void send_data_over_ethernet(void)
{
	u8 *outputBuf_ptr = (u8*)RX_BUFFER_BASE;
	err_t err = ERR_OK;
	struct tcp_pcb *amazing = pcb; // Yeah this doesn't have the proper PCB in it, not the way to do it...clearly

	switch (send_file_state)
	{
		case STATE_INITIAL:
			// Need to extract size of file to be transferred
			xil_printf("I'm sending some data now look at me go!\n");
			memcpy(&size_of_byte_stream, outputBuf_ptr, sizeof(size_of_byte_stream));
			if (size_of_byte_stream < TCP_SND_BUF)
			{
				//Great we can fit everything in one send!
				err = tcp_write(pcb, (outputBuf_ptr + sizeof(size_of_byte_stream)), size_of_byte_stream, 1);
				send_file_state = STATE_TRANSFER_COMPLETE;
			}
			else
			{
				// Looks like we'll need to split up the data in multiple packets
				err = tcp_write(pcb, (outputBuf_ptr + sizeof(size_of_byte_stream)), TCP_SND_BUF, 1);
				bytes_to_be_sent_remaining = size_of_byte_stream - TCP_SND_BUF;
				last_address_sent = outputBuf_ptr + TCP_SND_BUF;
				send_file_state = STATE_TRANSFER_NOT_COMPLETE;
			}
			break;
		case STATE_TRANSFER_NOT_COMPLETE:
			xil_printf("I still have %i!\n", bytes_to_be_sent_remaining);
			if (bytes_to_be_sent_remaining <= TCP_SND_BUF)
			{
				// Cool, we can send the remaining bytes
				err = tcp_write(pcb, last_address_sent, bytes_to_be_sent_remaining, 1);
				send_file_state = STATE_TRANSFER_COMPLETE;
			}
			else
			{
				// We still have work to do, let's send more data
				err = tcp_write(pcb, last_address_sent, TCP_SND_BUF, 1);
				last_address_sent += TCP_SND_BUF;
				bytes_to_be_sent_remaining -= TCP_SND_BUF;
			}
			break;
		case STATE_TRANSFER_COMPLETE:
			return_data_available = 0;
			send_file_state = STATE_INITIAL;
			break;
		default:
			break;
	}
	if (err != ERR_OK)
	{
		xil_printf("We have an error with %i remaining\n", bytes_to_be_sent_remaining);
	}
}


int start_application()
{
//	struct tcp_pcb *pcb;
	err_t err;
	unsigned port = 7;

	// No data is available to send
	return_data_available = 0;

	/* create new TCP PCB structure */  //Daniel - we may need to make this public so we can send stuff in another function
	pcb = tcp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return -1;
	}

	/* bind to specified @port */
	err = tcp_bind(pcb, IP_ADDR_ANY, port);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
		return -2;
	}

	/* we do not need any arguments to callback functions */
	tcp_arg(pcb, NULL);

	/* listen for connections */
	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\n\r");
		return -3;
	}

	/* specify callback to use for incoming connections */
	tcp_accept(pcb, accept_callback);

	xil_printf("TCP echo server started @ port %d\n\r", port);

	return 0;
}
