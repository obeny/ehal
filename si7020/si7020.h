#ifndef _SI7020_H
#define _SI7020_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "ehal/global.h"
#include "lib/i2c/i2c_march.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

// compiletime checks
#ifndef SI7020_TIMEOUT
	#error "SI7020: SI7020_TIMEOUT not set"
#endif

// I2C address
#define SI7020_I2C_ADDR 0x40

// commands
#define SI7020_CMD_RESET 0xFE
#define SI7020_CMD_FWVER1 0x84
#define SI7020_CMD_FWVER2 0xB8
#define SI7020_CMD_GET_RH_NOHOLD 0xF5
#define SI7020_CMD_GET_PREV_TEMP 0xE0

// register values
#define SI7020_FWVER_REG_V1 0xFF
#define SI7020_FWVER_REG_V2 0x20

// data structure
typedef struct
{
	INT16 temperature;
	INT16 humidity;
} SI7020_DATA_t;

/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

BOOL si7020Init(const i2c_cfg_st *i2c_st);

BOOL si7020GetMeasurement(SI7020_DATA_t *data);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _SI7020_H

// END
