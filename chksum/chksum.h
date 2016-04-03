/*!
 * \file common.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 08.02.2014
 * \version 13
 *
 * \brief Simple byte adding checksum functions - implementation
 */

#ifndef _CHKSUM_H
#define _CHKSUM_H

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"
#include "lib_func_attr.h"


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn checksum8Bit(BYTE* pc_ptr, const UINT16 ui16_length)
 * \brief calculate 8-bit checksum of buffer that length is lower than 16bit uint range
 * \param pc_ptr buffer pointer
 * \param ui16_length number of bytes of buffer to process
 * \return byte of checksum (added bytes with no carry)
 */
CHKSUM_CHECKSUM8BIT_ATTR BYTE checksum8Bit(BYTE* pc_ptr, const UINT16 ui16_length);
/*!
 * \fn checksum16Bit(BYTE* pc_ptr, const UINT32 ui32_length)
 * \brief calculate 16-bit checksum of buffer that length is lower than 32bit uint range
 * \param pc_ptr buffer pointer
 * \param ui32_length number of bytes of buffer to process
 * \return byte of checksum (added bytes with no carry)
 */
CHKSUM_CHECKSUM16BIT_ATTR UINT16 checksum16Bit(BYTE* pc_ptr, const UINT32 ui32_length);

#endif // _CHKSUM_H

// END
