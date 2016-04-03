/*!
 * \file usart.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 09.03.2016
 * \version 24
 *
 * \brief declarations used in buffered usart handling libraries
 * \details
 * Defines baudrate and parity enums.
 * 
 * This library needs to work following definitions to be set in config.h:
 * - F_CPU - cpu frequency
 * - USART_RBUF_SIZE - size of rx ring buffer
 * - USART_TBUF_SIZE - size of tx ring buffer
 * - USART_SEND_MAX_LENGTH - max length of transmitted data block
 * - USART_BIG_BUFFERS - enable support for buffers bigger than 256 bytes
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in usart_march.c.
 */

#ifndef _USART_H
#define _USART_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "config.h"
#include "lib_mcu_noarch/global.h"
#include "lib/usart/usart_march.h"
#include "lib_func_attr.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

/*!
 * \enum e_usartparity
 * \brief usart parity modes
 */
enum e_usartparity
{
	USART_PARITY_NONE = 0,
	USART_PARITY_EVEN,
	USART_PARITY_ODD
};
/*!
 * \typedef e_usartparity_t
 * \brief usart parity modes
 */
typedef enum e_usartparity e_usartparity_t;


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/
/*!
 * \fn usart1Close
 * \brief close USART and flush buffers
 */
USART_USARTCLOSE_ATTR VOID usartClose(usart_cfg_st *usart);

/*!
 * \fn usart1Flush
 * \brief flush bytes waiting in buffers (ignore them)
 */
VOID usartFlush(usart_cfg_st *usart);

/*!
 * \fn usart1Init(const UINT16 ui16_baudrate, const BOOL b_two_stop_bits, const UINT8 ui8_parity)
 * \brief initialization of USART
 * \param ui16_baudrate baudrate divided by 100 or value from e_usartbaud
 * \param b_two_stop_bits use two stop bits in frame
 * \param ui8_parity use parity checking, see e_usartparity
 */
USART_USARTINIT_ATTR VOID usartInit(usart_cfg_st *usart, const UINT16 ui16_baudrate, const BOOL b_two_stop_bits, const e_usartparity_t parity);
/*!
 * \fn usart1Read(BYTE* pc_str, const UINT16 ui16_count, UINT16 ui16_val)
 * \brief read bytes and write them to buffer
 * \param pc_str pointer to buffer, where data will be written
 * \param ui16_count count of bytes to read
 * \param ui16_val value for timeout, read note!
 * \return pointer to buffer where data was written
 * \note If USART1_USE_TIMER was defined in config.h, defines read timeout in miliseconds, otherwise it defines iteration count after
 * which waitung for byte will be interrupted.
 */
USART_USARTREAD_ATTR BOOL usartRead(usart_cfg_st *usart, BYTE* pc_str, const UINT16 ui16_count, UINT16 ui16_val);
/*!
 * \fn usart1ReadByte
 * \brief read single byte
 * \return read byte from buffer
 */
USART_USARTREADBYTE_ATTR BYTE usartReadByte(usart_cfg_st *usart);
/*!
 * \fn usart1ReceiveBufferOverflow
 * \brief tells whether receive buffer overflow occurs
 * \return true if receive buffer overflow occured, false otherwise
 */
BOOL usartReceiveBufferOverflow(const usart_cfg_st *usart);
/*!
 * \fn usart1Send(BYTE* pc_str, const UINT16 ui16_length)
 * \brief send data from buffer
 * \param pc_str data in buffer to be sent
 * \param ui16_length max length of given string (limited to USART_SEND_MAX_LENGTH)
 */
VOID usartSend(usart_cfg_st *usart, const BYTE* pc_str, const UINT16 ui16_length);
/*!
 * \fn usart1SendByte(const BYTE c_byte)
 * \brief send single byte
 * \param c_byte byte to be transmitted
 */
USART_USARTSENDBYTE_ATTR VOID usartSendByte(usart_cfg_st *usart, const BYTE c_byte);
/*!
 * \fn usart1SendString(BYTE* spc_str, const UINT16 ui16_length)
 * \brief send string
 * \param pc_str string to be sent
 * \param ui16_length max length of given string (limited to USART_SEND_MAX_LENGTH)
 */
USART_USARTSENDSTRING_ATTR VOID usartSendString(usart_cfg_st *usart, const BYTE* pc_str, const UINT16 ui16_length);
/*!
 * \fn usart1UnreadBytes
 * \brief unread bytes count
 * \return number of unread bytes in buffer
 */
USART_USARTUNREADBYTES_ATTR usart_bufsize_t usartUnreadBytes(const usart_cfg_st *usart);
/*!
 * \fn usart1UnsentBytes
 * \brief unsent bytes count
 * \return number of unsent bytes in buffer
 */
usart_bufsize_t usartUnsentBytes(const usart_cfg_st *usart);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _USART_H

// END
