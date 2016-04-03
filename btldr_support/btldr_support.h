/*!
 * \file btldr_support.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 08.05.2014
 * \version 1
 *
 * \brief helpers for handling software version information - definitions.
 * \details
 * Helpers for reading writing software information processed by bootloader.
 * \note
 * This library need to set following constants in config.h to work:
 * - BTLDR_SW_VER_SIZE	- size of version information block (must match with bootloader)
 * - BTLDR_SW_VER_LOC   - address where in non-volatile memory version information will be stored
 *                        (must match with bootloader)
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in btldr_support_march.c.
 */

#ifndef _BTLDR_SUPPORT_H
#define _BTLDR_SUPPORT_H

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"

/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn btldrVersionInfoRead(BYTE *pc_data)
 * \brief read current software version visible to bootloader
 * \param pc_data pointer to buffer for data read
 * \note buffer size have to be equal or greater than BTLDR_SW_VER_SIZE
 */
VOID btldrVersionInfoRead(BYTE *pc_data);
/*!
 * \fn btldrVersionInfoWrite(const BYTE *pc_data)
 * \brief write software version visible to bootloader
 * \param pc_data pointer to buffer for string with data to write
 * \param ui8_size length of version string (strlen)
 * \return true if write was successful, false otherwise
 */
BOOL btldrVersionInfoWrite(const BYTE *pc_data, UINT8 ui8_size);

#endif // _BTLDR_SUPPORT_H
// END
