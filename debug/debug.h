#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

void xxd(BYTE *buf, UINT32 len);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _DEBUG_H

// END
