#ifndef _NRF24L01_H
#define _NRF24L01_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "ehal/global.h"
#include "lib/spi/spi_march.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

#define NRF24_PAYLOAD_SIZE_MAX 32
#define NRF24_ADDR_SIZE_MAX    5
#define NRF24_PIPE_COUNT_MAX   5
#define NRF24_CHANNEL_MAX      126

// private
// --------------------------------------------------------------------------
// register Map
#define NRF24_REG_CONFIG      0x00
#define NRF24_REG_EN_AA       0x01
#define NRF24_REG_EN_RXADDR   0x02
#define NRF24_REG_SETUP_AW    0x03
#define NRF24_REG_SETUP_RETR  0x04
#define NRF24_REG_RF_CH       0x05
#define NRF24_REG_RF_SETUP    0x06
#define NRF24_REG_STATUS      0x07
#define NRF24_REG_OBSERVE_TX  0x08
#define NRF24_REG_CD          0x09
#define NRF24_REG_RX_ADDR_P0  0x0A
#define NRF24_REG_RX_ADDR_P1  0x0B
#define NRF24_REG_RX_ADDR_P2  0x0C
#define NRF24_REG_RX_ADDR_P3  0x0D
#define NRF24_REG_RX_ADDR_P4  0x0E
#define NRF24_REG_RX_ADDR_P5  0x0F
#define NRF24_REG_TX_ADDR     0x10
#define NRF24_REG_RX_PW_P0    0x11
#define NRF24_REG_RX_PW_P1    0x12
#define NRF24_REG_RX_PW_P2    0x13
#define NRF24_REG_RX_PW_P3    0x14
#define NRF24_REG_RX_PW_P4    0x15
#define NRF24_REG_RX_PW_P5    0x16
#define NRF24_REG_FIFO_STATUS 0x17
#define NRF24_REG_DYNPD       0x1C
#define NRF24_REG_FEATURE     0x1D

#define NRF24_REG_MASK        0x1F

// commands
#define NRF24_CMD_R_REGISTER         0x00
#define NRF24_CMD_W_REGISTER         0x20
#define NRF24_CMD_R_RX_PAYLOAD       0x61
#define NRF24_CMD_W_TX_PAYLOAD       0xA0
#define NRF24_CMD_FLUSH_TX           0xE1
#define NRF24_CMD_FLUSH_RX           0xE2
#define NRF24_CMD_REUSE_TX_PL        0xE3
#define NRF24_CMD_ACTIVATE           0x50
#define NRF24_CMD_R_RX_PL_WID        0x60
#define NRF24_CMD_W_ACK_PAYLOAD      0xA8
#define NRF24_CMD_W_TX_PAYLOAD_NOACK 0xB0
#define NRF24_CMD_NOP                0xFF

// data structures
typedef struct
{
	UINT8 PRIM_RX     : 1;
	UINT8 PWR_UP      : 1;
	UINT8 CRCO        : 1;
	UINT8 EN_CRC      : 1;
	UINT8 MASK_MAX_RT : 1;
	UINT8 MASK_TX_DS  : 1;
	UINT8 MAX_RX_DR   : 1;
	UINT8 reserved    : 1;
		
} NRF24_CONFIG_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 ENAA_P0  : 1;
	UINT8 ENAA_P1  : 1;
	UINT8 ENAA_P2  : 1;
	UINT8 ENAA_P3  : 1;
	UINT8 ENAA_P4  : 1;
	UINT8 ENAA_P5  : 1;
	UINT8 reserved : 2;
} NRF24_EN_AA_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 ERX_P0   : 1;
	UINT8 ERX_P1   : 1;
	UINT8 ERX_P2   : 1;
	UINT8 ERX_P3   : 1;
	UINT8 ERX_P4   : 1;
	UINT8 ERX_P5   : 1;
	UINT8 reserved : 2;
		
} NRF24_EN_RXADDR_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 AW       : 2;
	UINT8 reserved : 6;
} NRF24_SETUP_AW_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 ARC : 4;
	UINT8 ARD : 4;
} NRF24_SETUP_RETR_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 RF_CH    : 7;
	UINT8 reserved : 1;
} NRF24_RF_CH_t;

// --------------------------------------------------------------------------
// nrf24l01+ variant
typedef struct
{
	UINT8 dontcare   : 1;
	UINT8 RF_PWR     : 2;
	UINT8 RF_DR_HIGH : 1;
	UINT8 PLL_LOCK   : 1;
	UINT8 RF_DR_LOW  : 1;
	UINT8 reserved   : 1;
	UINT8 CONT_WAVE  : 1;
} NRF24_RF_SETUP_PLUS_t;
// nrf24l01 variant
typedef struct
{
	UINT8 LNA_HCURR  : 1;
	UINT8 RF_PWR     : 2;
	UINT8 RF_DR      : 1;
	UINT8 PLL_LOCK   : 1;
	UINT8 reserved   : 3;
} NRF24_RF_SETUP_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 TX_FULL  : 1;
	UINT8 RX_P_NO  : 3;
	UINT8 MAX_RT   : 1;
	UINT8 TX_DS    : 1;
	UINT8 RX_DR    : 1;
	UINT8 reserved : 1;
} NRF24_STATUS_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 ARC_CNT  : 4;
	UINT8 PLOS_CNT : 4;
} NRF24_OBSERVE_TX_t;

