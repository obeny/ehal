/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*!
 * \file rs4xx.c
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 11.03.2016
 * \version 1
 *
 * \brief Blocking RS422/485 interface handler - implementation.
 * \note
 * For detailed description see header file.
 */

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "config.h"
#include "lib/io.h"
#include "lib_mcu_noarch/rs4xx/rs4xx.h"
#include "lib/usart/usart_march.h"


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
void rs4xxSend(usart_cfg_st *usart, const BYTE* pc_str, const UINT16 ui16_length)
{
	UINT16 ui16_computed_length;

	// compute max length of data to send
	if (!ui16_length)
		ui16_computed_length = USART_SEND_MAX_LENGTH;
	else
		ui16_computed_length = (ui16_length > USART_SEND_MAX_LENGTH)?(USART_SEND_MAX_LENGTH):(ui16_length);

	// start sending data
	for (UINT16 i = 0; i < ui16_computed_length; ++i)
		rs4xxSendByte(usart, *(pc_str + i));
}

// --------------------------------------------------------------------------
void rs4xxSendByte(usart_cfg_st *usart, const BYTE c_byte)
{
	RS4XX_TX_ENA();
	usartSendByte(usart, c_byte);
	WAIT_FOR(USART_GetFlagStatus((USART_TypeDef*)usart->usart_if, USART_FLAG_TC) == RESET);
	RS4XX_TX_DIS();
}

// --------------------------------------------------------------------------
void rs4xxSendString(usart_cfg_st *usart, const BYTE* pc_str, const UINT16 ui16_length)
{
	UINT16 ui16_computed_length;

	// compute max length of data to send
	if (!ui16_length)
		ui16_computed_length = USART_SEND_MAX_LENGTH;
	else
		ui16_computed_length = (ui16_length > USART_SEND_MAX_LENGTH)?(USART_SEND_MAX_LENGTH):(ui16_length);

	// start sending data
	for (UINT16 i = 0; (i < ui16_computed_length) && *(pc_str + i); ++i)
		rs4xxSendByte(usart, *(pc_str + i));
}

// END
