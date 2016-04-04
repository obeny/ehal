/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 * TODO:
 *  - docs
 *  - irq handling
 *  - dynamic payloads
 *  - getters
 *  - handling fifo full (fast writes)
 *  - tx payload reuse
 *  - carrier testing
 */

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include <stdio.h>

#include "config.h"

#include "ehal/nrf24l01/nrf24l01.h"
#include "ehal/spi/spi.h"
#include "ehal/sync_timer/sync_timer.h"
#include "ehal/util/util.h"
#include "lib/spi/spi_march.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

#define NRF24_STARTUP_REG_READ_CNT 10
#define NRF24_WAKEUP_TIME_US 130
#define NRF24_REG_SIZE_MAX NRF24_ADDR_SIZE_MAX

// debug helpers
#ifdef NRF24_DEBUG
#define nrf24_debug(...) \
{ \
 	printf("NRF24 %s(): ", __FUNCTION__); \
	printf(__VA_ARGS__); \
}
#else
#define nrf24_debug(...)
#endif // NRF24_DEBUG

typedef BOOL (*setmode_fnc_t) (void);

static spi_cfg_st *spi;
static BOOL b_is_p_variant;
static BOOL b_dynamic_payloads;
static UINT8 addr_width;
static e_nrf24_mode_t curr_mode;

// static functions
static void nrf24_readRegister(UINT8 reg, BYTE *data, UINT8 len);
static void nrf24_readByteRegister(UINT8 reg, BYTE *data);

static void nrf24_writeRegister(UINT8 reg, BYTE *data, UINT8 len);
static BOOL nrf24_writeByteRegister(UINT8 reg, BYTE *data);
static void nrf24_configureRegisterBit(UINT8 reg, UINT8 pos, BOOL val);

static void nrf24_enableFeatures(void);

static void nrf24_resetStatus(BOOL rxDataReady, BOOL dataSent, BOOL maxRetransmit);

static BOOL nrf24_modePowerDown(void);
static BOOL nrf24_modeStandBy(void);
static BOOL nrf24_modeRx(void);
static BOOL nrf24_modeTx(void);

static setmode_fnc_t mode_func_array[] = {
	nrf24_modePowerDown,
	nrf24_modeStandBy,
	nrf24_modeTx,
	nrf24_modeRx
};

static UINT8 nrf24_getRxPayloadLength(void);
static UINT8 nrf24_readPayload(BYTE* buffer, UINT8 len);
static UINT8 nrf24_writePayload(BYTE* buffer, UINT8 len, BOOL ack);

// debug helpers
#ifdef NRF24_DEBUG
static const char* nrf24_debugGetDataRateStr(e_nrf24_drate_t drate);
static const char* nrf24_debugGetModeStr(e_nrf24_mode_t mode);
static const char* nrf24_debugGetPowerLevelStr(e_nrf24_pl_t power);

