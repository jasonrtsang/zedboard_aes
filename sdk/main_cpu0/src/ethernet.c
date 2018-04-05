/*
 * ethernet.c
 *
 *  Created on: Apr 3, 2018
 *      Author: drdixon
 */

#include "common.h"
#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include "xil_cache.h"

#include "netif/xadapter.h"


/* defined by each RAW mode application */
int start_application();
int transfer_data();
void send_data_over_ethernet();

void tcp_fasttmr(void);
void tcp_slowtmr(void);

/* missing declaration in lwIP */
void lwip_init();

// More functions found in original Echo.c
err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
// In platform.c
void platform_enable_interrupts();
void enable_cache_platform();
void disable_cache_platform();
void disable_timer_interrupt();
void enable_timer_interrupt();

extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);

void perform_encryption(uint32_t *outputBuf, uint32_t *inputBuf, u32 data_len, enum AESMODE mode);

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *echo_netif;

static XAxiDma *ethernetAxiDma;

// TODO - Make an init funciton for these
static int ethernet_initialized = 0;
static int application_initialized = 0;

typedef enum
{
		STATE_INITIAL = 0,
		STATE_TRANSFER_NOT_COMPLETE,
		STATE_TRANSFER_COMPLETE,
}file_transfer_state_E;

// STATIC VARIABLES FOUND IN ECHO.C
static u32 size_of_byte_stream = 0;
static u32 header_info = 0;
static u32 remaining_bytes = 0;
static u8 *last_write_location;

static u32 bytes_to_be_sent_remaining = 0;
u8* last_address_sent;

static file_transfer_state_E recv_file_state = STATE_INITIAL;
static file_transfer_state_E send_file_state = STATE_INITIAL;


static struct ip_addr ipaddr, netmask, gw;

static struct tcp_pcb *pcb;
// END OF STATIC IN ECHO.C

// Now let's have some stuff for the OLED
char* enteringEthernetMode[] = {"    ENTERING    ",
							    "    ETHERNET    ",
							    "     MODE       ",
							    "                "};

char printbuf[16];


void print_ip(char *msg, struct ip_addr *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}

