/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*!
 * \file chksum.c
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 08.02.2014
 * \version 13
 *
 * \brief Simple byte adding checksum functions - implementation
 * \note
 * For detailed description see header file.
 */

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "ehal/chksum/chksum.h"
#include "ehal/global.h"


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
CHKSUM_CHECKSUM8BIT_ATTR BYTE checksum8Bit(BYTE* pc_ptr, const UINT16 ui16_length)
{
	BYTE c_chksum = 0;

	for (UINT16 i = 0; i < ui16_length; ++i)
		c_chksum += *pc_ptr++;

	return (c_chksum);
}

// --------------------------------------------------------------------------
CHKSUM_CHECKSUM16BIT_ATTR UINT16 checksum16Bit(BYTE* pc_ptr, const UINT32 ui32_length)
{
	UINT16 c_chksum = 0;

	for (UINT32 i = 0; i < ui32_length; ++i)
		c_chksum += *pc_ptr++;

	return (c_chksum);
}

// END