static void nrf24_debugDumpRegister(const char* name, UINT8 reg, UINT8 size, UINT8 count);
#endif // NRF24_DEBUG


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
BOOL nrf24Init(const spi_cfg_st *spi_st)
{
	u_nrf24_reg_t reg_val;
	UINT8 i;

	spi = (spi_cfg_st*)spi_st;
	curr_mode = NRF24_MODE_POWERDOWN;
	b_dynamic_payloads = false;
	b_is_p_variant = false;

	NRF24_CSN_HIGH();
	NRF24_CE_LOW();
	delayMs(100);

	// initialize SPI and do some dummy reads to allow chip settle down
	spiMasterInit(spi, SPI_CLK_POL_POS, SPI_CLK_PHA_SAMPLE, SPI_CLK_ORD_MSB);
	for (i = 0; i < NRF24_STARTUP_REG_READ_CNT; ++i)
		nrf24_readByteRegister(NRF24_REG_CONFIG, &reg_val.byte);

	reg_val.byte = 0;
	nrf24_writeByteRegister(NRF24_REG_CONFIG, &reg_val.byte);

	// check communication
	reg_val.CONFIG.EN_CRC = 1;
	if (!nrf24_writeByteRegister(NRF24_REG_CONFIG, &reg_val.byte))
	{
		nrf24_debug("communication problem, please check wiring\n");
		return (false);
	}
	else
	{
		nrf24_debug("communication ok\n");
	}

	// check whether we have nrf24l01+ variant
	reg_val.byte = 0;
	reg_val.RF_SETUP_PLUS.RF_DR_LOW = 1;
	if (nrf24_writeByteRegister(NRF24_REG_RF_SETUP, &reg_val.byte))
	{
		nrf24_debug("detected nRF24L01+ chip\n");
		b_is_p_variant = true;
	}
	else
	{
		nrf24_debug("detected nRF24L01 chip\n");
	}

	//
	// SETUP PHASE
	//

	// disable chip by default
	if (!nrf24SetMode(NRF24_MODE_POWERDOWN))
		return (false);
	// set 2 byte CRC by default
	if (!nrf24SetCrcLength(NRF24_CRC_16))
		return (false);
	// set data rate to 1Mbps (most reliable and supported by all chips)
	if (!nrf24SetDataRate(NRF24_DRATE_1M))
		return (false);
	// leave tx power at low level
	if (!nrf24SetPowerLevel(NRF24_PL_LOW))
		return (false);
	// set default address width to 5 bytes
	if (!nrf24SetAddressWidth(NRF24_ADRWIDTH_5))
		return (false);
	// set default channel in the middle of spectrum
	if (!nrf24SetChannel(64))
		return (false);
	// set maximum delay (1500us) for retransmission (15 retries) to make testing easier
	if (!nrf24SetAutoRetransmit(1500, 15))
		return (false);

	// disable auto acknowledgement on all pipes
	reg_val.byte = 0;
	nrf24_writeByteRegister(NRF24_REG_EN_AA, &reg_val.byte);

	// default receive pipe addresses
	nrf24_writeRegister(NRF24_REG_RX_ADDR_P0, (BYTE*)"00000", 5);
	nrf24_writeRegister(NRF24_REG_RX_ADDR_P1, (BYTE*)"11111", 5);
	nrf24_writeRegister(NRF24_REG_RX_ADDR_P2, (BYTE*)"2", 1);
	nrf24_writeRegister(NRF24_REG_RX_ADDR_P3, (BYTE*)"3", 1);
	nrf24_writeRegister(NRF24_REG_RX_ADDR_P4, (BYTE*)"4", 1);
	nrf24_writeRegister(NRF24_REG_RX_ADDR_P5, (BYTE*)"5", 1);
	
	// default transmitter address
	nrf24_writeRegister(NRF24_REG_TX_ADDR, (BYTE*)"00000", 5);

	// disable all pipes
	nrf24_writeByteRegister(NRF24_REG_EN_RXADDR, &reg_val.byte);

	// disable all receive pipes (no payload expected)
	nrf24_writeByteRegister(NRF24_REG_RX_PW_P0, &reg_val.byte);
	nrf24_writeByteRegister(NRF24_REG_RX_PW_P1, &reg_val.byte);
	nrf24_writeByteRegister(NRF24_REG_RX_PW_P2, &reg_val.byte);
	nrf24_writeByteRegister(NRF24_REG_RX_PW_P3, &reg_val.byte);
	nrf24_writeByteRegister(NRF24_REG_RX_PW_P4, &reg_val.byte);
	nrf24_writeByteRegister(NRF24_REG_RX_PW_P5, &reg_val.byte);

	// activate disabled registers
	nrf24_enableFeatures();

	// disable dynamic payloads
	reg_val.byte = 0;
	if (!nrf24_writeByteRegister(NRF24_REG_DYNPD, &reg_val.byte))
		return (false);
	if (!nrf24_writeByteRegister(NRF24_REG_FEATURE, &reg_val.byte))
		return (false);

	// reset status
	nrf24_resetStatus(true, true, true);

	// flush tx and rx fifos
	nrf24FlushRx();
	nrf24FlushTx();

	nrf24_debug("init ok\n");

	return (true);
}

// --------------------------------------------------------------------------
BOOL nrf24SetMode(e_nrf24_mode_t mode)
{
	BOOL result = true;
	
	if (mode != curr_mode)
		result = mode_func_array[mode]();

	if (result)
	{
		nrf24_debug("set mode=%s\n", nrf24_debugGetModeStr(mode));
	}
	else
	{
		nrf24_debug("mode set failed\n");
	}
	return (result);
}

// --------------------------------------------------------------------------
UINT8 nrf24GetAddressWidth(void)
{
	return (addr_width);
}