int ethernet_mode_run(XAxiDma *axiDma)
{
	// Notify the user of ethernet mode
	oled_print_screen(enteringEthernetMode);

	// Hold onto the axiDma address for use in the callback function
	ethernetAxiDma = axiDma;

	if (0 == ethernet_initialized)
	{
		/* the mac address of the board. this should be unique per board */
		unsigned char mac_ethernet_address[] =
		{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

		echo_netif = &server_netif;

		init_platform();

		ipaddr.addr = 0;
		gw.addr = 0;
		netmask.addr = 0;

		lwip_init();

		if (!xemac_add(echo_netif, &ipaddr, &netmask,
							&gw, mac_ethernet_address,
							XPAR_XEMACPS_0_BASEADDR)) {
			xil_printf("Error adding N/W interface\n\r");
			return -1;
		}
		netif_set_default(echo_netif);

		/* now enable interrupts */
		platform_enable_interrupts();
	
		/* specify that the network if is up */
		netif_set_up(echo_netif);

		/* Create a new DHCP client for this interface.
		 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
		 * the predefined regular intervals after starting the client.
		 */
		dhcp_start(echo_netif);
		dhcp_timoutcntr = 24;
	
		while(((echo_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
			xemacif_input(echo_netif);
	
		if (dhcp_timoutcntr <= 0) {
			if ((echo_netif->ip_addr.addr) == 0) {
				xil_printf("DHCP Timeout\r\n");
				xil_printf("Configuring default IP of 192.168.1.10\r\n");
				IP4_ADDR(&(echo_netif->ip_addr),  192, 168,   1, 10);
				IP4_ADDR(&(echo_netif->netmask), 255, 255, 255,  0);
				IP4_ADDR(&(echo_netif->gw),      192, 168,   1,  1);
			}
		}
	
		ipaddr.addr = echo_netif->ip_addr.addr;
		gw.addr = echo_netif->gw.addr;
		netmask.addr = echo_netif->netmask.addr;

//		disable_cache_platform();

		ethernet_initialized = 1;
	}
	else
	{
		// We are already initialized
	}
	
	// TODO, PRINT IP ADDRESS AND PORT HERE ON OLED
	print_ip_settings(&ipaddr, &netmask, &gw);
	
	oled_clear();

	sprintf(printbuf, "   Ethernet IP  ");
	oled_print_line(printbuf, 0);
	sprintf(printbuf, "%d.%d.%d.%d", ip4_addr1(&ipaddr), ip4_addr2(&ipaddr),ip4_addr3(&ipaddr), ip4_addr4(&ipaddr));
	oled_print_line(printbuf, 1);


	start_application();
	
	/* receive and process packets */
	while (1) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(echo_netif);
		transfer_data();
		// TODO - NEED TO ADD CANCEL BUTTON INTERRUPT HERE
		if (cancelFlag)
		{
			// Do some cleanup
			tcp_close(pcb);
			break;
		}
	}

	return 0;
}

int transfer_data() {
	return 0;
}

int start_application()
{
	err_t err;
	unsigned port = 7;
	if (application_initialized == 0)
	{
		/* create new TCP PCB structure */
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
		application_initialized = 1;
	}
	else
	{
		// We have already set up the pcb
	}

	xil_printf("TCP echo server started @ port %d\n\r", port);

	sprintf(printbuf, "      Port      ");
	oled_print_line(printbuf, 2);
	sprintf(printbuf, "        %i", port);
	oled_print_line(printbuf, 3);

	// Try adding the cancel button GIC back
	gic_init();

	return 0;
}

err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err)
{
	enum AESMODE mode;
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
			// Grab the information from the header (how big will the file be and what mode?)
			memcpy(&size_of_byte_stream, p->payload, sizeof(size_of_byte_stream));
			memcpy(&header_info, p->payload + sizeof(size_of_byte_stream), sizeof(header_info));

			remaining_bytes = size_of_byte_stream - (p->len - sizeof(header_info));

			if (0 == remaining_bytes)
			{
				memcpy(inputBuf_ptr, (p->payload + sizeof(size_of_byte_stream) + sizeof(header_info)), p->len - sizeof(size_of_byte_stream) - sizeof(header_info));
				recv_file_state = STATE_TRANSFER_COMPLETE;
			}
			else
			{
				// We know to expect more packets, so get ready to come to this state machine again
				recv_file_state = STATE_TRANSFER_NOT_COMPLETE;
				// Now let's store that data
				memcpy(inputBuf_ptr, (p->payload + sizeof(size_of_byte_stream) + sizeof(header_info)), p->len - sizeof(size_of_byte_stream) - sizeof(header_info));
				remaining_bytes = size_of_byte_stream - (p->len - sizeof(header_info));
				// And we'll need to save the place in the buffer where to write the next bit of data
				last_write_location = inputBuf_ptr + (p->len - sizeof(size_of_byte_stream) - sizeof(header_info));
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

	/* free the received pbuf */
	pbuf_free(p);


	if (recv_file_state == STATE_TRANSFER_COMPLETE)
	{
		xil_printf("Data transfer complete\n\r");
		recv_file_state = STATE_INITIAL;
		send_file_state = STATE_INITIAL;

		if (0 == header_info)
		{
			mode = ENCRYPTION;
		}
		else
		{
			mode = DECRYPTION;
		}
		// TODO - Temp memset, can remove soon
		memset(outputBuf_ptr, 0xAD, 100);

		// We want to remove the header info count from the byte stream size
		size_of_byte_stream -= sizeof(header_info);

		perform_encryption((u32*)(outputBuf_ptr + sizeof(size_of_byte_stream)), (u32*)(inputBuf_ptr), size_of_byte_stream, mode); // Ignore the headers

		memcpy(outputBuf_ptr, &size_of_byte_stream, sizeof(size_of_byte_stream)); // Add the byte header

		// Kick off the send data
		send_data_over_ethernet();
	}

	return ERR_OK;
}

void perform_encryption(uint32_t *outputBuf, uint32_t *inputBuf, u32 data_len, enum AESMODE mode)
{
	int bytesToPad;
	int ii = 0;
	const uint8_t ethernet_key[16] = {0x72, 0x42, 0xf8, 0xeb, 0xe2, 0xca, 0x6c, 0x20, 0x6c, 0xd8, 0xdf, 0x1a, 0xcd, 0xe3, 0xfd, 0xe7};

	bytesToPad = 0;
 	if(mode == ENCRYPTION) {
		bytesToPad = 16 - data_len % 16; // Difference of remainder length in bytes

		char pad = (char)(16-data_len%16); // Character pattern used for padding
		uint8_t *inputBufEnd = (uint8_t *)TX_BUFFER_BASE;
		for(int i = 0; i < bytesToPad ; i++) {
			*(inputBufEnd+data_len+i) = pad;
		}
		size_of_byte_stream += bytesToPad;
	}
	
	aes_process_init(ethernet_key, mode);

	// Loop till entire file is done
	for(ii = 0; ii < (data_len + bytesToPad); ii += AES_BLOCKLEN)
	{
		// Stream state to AES_PROCESS IP
		dma_aes_process_transfer(ethernetAxiDma, inputBuf, outputBuf);
		dma_aes_process_transfer(ethernetAxiDma, inputBuf, outputBuf);
		inputBuf += AES_BLOCKLEN/4;
		outputBuf += AES_BLOCKLEN/4;
		// Cancel interrupt flag
		if (cancelFlag) {
			xil_printf("CANCEL FLAG WORKED");
			return;
		}
	}

	// Remove padding if decryption by adjusting the data_len
	if(mode == DECRYPTION) {
		int newFileSize = data_len;
		uint8_t *outputBufEnd = (uint8_t *)RX_BUFFER_BASE + sizeof(size_of_byte_stream);
		for(int i = data_len; i > data_len-16-1; i--) {
			if(*(outputBufEnd+data_len-1) == *(outputBufEnd+i-2)) {
				newFileSize-=1; // If last padding character matches previous, decrease file size
			}
		}
		newFileSize-=1; // Remove original last one used for comparison
		data_len = newFileSize; // TODO Need to modify the header to be sent over TCP - this is not updated yet
		// Let's see what data_len is compared to the header value, maybe we'll change it here
		size_of_byte_stream = newFileSize;
	}
}


err_t sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  LWIP_UNUSED_ARG(len);

  // Nice, let's use this to send the next data if needed
  if (send_file_state == STATE_TRANSFER_COMPLETE)
  {
	  // Reset file send state machine
	  // Reset both sent and receive
	  send_file_state = STATE_INITIAL;
	  recv_file_state = STATE_INITIAL;
  }
  else
  {
	  // More data still needs to be sent
	  send_data_over_ethernet();
  }
  return ERR_OK;
}


err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;

	/* set the receive callback for this connection */
	tcp_recv(newpcb, recv_callback);

	/* set the sent callback for this connection */
	tcp_sent(newpcb, sent_callback);

	// Let's hold onto this connection for sending data later
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

	switch (send_file_state)
	{
		case STATE_INITIAL:
			// Need to extract size of file to be transferred
			xil_printf("\nSending data to host!\n");

			// WARINING, SHITTY HACK, subtract one from size of byte stream because we aren't including the mode anymore
			// TODO - I don't think this hack needs to be here, will confirm soon
//			size_of_byte_stream -= sizeof(size_of_byte_stream);
			memcpy(&size_of_byte_stream, outputBuf_ptr, sizeof(size_of_byte_stream));
			if ((size_of_byte_stream + sizeof(size_of_byte_stream)) < TCP_SND_BUF)
			{
				//Great we can fit everything in one send!
				err = tcp_write(pcb, outputBuf_ptr, size_of_byte_stream + sizeof(size_of_byte_stream), 1); // include sending the header
				if (err != ERR_OK)
				{
					// Early return, stay in initial state
					return;
				}
				else
				{
					send_file_state = STATE_TRANSFER_COMPLETE;
				}
			}
			else
			{
				// Looks like we'll need to split up the data in multiple packets
				err = tcp_write(pcb, outputBuf_ptr, TCP_SND_BUF, 1);
				if (err != ERR_OK)
				{
					// Early return, stay in initial state
					return;
				}
				else
				{
					bytes_to_be_sent_remaining = size_of_byte_stream - TCP_SND_BUF;
					last_address_sent = outputBuf_ptr + TCP_SND_BUF;
					send_file_state = STATE_TRANSFER_NOT_COMPLETE;
				}
			}
			break;
		case STATE_TRANSFER_NOT_COMPLETE:
			if (bytes_to_be_sent_remaining <= TCP_SND_BUF)
			{
				// Cool, we can send the remaining bytes
				err = tcp_write(pcb, last_address_sent, bytes_to_be_sent_remaining + sizeof(size_of_byte_stream), 1); // Include header in transmission
				if (err != ERR_OK)
				{
					// Failed to send data, early return, do not update anything
					return;
				}
				else
				{
					send_file_state = STATE_TRANSFER_COMPLETE;
					xil_printf("\nYAY Transfer is complete\n\r");
				}
			}
			else
			{
				// We still have work to do, let's send more data
				err = tcp_write(pcb, last_address_sent, TCP_SND_BUF, 1);
				if (err != ERR_OK)
				{
					// Failed to send data, early return, do not update anything
					return;
				}
				else
				{
					last_address_sent += TCP_SND_BUF;
					bytes_to_be_sent_remaining -= TCP_SND_BUF;
				}
			}
			break;
		case STATE_TRANSFER_COMPLETE:
			// Take no action
			break;
		default:
			break;
	}
}

