/*!
 * \file util.h
 *
 * \author Marcin O'BenY Benka <obeny@obeny.net>
 * \date 04.01.2014
 * \version 1
 *
 * \brief Utilities package - definitions
 * \warning
 * Depending on MCU architecture additional configuration definitions may be required.
 * Implementation for particular architecture is contained in related version of library in util_march.c.
 */

#ifndef _UTIL_H
#define _UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"
#include "lib_func_attr.h"


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

/*!
 * \fn checkStack(UINT16 range)
 * \brief checks whether stack area was not exhausted
 * \param range stack area to check, 0 for checking whether all stack area was used
 * \return true if stack area overflow occured, false otherwise
 */
BOOL checkStack(UINT16 range);

/*!
 * \fn delayUs(void)
 * \brief performs active waiting for given amount of microseconds
 * \param us time to wait
 */
void __naked delayUs(volatile UINT32 us);

/*!
 * \fn delayMs(void)
 * \brief performs active waiting for given amount of microseconds
 * \param ms time to wait
 */

void delayMs(UINT16 ms);

/*!
 * \fn softReset(void)
 * \brief performs soft reset of MCU (no return)
 */
__no_return UTIL_SOFTRESET_ATTR void softReset(void);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _UTIL_H

// END