// --------------------------------------------------------------------------
BOOL nrf24SetAddressWidth(e_nrf24_adrwidth_t width)
{
	BOOL result;
	u_nrf24_reg_t reg_val;

	reg_val.byte = 0;
	reg_val.SETUP_AW.AW = width;
	addr_width = 2 + width;

	result = nrf24_writeByteRegister(NRF24_REG_SETUP_AW, &reg_val.byte);
	if (result)
	{
		nrf24_debug("set address_width=%d\n", addr_width);
	}
	else
	{
		nrf24_debug("address_width set failed\n");
	}
	return (result);
}

// --------------------------------------------------------------------------
BOOL nrf24SetAutoRetransmit(UINT16 delay_us, UINT8 count)
{
	BOOL result;
	u_nrf24_reg_t reg_val;

	if (delay_us > 4000)
		delay_us = 4000;
	if (delay_us < 250)
		delay_us = 250;

	reg_val.SETUP_RETR.ARD = ((delay_us / 250) - 1) & 0xF;
	reg_val.SETUP_RETR.ARC = count & 0xF;

	result = nrf24_writeByteRegister(NRF24_REG_SETUP_RETR, &reg_val.byte);
	if (result)
	{
		nrf24_debug("set auto_retransmit: delay=%dus, count=%d\n", delay_us, count);
	}
	else
	{
		nrf24_debug("auto_transmit set failed\n");
	}
	return (result);
}

// --------------------------------------------------------------------------
BOOL nrf24SetChannel(UINT8 channel)
{
	BOOL result;
	u_nrf24_reg_t reg_val;

	reg_val.byte = 0;
	if (channel > NRF24_CHANNEL_MAX)
		channel = NRF24_CHANNEL_MAX;
	reg_val.RF_CH.RF_CH = channel;

	result = nrf24_writeByteRegister(NRF24_REG_RF_CH, &reg_val.byte);
	if (result)
	{
		nrf24_debug("set channel=%d\n", channel);
	}
	else
	{
		nrf24_debug("channel set failed\n");
	}
	return (result);
}

// --------------------------------------------------------------------------
BOOL nrf24SetCrcLength(e_nrf24_crc_t crc)
{
	BOOL result;
	NRF24_CONFIG_t config;

	nrf24_readByteRegister(NRF24_REG_CONFIG, (BYTE*)&config);
	config.EN_CRC = 1;
	if (NRF24_CRC_DISABLED == crc)
	{
		config.EN_CRC = 0;
		config.CRCO = 0;
	}
	else if (NRF24_CRC_8 == crc)
	{
		config.CRCO = 0;
	}
	else if (NRF24_CRC_16 == crc)
	{
		config.CRCO = 1;
	}

	result = nrf24_writeByteRegister(NRF24_REG_CONFIG, (BYTE*)&config);
	if (result)
	{
		nrf24_debug("set crc_length=%d\n", crc);
	}
	else
	{
		nrf24_debug("crc_length set failed\n");
	}
	return (result);
}

// --------------------------------------------------------------------------
BOOL nrf24SetDataRate(e_nrf24_drate_t drate)
{
	BOOL result = true;
	u_nrf24_reg_t rf_setup;

	nrf24_readByteRegister(NRF24_REG_RF_SETUP, &rf_setup.byte);
	if (b_is_p_variant)
		rf_setup.RF_SETUP_PLUS.RF_DR_LOW = 0;

	if (NRF24_DRATE_2M == drate)
		rf_setup.RF_SETUP.RF_DR = 1;
	else if (NRF24_DRATE_1M == drate)
		rf_setup.RF_SETUP.RF_DR = 0;
	else if (NRF24_DRATE_250K == drate)
	{
		if (b_is_p_variant)
			rf_setup.RF_SETUP_PLUS.RF_DR_LOW = 1;
		else
			result = false;
	}

	if (!result)
	{
		nrf24_debug("not supported on nRF24L01 chip\n");
	}
	else
	{
		result = nrf24_writeByteRegister(NRF24_REG_RF_SETUP, &rf_setup.byte);
		if (result)
		{
			nrf24_debug("set data_rate=%s\n", nrf24_debugGetDataRateStr(drate));
		}
		else
		{
			nrf24_debug("data_rate set failed\n");
		}
	}
	return (result);
}

