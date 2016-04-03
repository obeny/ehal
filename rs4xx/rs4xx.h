/*!
 * \file rs4xx.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 11.03.2016
 * \version 1
 *
 * \brief Blocking RS422/485 interface handler - definitions.
 * \details
 * Blocking implementation of RS422/485 control handler.
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in spi_march.c.
 */

 #ifndef _RS4XX_H
#define _RS4XX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"
#include "lib_mcu_noarch/usart/usart.h"

/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

void rs4xxSend(usart_cfg_st *usart, const BYTE* pc_str, const UINT16 ui16_length);
void rs4xxSendByte(usart_cfg_st *usart, const BYTE c_byte);
void rs4xxSendString(usart_cfg_st *usart, const BYTE* pc_str, const UINT16 ui16_length);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _RS4XX_H

// END
