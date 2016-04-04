/*!
 * \file nvm_settings.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 02.01.2014
 * \version 12
 *
 * \brief NVM based settings operations - definitions
 * \details
 * Non volatile space is utilized to store configuration data. It is crucial to locate array used for storing data
 * in memory section in non-volatile memory. Use appropriate attributes to achieve it.
 */

#ifndef _NVM_SETTINGS_H
#define _NVM_SETTINGS_H

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "ehal/global.h"


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn nvmReadSettings(const BYTE *pc_nvm_ptr, const BYTE *pc_mem_ptr, const UINT8 ui8_size, const BOOL b_chksum)
 * \brief read configuration data from nvm
 * \param pc_nvm_ptr pointer to nvm data
 * \param pc_mem_ptr pointer to memory data
 * \param ui8_size size of data
 * \param b_chksum indicates whether to check data checksum
 * \return true if configuration data read successfully, false otherwise
 * \note In case checksum checking is enabled, data size have to be incremented by one.
 */
BOOL nvmReadSettings(const BYTE *pc_nvm_ptr, BYTE *pc_mem_ptr, const UINT8 ui8_size, const BOOL b_chksum);
/*!
 * \fn nvmWriteSettings(const BYTE *pc_nvm_ptr, const BYTE *pc_mem_ptr, const UINT8 ui8_size)
 * \brief store configuration data in nvm
 * \param pc_nvm_ptr pointer to nvm data
 * \param pc_mem_ptr pointer to memory data
 * \param ui8_size size of data
 * \param b_chksum indicates whether checksum have to be computed
 * \note In case checksum writing is enabled, data size have to be incremented by one.
 */
VOID nvmWriteSettings(BYTE *pc_nvm_ptr, BYTE *pc_mem_ptr, const UINT8 ui8_size, const BOOL b_chksum);

#endif // _NVM_SETTINGS_H

// END