// --------------------------------------------------------------------------
BOOL nrf24SetPowerLevel(e_nrf24_pl_t power)
{
	BOOL result;
	NRF24_RF_SETUP_t rf_setup;

	nrf24_readByteRegister(NRF24_REG_RF_SETUP, (BYTE*)&rf_setup);
	rf_setup.RF_PWR = power;
	
	result = nrf24_writeByteRegister(NRF24_REG_RF_SETUP, (BYTE*)&rf_setup);
	if (result)
	{
		nrf24_debug("set power_level=%s\n", nrf24_debugGetPowerLevelStr(power));
	}
	else
	{
		nrf24_debug("power_level set failed\n");
	}
	return (result);
}

// --------------------------------------------------------------------------
void nrf24FlushRx(void)
{
	NRF24_CSN_LOW();
	spiSendByte(spi, NRF24_CMD_FLUSH_RX);
	NRF24_CSN_HIGH();
}

// --------------------------------------------------------------------------
void nrf24FlushTx(void)
{
	NRF24_CSN_LOW();
	spiSendByte(spi, NRF24_CMD_FLUSH_TX);
	NRF24_CSN_HIGH();
}

// --------------------------------------------------------------------------
NRF24_STATUS_t nrf24GetStatus(void)
{
	u_nrf24_reg_t status;

	NRF24_CSN_LOW();
	status.byte = spiTransferByte(spi, NRF24_CMD_NOP);
	NRF24_CSN_HIGH();
	return (status.STATUS);
}

// --------------------------------------------------------------------------
NRF24_FIFO_STATUS_t nrf24GetFifoStatus(void)
{
	NRF24_FIFO_STATUS_t fifo_status;
	
	nrf24_readByteRegister(NRF24_REG_FIFO_STATUS, (BYTE*)&fifo_status);
	return (fifo_status);
}

// --------------------------------------------------------------------------
BOOL nrf24UnreadData(UINT8 *pipe_num)
{
	NRF24_FIFO_STATUS_t fifo_status = nrf24GetFifoStatus();
	NRF24_STATUS_t status;

	if (!fifo_status.RX_EMPTY)
	{
		if (pipe_num)
		{
			status = nrf24GetStatus();
			*pipe_num = status.RX_P_NO;
		}
		return (true);
	}
	return (false);
}

// --------------------------------------------------------------------------
BOOL nrf24UnsentData(void)
{
	NRF24_FIFO_STATUS_t fifo_status = nrf24GetFifoStatus();
	return (!fifo_status.TX_EMPTY);
}

// --------------------------------------------------------------------------
void nrf24SetRxPipeAddress(UINT8 pipe_num, BYTE *address, BOOL enable)
{
	if (pipe_num > NRF24_PIPE_COUNT_MAX)
		return;

	if (pipe_num < 2)
		nrf24_writeRegister(NRF24_REG_RX_ADDR_P0 + pipe_num, (BYTE*)address, addr_width);
	else
		nrf24_writeRegister(NRF24_REG_RX_ADDR_P0 + pipe_num, (BYTE*)address, 1);

	nrf24SetRxPipeEnabled(pipe_num, enable);
}

// --------------------------------------------------------------------------
void nrf24SetTxPipeAddress(BYTE *address, BOOL ack)
{
	nrf24_writeRegister(NRF24_REG_TX_ADDR, address, addr_width);
	// configure pipe 0 with same address as TX to allow ack and data reception
	if (ack)
		nrf24_writeRegister(NRF24_REG_RX_ADDR_P0, address, addr_width);
}

// --------------------------------------------------------------------------
void nrf24SetPipePayloadSize(UINT8 pipe_num, UINT8 size)
{
	if (pipe_num > NRF24_PIPE_COUNT_MAX)
		return;

	if (size > NRF24_PAYLOAD_SIZE_MAX)
		size = NRF24_PAYLOAD_SIZE_MAX;
	nrf24_writeByteRegister(NRF24_REG_RX_PW_P0 + pipe_num, (BYTE*)&size);
}

// --------------------------------------------------------------------------
void nrf24SetRxPipeEnabled(UINT8 pipe_num, BOOL enable)
{
	if (pipe_num > NRF24_PIPE_COUNT_MAX)
		return;

	nrf24_configureRegisterBit(NRF24_REG_EN_RXADDR, pipe_num, enable);
}

// --------------------------------------------------------------------------
void nrf24SetRxPipeAutoAck(UINT8 pipe_num, BOOL enable)
{
	if (pipe_num > NRF24_PIPE_COUNT_MAX)
		return;

	nrf24_configureRegisterBit(NRF24_REG_EN_AA, pipe_num, enable);
}