// --------------------------------------------------------------------------
// same register different names for nrf24l01 and nrf24l01+ variants (RPD, CD)
typedef struct
{
	UINT8 CD       : 1;
	UINT8 reserved : 1;
} NRF24_CD_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 RX_EMPTY  : 1;
	UINT8 RX_FULL   : 1;
	UINT8 reserved1 : 2;
	UINT8 TX_EMPTY  : 1;
	UINT8 TX_FULL   : 1;
	UINT8 TX_REUSE  : 1;
	UINT8 reserved2 : 1;
} NRF24_FIFO_STATUS_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 EN_DYN_ACK : 1;
	UINT8 EN_ACK_PAY : 1;
	UINT8 EN_DPL     : 1;
	UINT8 reserved   : 5;
} NRF24_FEATURE_t;

// --------------------------------------------------------------------------
typedef struct
{
	UINT8 DPL_P0   : 1;
	UINT8 DPL_P1   : 1;
	UINT8 DPL_P2   : 1;
	UINT8 DPL_P3   : 1;
	UINT8 DPL_P4   : 1;
	UINT8 DPL_P5   : 1;
	UINT8 reserved : 2;
} NRF24_DYNPD_t;

// --------------------------------------------------------------------------
// union containing all of available registers
typedef union
{
	NRF24_CONFIG_t CONFIG;
	NRF24_EN_AA_t EN_AA;
	NRF24_EN_RXADDR_t EN_RXADDR;
	NRF24_SETUP_AW_t SETUP_AW;
	NRF24_SETUP_RETR_t SETUP_RETR;
	NRF24_RF_CH_t RF_CH;
	NRF24_RF_SETUP_PLUS_t RF_SETUP_PLUS;
	NRF24_RF_SETUP_t RF_SETUP;
	NRF24_STATUS_t STATUS;
	NRF24_OBSERVE_TX_t OBSERVE_TX;
	NRF24_CD_t CD;
	NRF24_FIFO_STATUS_t FIFO_STATUS;
	NRF24_FEATURE_t FEATURE;
	NRF24_DYNPD_t DYNPD;

	BYTE byte;
} u_nrf24_reg_t;

// --------------------------------------------------------------------------
typedef	enum
{
	NRF24_MODE_POWERDOWN = 0,
	NRF24_MODE_STANDBY,
	NRF24_MODE_TX,
	NRF24_MODE_RX
} e_nrf24_mode_t;

typedef enum {
	NRF24_PL_MIN = 0,
	NRF24_PL_LOW,
	NRF24_PL_HIGH,
	NRF24_PL_MAX
} e_nrf24_pl_t;

typedef enum
{
	NRF24_DRATE_1M = 0,
	NRF24_DRATE_2M,
	NRF24_DRATE_250K
} e_nrf24_drate_t;

typedef enum
{
	NRF24_ADRWIDTH_3 = 1,
	NRF24_ADRWIDTH_4,
	NRF24_ADRWIDTH_5
} e_nrf24_adrwidth_t;

typedef enum
{
	NRF24_CRC_DISABLED = 0,
	NRF24_CRC_8,
	NRF24_CRC_16
} e_nrf24_crc_t;


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

BOOL nrf24Init(const spi_cfg_st *spi_st);

BOOL nrf24SetMode(e_nrf24_mode_t mode);

UINT8 nrf24GetAddressWidth(void);

BOOL nrf24SetAddressWidth(e_nrf24_adrwidth_t width);
BOOL nrf24SetAutoRetransmit(UINT16 delay, UINT8 count);
BOOL nrf24SetChannel(UINT8 channel);
BOOL nrf24SetCrcLength(e_nrf24_crc_t crc);
BOOL nrf24SetDataRate(e_nrf24_drate_t drate);
BOOL nrf24SetPowerLevel(e_nrf24_pl_t power);

void nrf24FlushRx(void);
void nrf24FlushTx(void);

NRF24_STATUS_t nrf24GetStatus(void);
NRF24_FIFO_STATUS_t nrf24GetFifoStatus(void);

BOOL nrf24UnreadData(UINT8 *pipe_num);
BOOL nrf24UnsentData(void);

void nrf24SetRxPipeAddress(UINT8 pipe_num, BYTE *address, BOOL enable);
void nrf24SetTxPipeAddress(BYTE *address, BOOL ack);

void nrf24SetPipePayloadSize(UINT8 pipe_num, UINT8 size);

void nrf24SetRxPipeEnabled(UINT8 pipe_num, BOOL enable);
void nrf24SetRxPipeAutoAck(UINT8 pipe_num, BOOL enable);

UINT8 nrf24Receive(BYTE *buffer, UINT8 len, UINT8 *pipe_num, UINT16 timeout_ms);
UINT8 nrf24Send(BYTE* buffer, UINT8 len, BOOL ack, BOOL listen);

// debug helper
void nrf24DumpRegisters(void);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _NRF24L01_H

// END
