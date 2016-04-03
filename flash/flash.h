/*!
 * \file flash.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 26.05.2014
 * \version 4
 *
 * \brief Internal flash read/write operations - definitions.
 * \details
 * Operations on internal MCU flash. Allows to read/write flash.
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in flash_march.c.
 */

#ifndef _FLASH_H
#define _FLASH_H

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"
#include "lib_func_attr.h"

/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn flashRead(BYTE *pc_flash_ptr, const BYTE *pc_data_ptr, const UINT16 ui16_size)
 * \brief read data from flash to buffer
 * \param pc_flash_ptr pointer to flash data
 * \param pc_data_ptr pointer to memory data
 * \param ui32_size size of data
 * \return pointer to read data (pc_flash_ptr)
 */
FLASH_FLASHREAD_ATTR BYTE* flashRead(BYTE *pc_flash_ptr, BYTE *pc_data_ptr, const UINT16 ui16_size);
/*!
 * \fn flashWrite(const VOID *pv_flash_ptr, const BYTE *pc_data_ptr, const UINT16 ui16_size)
 * \brief write data from buffer to flash
 * \param pv_flash_ptr pointer to flash data
 * \param pc_data_ptr pointer to memory data
 * \param ui32_size size of data
 * \return true if data was written successfully, false otherwise
 */
FLASH_FLASHWRITE_ATTR BOOL flashWrite(VOID *pv_flash_ptr, const BYTE *pc_data_ptr, const UINT16 ui16_size);

#endif // _FLASH_H
// END