// --------------------------------------------------------------------------
UINT8 nrf24Receive(BYTE *buffer, UINT8 len, UINT8 *pipe_num, UINT16 timeout_ms)
{
	UINT32 end_time = jiffies + timeout_ms;
	UINT8 received_len = 0;

	nrf24SetMode(NRF24_MODE_RX);

	do
	{
		if (nrf24UnreadData(pipe_num))
		{
			received_len = nrf24_readPayload(buffer, len);
			nrf24_resetStatus(true, true, true);
			break;
		}
	}
	while (jiffies < end_time);

	return (received_len);
}

// --------------------------------------------------------------------------
UINT8 nrf24Send(BYTE* buffer, UINT8 len, BOOL ack, BOOL listen)
{
	NRF24_STATUS_t status;

	if (ack)
		nrf24SetRxPipeEnabled(0, true);
	nrf24SetMode(NRF24_MODE_TX);
	len = nrf24_writePayload(buffer, len, ack);

	NRF24_CE_HIGH();
	while (1)
	{
		status = nrf24GetStatus();
		if (status.TX_DS)
			break;

		if (ack)
		{
			if (status.MAX_RT)
			{
				len = 0;
				nrf24FlushTx();
				break;
			}
		}
	}
	NRF24_CE_LOW();

	nrf24_resetStatus(true, true, true);
	nrf24SetRxPipeEnabled(0, false);
	if (listen)
		nrf24SetMode(NRF24_MODE_RX);

	return (len);
}

#ifdef NRF24_DEBUG
// debug helpers
// --------------------------------------------------------------------------
void nrf24DumpRegisters(void)
{
	printf("NRF24 pipe config:\n");
	nrf24_debugDumpRegister(" RX_ADDR_P0-1", NRF24_REG_RX_ADDR_P0, 5, 2);
	nrf24_debugDumpRegister(" RX_ADDR_P2-5", NRF24_REG_RX_ADDR_P2, 1, 4);
	nrf24_debugDumpRegister(" TX_ADDR", NRF24_REG_TX_ADDR, 5, 1);
	nrf24_debugDumpRegister(" RX_PW_P0-6", NRF24_REG_RX_PW_P0, 1, 6);
	nrf24_debugDumpRegister(" EN_RXADDR", NRF24_REG_EN_RXADDR, 1, 1);

	printf("NRF24 transmission config:\n");
	nrf24_debugDumpRegister(" CONFIG\t", NRF24_REG_CONFIG, 1, 1);
	nrf24_debugDumpRegister(" EN_AA\t", NRF24_REG_EN_AA, 1, 1);
	nrf24_debugDumpRegister(" SETUP_AW", NRF24_REG_SETUP_AW, 1, 1);
	nrf24_debugDumpRegister(" SETUP_RETR", NRF24_REG_SETUP_RETR, 1, 1);
	nrf24_debugDumpRegister(" RF_CH\t", NRF24_REG_RF_CH, 1, 1);
	nrf24_debugDumpRegister(" RF_SETUP", NRF24_REG_RF_SETUP, 1, 1);
	nrf24_debugDumpRegister(" OBSERVE_TX", NRF24_REG_OBSERVE_TX, 1, 1);
	nrf24_debugDumpRegister(" DYNPD/FEATURE", NRF24_REG_DYNPD, 1, 2);

	printf("NRF24 status:\n");
	nrf24_debugDumpRegister(" STATUS", NRF24_REG_STATUS, 1, 1);
	nrf24_debugDumpRegister(" FIFO_STATUS", NRF24_REG_FIFO_STATUS, 1, 1);
}
#endif // NRF24_DEBUG

// static functions
// --------------------------------------------------------------------------
static void nrf24_readRegister(UINT8 reg, BYTE *data, UINT8 len)
{
	NRF24_CSN_LOW();
	spiSendByte(spi, NRF24_CMD_R_REGISTER | (reg & NRF24_REG_MASK));
	while (len > 0)
	{
		*data++ = spiTransferByte(spi, NRF24_CMD_NOP);
		--len;
	}
	NRF24_CSN_HIGH();
}

// --------------------------------------------------------------------------
static void nrf24_readByteRegister(UINT8 reg, BYTE *data)
{
	nrf24_readRegister(reg, data, 1);
}

