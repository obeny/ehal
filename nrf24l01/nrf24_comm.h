#ifndef _COMM_H
#define _COMM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "lib_mcu_noarch/global.h"


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

#define NRF24_COMM_BROADCAST_GROUP 0x00
#define NRF24_COMM_BROADCAST_ID 0x00

#define NRF24_COMM_IGNORE_FRAME_TYPE 0xFF

// frame types
typedef enum
{
	NRF24COM_FRM_UNKN = 0,
	NRF24COM_FRM_INIT,
	NRF24COM_FRM_INITACK,
	NRF24COM_FRM_DATA,
	NRF24COM_FRM_DATAACK
} e_nrf24com_frmtype_t;

// protocols
typedef enum
{
	NRF24COM_PROTO_UNKN = 0,
	NRF24COM_PROTO_V1
} e_nrf24com_proto_t;

// receiver state
typedef enum
{
	NRF24COM_DST_STATE_ACK = 0,
	NRF24COM_DST_STATE_WAIT,
	NRF24COM_DST_STATE_UNKN_PROTO
} nrf24com_dst_state_t;

// node id
typedef struct
{
	UINT8 priv  : 1;
	UINT8 group : 3;
	UINT8 id    : 4;
} nrf24com_node_id_t;

// frame header
typedef struct
{
	UINT8 proto    : 4;
	UINT8 frmtype  : 4;
	nrf24com_node_id_t dst_node_id;
} nrf24com_hdr_t;

// frame definitions
// --------------------------------------------------------------------------
typedef struct
{
	nrf24com_hdr_t header;
	nrf24com_node_id_t src_node_id;
	UINT16 length;
	UINT8 data_chksum;
	UINT8 chksum;
} nrf24com_frm_init;

// --------------------------------------------------------------------------
typedef struct
{
	nrf24com_hdr_t header;
	nrf24com_node_id_t src_node_id;
	nrf24com_dst_state_t dst_state;
	UINT8 chksum;
} nrf24com_frm_initack;

// --------------------------------------------------------------------------
typedef struct
{
	nrf24com_hdr_t header;
	UINT8 seq_id;
} nrf24com_frm_data;

// --------------------------------------------------------------------------
typedef struct
{
	nrf24com_hdr_t header;
	UINT8 seq_id;
	UINT8 chksum;
} nrf24com_frm_dataack;


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

BOOL nrf24CommSetAddress(BYTE *address, nrf24com_node_id_t node_id);

UINT16 nrf24ReceiveBlock(BYTE *data, UINT16 max_len, UINT16 timeout_ms, UINT8 retries);
BOOL nrf24SendBlock(BYTE *data, UINT16 len, nrf24com_node_id_t dst_node_id, UINT16 timeout_ms, UINT8 retries);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // _COMM_H

// END
