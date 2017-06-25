/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "config.h"

#include "ehal/si7020/si7020.h"
#include "ehal/i2c/i2c.h"
#include "lib/i2c/i2c_march.h"

/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

static i2c_cfg_st *i2c;

static const char si7020_cmd_reset[] = {SI7020_CMD_RESET};
static const char si7020_cmd_getfwver[] = {SI7020_CMD_FWVER1, SI7020_CMD_FWVER2};
static const char si7020_cmd_getrh[] = {SI7020_CMD_GET_RH_NOHOLD};
static const char si7020_cmd_gettemp[] = {SI7020_CMD_GET_PREV_TEMP};

// static functions
static UINT8 si7020_getFirmwareVersion(void);
static BOOL si7020_reset(void);

/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
BOOL si7020Init(const i2c_cfg_st *i2c_st)
{
	i2c = (i2c_cfg_st*)i2c_st;

	// initialize the I2C bus master
	i2cMasterInit(i2c, true);

	// reset the device, exit with failure if device not replied
	if (false == si7020_reset())
		return (false);
	delayMs(10);
	if (0x00 == si7020_getFirmwareVersion())
		return (false);

	return (true);
}

// --------------------------------------------------------------------------
BOOL si7020GetMeasurement(SI7020_DATA_t *data)
{
	uint8_t s[2] = {0};
	i2cMasterTransfer(i2c, true, SI7020_I2C_ADDR, si7020_cmd_getrh, 1, NULL, 0, SI7020_TIMEOUT);
	delayMs(100);
	i2cMasterTransfer(i2c, true, SI7020_I2C_ADDR, NULL, 0, &s, 2, SI7020_TIMEOUT);
	data->humidity = (1250 * (s[1] | s[0] << 8))/65536 - 60;

	i2cMasterTransfer(i2c, true, SI7020_I2C_ADDR, si7020_cmd_gettemp, 1, NULL, 0, SI7020_TIMEOUT);
	i2cMasterTransfer(i2c, true, SI7020_I2C_ADDR, NULL, 0, &s, 2, SI7020_TIMEOUT);
	data->temperature = (1750.72f * (s[1] | s[0] << 8))/65536 - 468.5f;

	return (true);
}

// static functions
// --------------------------------------------------------------------------
static UINT8 si7020_getFirmwareVersion(void)
{
	UINT8 fwver;

	if (i2cMasterTransfer(i2c, true, SI7020_I2C_ADDR, si7020_cmd_getfwver, SI7020_CMD_FWVER_SIZE, (char*)&fwver, 1, SI7020_TIMEOUT))
	{
		if (SI7020_FWVER_REG_V1 == fwver || SI7020_FWVER_REG_V2 == fwver)
			return (fwver);
	}
	return (0x00);
}

// --------------------------------------------------------------------------
static BOOL si7020_reset(void)
{
	return (i2cMasterTransfer(i2c, true, SI7020_I2C_ADDR, si7020_cmd_reset, SI7020_CMD_RESET_SIZE, NULL, 0, SI7020_TIMEOUT));
}

// END