// --------------------------------------------------------------------------
static void nrf24_writeRegister(UINT8 reg, BYTE *data, UINT8 len)
{
	NRF24_CSN_LOW();
	spiSendByte(spi, NRF24_CMD_W_REGISTER | (reg & NRF24_REG_MASK));
	while (len > 0)
	{
		spiSendByte(spi, *data++);
		--len;
	}
	NRF24_CSN_HIGH();
}

// --------------------------------------------------------------------------
static BOOL nrf24_writeByteRegister(UINT8 reg, BYTE *data)
{
	BYTE read_val;

	nrf24_writeRegister(reg, data, 1);
	nrf24_readByteRegister(reg, &read_val);

	return (read_val == *data);
}

// --------------------------------------------------------------------------
static void nrf24_configureRegisterBit(UINT8 reg, UINT8 pos, BOOL val)
{
	BYTE reg_val;

	nrf24_readByteRegister(reg, &reg_val);
	if (val)
		sbi(reg_val, pos);
	else
		cbi(reg_val, pos);
	nrf24_writeByteRegister(reg, &reg_val);
}

// --------------------------------------------------------------------------
static void nrf24_enableFeatures(void)
{
	NRF24_CSN_LOW();
	spiSendByte(spi, NRF24_CMD_ACTIVATE);
    spiSendByte(spi, 0x73);
	NRF24_CSN_HIGH();
}

// --------------------------------------------------------------------------
static void nrf24_resetStatus(BOOL rxDataReady, BOOL dataSent, BOOL maxRetransmit)
{
	NRF24_STATUS_t status;

	nrf24_readByteRegister(NRF24_REG_STATUS, (BYTE*)&status);
	status.MAX_RT = maxRetransmit;
	status.TX_DS = dataSent;
	status.RX_DR = rxDataReady;
	nrf24_writeByteRegister(NRF24_REG_STATUS, (BYTE*)&status);
}

// --------------------------------------------------------------------------
static BOOL nrf24_modePowerDown(void)
{
	NRF24_CONFIG_t config;

	NRF24_CE_LOW();
	nrf24_readByteRegister(NRF24_REG_CONFIG, (BYTE*)&config);
	config.PWR_UP = 0;
	if (nrf24_writeByteRegister(NRF24_REG_CONFIG, (BYTE*)&config))
	{
		curr_mode = NRF24_MODE_POWERDOWN;
		return (true);
	}
	return (false);
}

// --------------------------------------------------------------------------
static BOOL nrf24_modeStandBy(void)
{
	NRF24_CONFIG_t config;

	NRF24_CE_LOW();
	nrf24_readByteRegister(NRF24_REG_CONFIG, (BYTE*)&config);
	if (!config.PWR_UP)
	{
		config.PWR_UP = 1;
		if (nrf24_writeByteRegister(NRF24_REG_CONFIG, (BYTE*)&config))
		{
			nrf24_resetStatus(true, true, true);
			nrf24FlushRx();
			nrf24FlushTx();
			curr_mode = NRF24_MODE_STANDBY;
			return (true);
		}
		return (false);
	}

	curr_mode = NRF24_MODE_STANDBY;
	return (true);
}

// --------------------------------------------------------------------------
static BOOL nrf24_modeRx(void)
{
	NRF24_CONFIG_t config;

	if (curr_mode < NRF24_MODE_STANDBY)
	{
		if (!nrf24_modeStandBy())
			return (false);
	}

	nrf24_readByteRegister(NRF24_REG_CONFIG, (BYTE*)&config);
	nrf24_resetStatus(true, true, true);
	NRF24_CE_HIGH();

	if (!config.PRIM_RX)
	{
		config.PRIM_RX = 1;
		if (!nrf24_writeByteRegister(NRF24_REG_CONFIG, (BYTE*)&config))
			return (false);
		delayUs(NRF24_WAKEUP_TIME_US);
	}

	curr_mode = NRF24_MODE_RX;
	return (true);
}

