/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*!
 * \file usart.c
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 02.01.2014
 * \version 23
 *
 * \brief buffered transmission support for usart - implementation.
 * \note
 * For detailed description see header file.
 */

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/usart/usart.h"
#include "lib_mcu_noarch/util/util.h"
#include "lib/usart/usart_march.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

// compiletime checks
#ifndef USART_SEND_MAX_LENGTH
	#error "USART: USART_SEND_MAX_LENGTH not set"
#endif

#ifndef USART_BIG_BUFFERS
	#if (USART_RBUF_SIZE >= 0xFF | USART_TBUF_SIZE >= 0xFF)
		#error "USART: buffers too big (max = 256 bytes)"
	#endif
#endif


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
VOID usartFlush(usart_cfg_st *usart)
{
	usart->usart_rx_tail = 0;
	usart->usart_rx_head = 0;
	usart->usart_tx_tail = 0;
	usart->usart_tx_head = 0;

	usart->b_usart_rx_overflow = false;
	usart->b_usart_tx_full = false;
}

// --------------------------------------------------------------------------
USART_USARTREAD_ATTR BOOL usartRead(usart_cfg_st *usart, BYTE *pc_str, const UINT16 ui16_count, UINT16 ui16_val)
{
	for (UINT16 i = 0; i < ui16_count; ++i)
	{
		// wait for incomming byte no more than ui16_val miliseconds
		while (!usartUnreadBytes(usart))
		{
			if (ui16_val == 0)
				return (false);
			--ui16_val;
			delayMs(1);
		}

		*pc_str = usartReadByte(usart);
		++pc_str;
	}

	return (true);
}

// --------------------------------------------------------------------------
USART_USARTREADBYTE_ATTR BYTE usartReadByte(usart_cfg_st *usart)
{
	BYTE b;

	// wait for byte
	WAIT_FOR(!usartUnreadBytes(usart));

	// get byte from buffer and update pointers
	b = usart->ac_usart_rx_buff[usart->usart_rx_tail];
	usart->usart_rx_tail++;
	if (usart->usart_rx_tail > (USART_RBUF_SIZE - 1))
		usart->usart_rx_tail = 0;
	usart->b_usart_rx_overflow = false;

	return (b);
}

// --------------------------------------------------------------------------
BOOL usartReceiveBufferOverflow(const usart_cfg_st *usart)
{
	return (usart->b_usart_rx_overflow);
}

// --------------------------------------------------------------------------
VOID usartSend(usart_cfg_st *usart, const BYTE *pc_str, const UINT16 ui16_length)
{
	UINT16 ui16_computed_length;

	// compute max length of data to send
	if (!ui16_length)
		ui16_computed_length = USART_SEND_MAX_LENGTH;
	else
		ui16_computed_length = (ui16_length > USART_SEND_MAX_LENGTH)?(USART_SEND_MAX_LENGTH):(ui16_length);

	// start sending data
	for (UINT16 i = 0; i < ui16_computed_length; ++i)
		usartSendByte(usart, *(pc_str + i));
}

// --------------------------------------------------------------------------
USART_USARTSENDBYTE_ATTR VOID usartSendByte(usart_cfg_st *usart, const BYTE c_byte)
{
	// wait for free space in send buffer
	WAIT_FOR(usart->b_usart_tx_full);

	// insert byte to transmit buffer
	usart->ac_usart_tx_buff[usart->usart_tx_head] = c_byte;
	usart->usart_tx_head++;
	if (usart->usart_tx_head > (USART_TBUF_SIZE - 1))
		usart->usart_tx_head = 0;

	if (usart->usart_tx_head == usart->usart_tx_tail)
		usart->b_usart_tx_full = true;

	march_usartEnableTXEInterrupt(usart);
}

// --------------------------------------------------------------------------
USART_USARTSENDSTRING_ATTR VOID usartSendString(usart_cfg_st *usart, const BYTE *pc_str, const UINT16 ui16_length)
{
	UINT16 ui16_computed_length;

	// compute max length of data to send
	if (!ui16_length)
		ui16_computed_length = USART_SEND_MAX_LENGTH;
	else
		ui16_computed_length = (ui16_length > USART_SEND_MAX_LENGTH)?(USART_SEND_MAX_LENGTH):(ui16_length);

	// start sending data
	for (UINT16 i = 0; (i < ui16_computed_length) && *(pc_str + i); ++i)
		usartSendByte(usart, *(pc_str + i));
}

// --------------------------------------------------------------------------
USART_USARTUNREADBYTES_ATTR usart_bufsize_t usartUnreadBytes(const usart_cfg_st *usart)
{
	if (usart->usart_rx_head > usart->usart_rx_tail)
		return (usart->usart_rx_head - usart->usart_rx_tail);
	else if (usart->usart_rx_head < usart->usart_rx_tail)
		return (USART_RBUF_SIZE - (usart->usart_rx_tail - usart->usart_rx_head));
	else if (usart->b_usart_rx_overflow)
		return (USART_RBUF_SIZE);

	return (0);
}

// --------------------------------------------------------------------------
usart_bufsize_t usartUnsentBytes(const usart_cfg_st *usart)
{
	if (usart->usart_tx_head > usart->usart_tx_tail)
		return (usart->usart_tx_head - usart->usart_tx_tail);
	else if (usart->usart_tx_head < usart->usart_tx_tail)
		return (USART_TBUF_SIZE - (usart->usart_tx_tail - usart->usart_tx_head));
	else if (usart->b_usart_tx_full)
		return (USART_TBUF_SIZE);

	return (0);
}

// END
