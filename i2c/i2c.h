#ifndef _I2C_H
#define _I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "config.h"
#include "ehal/global.h"
#include "lib/i2c/i2c_march.h"

/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

void i2cMasterInit(const i2c_cfg_st *i2c, const BOOL ack);
BOOL i2cMasterTransfer(const i2c_cfg_st *i2c, const BOOL repeated_start, const UINT8 addr, const char* send_buf,
	const UINT8 send_len, char *recv_buf, const UINT8 recv_len, const UINT16 timeout);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _I2C_H

// END