// --------------------------------------------------------------------------
static BOOL nrf24_modeTx(void)
{
	NRF24_CONFIG_t config;

	nrf24_resetStatus(true, true, true);
	if (curr_mode < NRF24_MODE_STANDBY)
	{
		if (!nrf24_modeStandBy())
			return (false);
	}

	nrf24_readByteRegister(NRF24_REG_CONFIG, (BYTE*)&config);
	if (config.PRIM_RX)
	{
		config.PRIM_RX = 0;
		if (!nrf24_writeByteRegister(NRF24_REG_CONFIG, (BYTE*)&config))
			return (false);
		delayUs(NRF24_WAKEUP_TIME_US);
	}

	NRF24_CE_LOW();

	curr_mode = NRF24_MODE_TX;
	return (true);
}

// --------------------------------------------------------------------------
static UINT8 nrf24_getRxPayloadLength(void)
{
	UINT8 len;

	NRF24_CSN_LOW();
	spiSendByte(spi, NRF24_CMD_R_RX_PL_WID);
	len = spiTransferByte(spi, NRF24_CMD_NOP);
	NRF24_CSN_HIGH();

	return (len);
}

// --------------------------------------------------------------------------
static UINT8 nrf24_readPayload(BYTE* buffer, UINT8 len)
{
	UINT8 payload_size = nrf24_getRxPayloadLength();
	UINT8 fill;
	UINT8 tmp_len = len;

	if (len > payload_size)
		len = payload_size;
	fill = payload_size - len;

	NRF24_CSN_LOW();
	spiSendByte(spi, NRF24_CMD_R_RX_PAYLOAD);
	while (tmp_len--)
		*buffer++ = spiTransferByte(spi, NRF24_CMD_NOP);
	while (fill--)
		spiSendByte(spi, NRF24_CMD_NOP);
	NRF24_CSN_HIGH();

	return (len);
}

// --------------------------------------------------------------------------
static UINT8 nrf24_writePayload(BYTE* buffer, UINT8 len, BOOL ack)
{
	UINT8 fill;
	UINT8 tmp_len = len;

	if (len > NRF24_PAYLOAD_SIZE_MAX)
		len = NRF24_PAYLOAD_SIZE_MAX;
	fill = NRF24_PAYLOAD_SIZE_MAX - len;

	NRF24_CSN_LOW();
	spiSendByte(spi, (ack)?(NRF24_CMD_W_TX_PAYLOAD):(NRF24_CMD_W_TX_PAYLOAD_NOACK));
	while (tmp_len--)
		spiSendByte(spi, *buffer++);
	while (fill--)
		spiSendByte(spi, 0x00);
	NRF24_CSN_HIGH();

	return (len);
}

#ifdef NRF24_DEBUG
// debug helpers
// --------------------------------------------------------------------------
static const char* nrf24_debugGetDataRateStr(e_nrf24_drate_t drate)
{
	switch (drate)
	{
		case NRF24_DRATE_1M:
			return ("drate_1M");
		case NRF24_DRATE_2M:
			return ("drate_2M");
		case NRF24_DRATE_250K:
			return ("drate_250k");
	}
	return ("drate_UNKNOWN");
}

// --------------------------------------------------------------------------
static const char* nrf24_debugGetModeStr(e_nrf24_mode_t mode)
{
	switch (mode)
	{
		case NRF24_MODE_POWERDOWN:
			return ("mode_POWERDOWN");
		case NRF24_MODE_STANDBY:
			return ("mode_STANDBY");
		case NRF24_MODE_TX:
			return ("mode_TX");
		case NRF24_MODE_RX:
			return ("mode_RX");
	}
	return ("mode_UNKNOWN");
}

// --------------------------------------------------------------------------
static const char* nrf24_debugGetPowerLevelStr(e_nrf24_pl_t power)
{
	switch (power)
	{
		case NRF24_PL_MIN:
			return ("power_MIN");
		case NRF24_PL_LOW:
			return ("power_LOW");
		case NRF24_PL_HIGH:
			return ("power_HIGH");
		case NRF24_PL_MAX:
			return ("power_MAX");
	}
	return ("power_UNKNOWN");
}

// --------------------------------------------------------------------------
static void nrf24_debugDumpRegister(const char* name, UINT8 reg, UINT8 size, UINT8 count)
{
	BYTE buffer[NRF24_REG_SIZE_MAX];
	UINT8 i;

	printf("%s\t =", name);
	while (count--)
	{
		printf(" 0x");
		nrf24_readRegister(reg, buffer, size);
		for(i = 0; i < size; ++i)
			printf("%02X", *(buffer + i));
		++reg;
	}
	printf("\n");
}

#endif // NRF24_DEBUG

// END
