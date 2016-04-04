/***************************************************************************
 *	INCLUDES
 ***************************************************************************/

#include "config.h"

#include "ehal/nrf24l01/nrf24_comm.h"
#include "ehal/chksum/chksum.h"
#include "ehal/nrf24l01/nrf24l01.h"
#include "ehal/sync_timer/sync_timer.h"

#include <stdio.h>
#include <string.h>


/***************************************************************************
 *	DEFINITIONS
 ***************************************************************************/

#define DATAFRAME_PAYLOAD_SIZE (NRF24_PAYLOAD_SIZE_MAX - sizeof(nrf24com_frm_data))

#define TX_PIPE 0
#define NET_PIPE 1
#define PRIVATE_PIPE 2

// debug helpers
#ifdef NRF24COMM_DEBUG
#define nrf24comm_debug(...) \
{ \
	printf("NRF24COMM %s(): ", __FUNCTION__); \
	printf(__VA_ARGS__); \
}
#else
#define nrf24comm_debug(...)
#endif // NRF24COMM_DEBUG

// receive state machine
typedef enum
{
	FSM_RECVBLK_WAIT_FOR_INIT = 0,
	FSM_RECVBLK_SEND_INITACK,
	FSM_RECVBLK_ENABLE_PRIVATE_PIPE,
	FSM_RECVBLK_WAIT_FOR_DATA,
	FSM_RECVBLK_SEND_DATAACK,
	FSM_RECVBLK_END
} fsm_recvblk_state_t;

typedef struct
{
	fsm_recvblk_state_t state;

	BYTE *data;
	UINT16 max_len;
	UINT16 timeout_ms;
	UINT8 retries;
	BYTE *buffer;

	UINT8 data_chksum;
	UINT16 data_length;
	nrf24com_node_id_t sender_node_id;

	UINT8 initack_resp;
	UINT8 seq_id;
	UINT8 data_pos;
	UINT8 remaining_len;

	UINT16 received_len;
} fsm_recvblk_instance_t;

// send state machine
typedef enum
{
	FSM_SENDBLK_SEND_INIT = 0,
	FSM_SENDBLK_WAIT_FOR_INITACK,
	FSM_SENDBLK_ENABLE_PRIVATE_PIPE,
	FSM_SENDBLK_SEND_DATA,
	FSM_SENDBLK_WAIT_FOR_DATAACK,
	FSM_SENDBLK_END
} fsm_sendblk_state_t;

typedef struct
{
	fsm_sendblk_state_t state;

	BYTE *data;
	UINT16 len;
	UINT16 timeout_ms;
	UINT8 retries;
	BYTE *buffer;

	nrf24com_node_id_t recipient_node_id;
	UINT8 seq_id;
	UINT8 data_pos;
	UINT8 remaining_len;
	UINT8 retransmit;

	BOOL ok;
} fsm_sendblk_instance_t;

typedef enum
{
	CHECKHDR_OK = 0,
	CHECKHDR_UNKN_PROTO,
	CHECKHDR_DISCARD
} header_content_t;

static nrf24com_node_id_t current_node_id;
static BYTE current_address[NRF24_ADDR_SIZE_MAX];

// static functions
static void configureNetworkPipes(BOOL enable_private);
static void configurePrivatePipes(nrf24com_node_id_t node_id);

static void prepareInitFrame(BYTE *buffer, BYTE *data, UINT16 len, nrf24com_node_id_t dst_node_id);
static void prepareInitAckFrame(BYTE *buffer, nrf24com_node_id_t dst_node_id, nrf24com_dst_state_t dst_state);
static void prepareDataFrame(BYTE *buffer, nrf24com_node_id_t dst_node_id, UINT8 seq_id, BYTE *data, UINT8 len);
static void prepareDataAckFrame(BYTE *buffer, nrf24com_node_id_t dst_node_id, UINT8 seq_id);

static BOOL checkInitFrame(nrf24com_frm_init *frame);
static BOOL checkInitAckFrame(nrf24com_frm_initack *frame);
static BOOL checkDataFrame(nrf24com_frm_data *frame, UINT8 seq_id);
static BOOL checkDataAckFrame(nrf24com_frm_dataack *frame);

// receive state machine states
static void fsm_recvblk_waitForInit(fsm_recvblk_instance_t *instance);
static void fsm_recvblk_sendInitAck(fsm_recvblk_instance_t *instance);
static void fsm_recvblk_enablePrivatePipe(fsm_recvblk_instance_t *instance);
static void fsm_recvblk_waitForData(fsm_recvblk_instance_t *instance);
static void fsm_recvblk_sendDataAck(fsm_recvblk_instance_t *instance);

// send state machine states
static void fsm_sendblk_sendInit(fsm_sendblk_instance_t *instance);
static void fsm_sendblk_waitForInitAck(fsm_sendblk_instance_t *instance);
static void fsm_sendblk_enablePrivatePipe(fsm_sendblk_instance_t *instance);
static void fsm_sendblk_sendData(fsm_sendblk_instance_t *instance);
static void fsm_sendblk_waitForDataAck(fsm_sendblk_instance_t *instance);

// frame validity checking
static header_content_t checkHeader(nrf24com_hdr_t *hdr, UINT8 frame_type);
static BOOL checkDestination(nrf24com_node_id_t dst_node_id);

// debug helpers
#ifdef NRF24COMM_DEBUG
static const char* nrf24comm_debugGetReceiveState(fsm_recvblk_state_t state);
static const char* nrf24comm_debugGetSendState(fsm_sendblk_state_t state);
#endif // NRF24COMM_DEBUG


/***************************************************************************
 *	FUNCTIONS
 ***************************************************************************/

// --------------------------------------------------------------------------
BOOL nrf24CommSetAddress(BYTE *address, nrf24com_node_id_t node_id)
{
	UINT8 addr_width;

	// if node_id is some kind of broadcast, we cannot use that
	if (
		(NRF24_COMM_BROADCAST_GROUP == node_id.group) ||
		(NRF24_COMM_BROADCAST_ID == node_id.id) ||
		(1 == node_id.priv)
	)
	{
		nrf24comm_debug("couldn't set broadcast address as node_id\n");
		return (false);
	}

	addr_width = nrf24GetAddressWidth();

	// LSB is set to 0x00, indicating network address
	current_address[0] = 0x00;
	memcpy(current_address + 1, address, (addr_width - 1));
	current_node_id = node_id;
	return (true);
}

// --------------------------------------------------------------------------
UINT16 nrf24ReceiveBlock(BYTE *data, UINT16 max_len, UINT16 timeout_ms, UINT8 retries)
{
	BYTE buffer[NRF24_PAYLOAD_SIZE_MAX];
	fsm_recvblk_instance_t instance;

	// reinitialize pipes before doing anything
	configureNetworkPipes(false);

	// giveup it there is nothing in receive fifo
	if (!nrf24UnreadData(NULL))
		return (0);

	memset(&instance, 0, sizeof(instance));
	instance.state = FSM_RECVBLK_WAIT_FOR_INIT;
	instance.data = data;
	instance.max_len = max_len;
	instance.timeout_ms = timeout_ms;
	instance.retries = retries;
	instance.buffer = buffer;

	while (instance.state != FSM_RECVBLK_END)
	{
		nrf24comm_debug("state=%s\n", nrf24comm_debugGetReceiveState(instance.state));
		switch (instance.state)
		{
			case FSM_RECVBLK_WAIT_FOR_INIT:
				fsm_recvblk_waitForInit(&instance);
				break;
			case FSM_RECVBLK_SEND_INITACK:
				fsm_recvblk_sendInitAck(&instance);
				break;
			case FSM_RECVBLK_ENABLE_PRIVATE_PIPE:
				fsm_recvblk_enablePrivatePipe(&instance);
				break;
			case FSM_RECVBLK_WAIT_FOR_DATA:
				fsm_recvblk_waitForData(&instance);
				break;
			case FSM_RECVBLK_SEND_DATAACK:
				fsm_recvblk_sendDataAck(&instance);
				break;

			default:
				instance.state = FSM_RECVBLK_END;
				break;
		}
	}

	// restore pipe configuration - disable private pipe
	configureNetworkPipes(false);
	nrf24comm_debug("returns=%d\n", instance.received_len);
	return (instance.received_len);
}

// --------------------------------------------------------------------------
BOOL nrf24SendBlock(BYTE *data, UINT16 len, nrf24com_node_id_t dst_node_id, UINT16 timeout_ms, UINT8 retries)
{
	BYTE buffer[NRF24_PAYLOAD_SIZE_MAX];
	fsm_sendblk_instance_t instance;

	// reinitialize pipes before doing anything
	configureNetworkPipes(false);

	memset(&instance, 0, sizeof(instance));
	instance.state = FSM_SENDBLK_SEND_INIT;
	instance.data = data;
	instance.len = len;
	instance.remaining_len = len;
	instance.recipient_node_id = dst_node_id;
	instance.timeout_ms = timeout_ms;
	instance.retries = retries;
	instance.buffer = buffer;

	while (instance.state != FSM_SENDBLK_END)
	{
		nrf24comm_debug("state=%s\n", nrf24comm_debugGetSendState(instance.state));
		switch (instance.state)
		{
			case FSM_SENDBLK_SEND_INIT:
				fsm_sendblk_sendInit(&instance);
				break;
			case FSM_SENDBLK_WAIT_FOR_INITACK:
				fsm_sendblk_waitForInitAck(&instance);
				break;
			case FSM_SENDBLK_ENABLE_PRIVATE_PIPE:
				fsm_sendblk_enablePrivatePipe(&instance);
				break;
			case FSM_SENDBLK_SEND_DATA:
				fsm_sendblk_sendData(&instance);
				break;
			case FSM_SENDBLK_WAIT_FOR_DATAACK:
				fsm_sendblk_waitForDataAck(&instance);
				break;

			default:
				instance.state = FSM_SENDBLK_END;
				break;
		}
	}

	// restore pipe configuration - disable private pipe
	configureNetworkPipes(false);
	nrf24comm_debug("returns=%d\n", instance.ok);
	return (instance.ok);
}

// static functions
// --------------------------------------------------------------------------
static void configureNetworkPipes(BOOL enable_private)
{
	// disable autoack, switch to shockburst proto instead of ENHANCED shockburst
	nrf24SetRxPipeAutoAck(TX_PIPE, false);
	nrf24SetRxPipeAutoAck(NET_PIPE, false);
	nrf24SetRxPipeAutoAck(PRIVATE_PIPE, false);

	// assign network address to pipe 1
	nrf24SetRxPipeAddress(NET_PIPE, current_address, true);
	// disable pipe 0 as receive pipe, use it only for transmission
	nrf24SetTxPipeAddress(current_address, false);
	nrf24SetRxPipeEnabled(TX_PIPE, false);

	// handle private pipe
	nrf24SetRxPipeEnabled(PRIVATE_PIPE, enable_private);
}

// --------------------------------------------------------------------------
static void configurePrivatePipes(nrf24com_node_id_t node_id)
{
	BYTE address[NRF24_ADDR_SIZE_MAX];
	UINT8 addr_width = nrf24GetAddressWidth();

	// indicate that we are working on private pipe
	node_id.priv = 1;

	// initialize TX pipe with current network address with node id
	address[0] = *((BYTE*)&node_id);
	memcpy(address + 1, current_address + 1, addr_width - 1);

	nrf24SetTxPipeAddress(address, false);
	nrf24SetRxPipeAddress(PRIVATE_PIPE, (BYTE*)&node_id, true);
}

// --------------------------------------------------------------------------
static void prepareInitFrame(BYTE *buffer, BYTE *data, UINT16 len, nrf24com_node_id_t dst_node_id)
{
	nrf24com_frm_init *frame = (nrf24com_frm_init*)buffer;

	frame->header.proto = NRF24COM_PROTO_V1;
	frame->header.frmtype = NRF24COM_FRM_INIT;
	frame->header.dst_node_id = dst_node_id;

	frame->src_node_id = current_node_id;
	frame->length = len;
	frame->data_chksum = checksum8Bit(data, len);
	frame->chksum = checksum8Bit(buffer, sizeof(nrf24com_frm_init) - 1);
}

// --------------------------------------------------------------------------
static void prepareInitAckFrame(BYTE *buffer, nrf24com_node_id_t dst_node_id, nrf24com_dst_state_t dst_state)
{
	nrf24com_frm_initack *frame = (nrf24com_frm_initack*)buffer;

	frame->header.proto = NRF24COM_PROTO_V1;
	frame->header.frmtype = NRF24COM_FRM_INITACK;
	frame->header.dst_node_id = dst_node_id;

	frame->src_node_id = current_node_id;
	frame->dst_state = dst_state;
	frame->chksum = checksum8Bit(buffer, sizeof(nrf24com_frm_initack) - 1);
}

// --------------------------------------------------------------------------
static void prepareDataFrame(BYTE *buffer, nrf24com_node_id_t dst_node_id, UINT8 seq_id, BYTE *data, UINT8 len)
{
	nrf24com_frm_data *frame = (nrf24com_frm_data*)buffer;

	frame->header.proto = NRF24COM_PROTO_V1;
	frame->header.frmtype = NRF24COM_FRM_DATA;
	frame->header.dst_node_id = dst_node_id;

	memcpy(buffer + sizeof(nrf24com_frm_data), data, len);

	frame->seq_id = seq_id;
}

// --------------------------------------------------------------------------
static void prepareDataAckFrame(BYTE *buffer, nrf24com_node_id_t dst_node_id, UINT8 seq_id)
{
	nrf24com_frm_dataack *frame = (nrf24com_frm_dataack*)buffer;

	frame->header.proto = NRF24COM_PROTO_V1;
	frame->header.frmtype = NRF24COM_FRM_DATAACK;
	frame->header.dst_node_id = dst_node_id;

	frame->seq_id = seq_id;
	frame->chksum = checksum8Bit(buffer, sizeof(nrf24com_frm_dataack) - 1);
}

// --------------------------------------------------------------------------
static BOOL checkInitFrame(nrf24com_frm_init *frame)
{
	BOOL result = (frame->chksum == checksum8Bit((BYTE*)frame, sizeof(nrf24com_frm_init) - 1));
	nrf24comm_debug("result=%d\n", result);
	return (result);
}

// --------------------------------------------------------------------------
static BOOL checkInitAckFrame(nrf24com_frm_initack *frame)
{
	BOOL result = (frame->chksum == checksum8Bit((BYTE*)frame, sizeof(nrf24com_frm_initack) - 1));
	nrf24comm_debug("result=%d\n", result);
	return (result);
}

// --------------------------------------------------------------------------
static BOOL checkDataFrame(nrf24com_frm_data *frame, UINT8 seq_id)
{
	BOOL result = (frame->seq_id == seq_id);
	nrf24comm_debug("result=%d\n", result);
	return (result);
}

// --------------------------------------------------------------------------
static BOOL checkDataAckFrame(nrf24com_frm_dataack *frame)
{
	BOOL result = (frame->chksum == checksum8Bit((BYTE*)frame, sizeof(nrf24com_frm_dataack) - 1));
	nrf24comm_debug("result=%d\n", result);
	return (result);
}

// receive state machine states
// --------------------------------------------------------------------------
static void fsm_recvblk_waitForInit(fsm_recvblk_instance_t *instance)
{
	header_content_t hdr_content;
	nrf24com_frm_init *frame;

	UINT8 rx_pipe;

	// there must be something in fifo, so we do not have to wait
	do
	{
		if (sizeof(nrf24com_frm_init) == nrf24Receive(instance->buffer, sizeof(nrf24com_frm_init), &rx_pipe, instance->timeout_ms))
		{
			if (NET_PIPE != rx_pipe)
			{
				nrf24comm_debug("something in private pipe, discarding it\n");
				continue;
			}
			hdr_content = checkHeader((nrf24com_hdr_t*)instance->buffer, NRF24COM_FRM_INIT);
			if (CHECKHDR_OK == hdr_content)
			{
				frame = (nrf24com_frm_init*)instance->buffer;
				if (checkInitFrame(frame))
				{
					instance->data_length = frame->length;
					// if data to receive is bigger then our expectation, just give up
					if (instance->data_length > instance->max_len)
						break;

					instance->remaining_len = frame->length;
					instance->data_chksum = frame->data_chksum;
					instance->sender_node_id = frame->src_node_id;

					// inform sender, that we are ready to go
					instance->initack_resp = NRF24COM_DST_STATE_ACK;
					instance->state = FSM_RECVBLK_SEND_INITACK;
					return;
				}
			}
			else if (CHECKHDR_UNKN_PROTO == hdr_content)
			{
				// inform sender, that it is using protocol we do not understand (currently this cannot happen)
				instance->initack_resp = NRF24COM_DST_STATE_UNKN_PROTO;
				instance->state = FSM_RECVBLK_SEND_INITACK;
				return;
			}
		}
	} while (nrf24UnreadData(NULL));

	instance->state = FSM_RECVBLK_END;
}

// --------------------------------------------------------------------------
static void fsm_recvblk_sendInitAck(fsm_recvblk_instance_t *instance)
{
	prepareInitAckFrame(instance->buffer, instance->sender_node_id, instance->initack_resp);
	if (sizeof(nrf24com_frm_initack) == nrf24Send(instance->buffer, sizeof(nrf24com_frm_initack), false, true))
		instance->state = FSM_RECVBLK_ENABLE_PRIVATE_PIPE;
	else
	{
		nrf24comm_debug("sending failed\n");
		instance->state = FSM_RECVBLK_END;
		return;
	}

	// start from beginning if unknown protocol detected
	if (NRF24COM_DST_STATE_UNKN_PROTO == instance->initack_resp)
	{
		nrf24comm_debug("unknown protocol\n");
		instance->state = FSM_RECVBLK_WAIT_FOR_INIT;
	}
}

// --------------------------------------------------------------------------
static void fsm_recvblk_enablePrivatePipe(fsm_recvblk_instance_t *instance)
{
	configurePrivatePipes(current_node_id);
	instance->state = FSM_RECVBLK_WAIT_FOR_DATA;
}

// --------------------------------------------------------------------------
static void fsm_recvblk_waitForData(fsm_recvblk_instance_t *instance)
{
	header_content_t hdr_content;
	nrf24com_frm_data *frame_data;
	nrf24com_frm_init *frame_init;
	UINT8 rx_pipe;

	UINT8 retry = 0;
	UINT8 data_len = (instance->remaining_len > DATAFRAME_PAYLOAD_SIZE)?(DATAFRAME_PAYLOAD_SIZE):(instance->remaining_len);

	do
	{
		if (NRF24_PAYLOAD_SIZE_MAX == nrf24Receive(instance->buffer, NRF24_PAYLOAD_SIZE_MAX, &rx_pipe, instance->timeout_ms))
		{
			// now we expect something in private pipe
			if (PRIVATE_PIPE == rx_pipe)
			{
				hdr_content = checkHeader((nrf24com_hdr_t*)instance->buffer, NRF24COM_FRM_DATA);
				if (CHECKHDR_OK == hdr_content)
				{
					frame_data = (nrf24com_frm_data*)instance->buffer;
					if (checkDataFrame(frame_data, instance->seq_id))
					{
						// copy data and update status for next data frame
						memcpy(instance->data + instance->data_pos, instance->buffer + sizeof(nrf24com_frm_data), data_len);
						instance->data_pos += data_len;
						instance->remaining_len -= data_len;
						instance->state = FSM_RECVBLK_SEND_DATAACK;
						return;
					}
					else
						break;
				}
			}
			// only init frame is interesting on network pipe
			else if (NET_PIPE == rx_pipe)
			{
				nrf24comm_debug("something in network pipe, send wait if init frame\n");
				hdr_content = checkHeader((nrf24com_hdr_t*)instance->buffer, NRF24COM_FRM_INIT);
				if (CHECKHDR_OK == hdr_content)
				{
					frame_init = (nrf24com_frm_init*)instance->buffer;
					if (checkInitFrame(frame_init))
					{
						nrf24comm_debug("sending wait\n");
						prepareInitAckFrame(instance->buffer, frame_init->src_node_id, NRF24COM_DST_STATE_WAIT);
						// set tx addr for network pipe and send initack with wait request to sender
						configureNetworkPipes(true);
						nrf24Send(instance->buffer, sizeof(nrf24com_frm_initack), false, true);
						// restore tx addr of private pipe
						configurePrivatePipes(current_node_id);
						continue;
					}
				}
				else
				{
					nrf24comm_debug("not init frame, discarding it\n");
				}
			}
		}
		++retry;
		nrf24comm_debug("retrying: %d/%d\n", retry, instance->retries);
	} while (retry < instance->retries);

	instance->state = FSM_SENDBLK_END;
}

// --------------------------------------------------------------------------
static void fsm_recvblk_sendDataAck(fsm_recvblk_instance_t *instance)
{
	UINT8 chksum;

	prepareDataAckFrame(instance->buffer, instance->sender_node_id, instance->seq_id);
	if (sizeof(nrf24com_frm_dataack) == nrf24Send(instance->buffer, sizeof(nrf24com_frm_dataack), false, true))
	{
		if (0 == instance->remaining_len)
		{
			chksum = checksum8Bit(instance->data, instance->data_length);
			if (instance->data_chksum == chksum)
				instance->received_len = instance->data_length;
			else
			{
				nrf24comm_debug("checksum do not match, got=%d, expected=%d\n", chksum, instance->data_chksum);
			}
			instance->state = FSM_RECVBLK_END;
		}
		else
		{
			instance->seq_id++;
			instance->state = FSM_RECVBLK_WAIT_FOR_DATA;
		}
	}
	else
	{
		nrf24comm_debug("sending failed\n");
		instance->state = FSM_RECVBLK_END;
	}
}

// send state machine states
// --------------------------------------------------------------------------
static void fsm_sendblk_sendInit(fsm_sendblk_instance_t *instance)
{
	// recipient may be broadcast, we will replace it after receiving initAck
	prepareInitFrame(instance->buffer, instance->data, instance->len, instance->recipient_node_id);
	if (sizeof(nrf24com_frm_init) == nrf24Send(instance->buffer, sizeof(nrf24com_frm_init), false, true))
		instance->state = FSM_SENDBLK_WAIT_FOR_INITACK;
	else
	{
		nrf24comm_debug("sending failed\n");
		instance->state = FSM_SENDBLK_END;
	}
}

// --------------------------------------------------------------------------
static void fsm_sendblk_waitForInitAck(fsm_sendblk_instance_t *instance)
{
	header_content_t hdr_content;
	nrf24com_frm_initack *frame;
	UINT8 rx_pipe;
	
	UINT8 retry = 0;

	do
	{
		if (sizeof(nrf24com_frm_initack) == nrf24Receive(instance->buffer, sizeof(nrf24com_frm_initack), &rx_pipe, instance->timeout_ms))
		{
			if (NET_PIPE != rx_pipe)
			{
				nrf24comm_debug("something in private pipe, discarding it\n");
				continue;
			}
			hdr_content = checkHeader((nrf24com_hdr_t*)instance->buffer, NRF24COM_FRM_INITACK);
			if (CHECKHDR_OK == hdr_content)
			{
				frame = (nrf24com_frm_initack*)instance->buffer;
				if (checkInitAckFrame(frame))
				{
					if (NRF24COM_DST_STATE_ACK == frame->dst_state)
					{
						// now we know who replied, update recipient node_id, we want to talk only to this node
						instance->recipient_node_id = frame->src_node_id;
						instance->state = FSM_SENDBLK_ENABLE_PRIVATE_PIPE;
						return;
					}
					else if (NRF24COM_DST_STATE_WAIT == frame->dst_state)
					{
						nrf24comm_debug("receiver not ready, waiting\n");
						retry = 0;
						continue;
					}
					else
						break;
				}
			}
			else
				break;
		}
		++retry;
		nrf24comm_debug("retrying: %d/%d\n", retry, instance->retries);
	} while (retry < instance->retries);

	instance->state = FSM_SENDBLK_END;
}

// --------------------------------------------------------------------------
static void fsm_sendblk_enablePrivatePipe(fsm_sendblk_instance_t *instance)
{
	configurePrivatePipes(instance->recipient_node_id);
	instance->state = FSM_SENDBLK_SEND_DATA;
}

// --------------------------------------------------------------------------
static void fsm_sendblk_sendData(fsm_sendblk_instance_t *instance)
{
	UINT8 data_len = (instance->remaining_len > DATAFRAME_PAYLOAD_SIZE)?(DATAFRAME_PAYLOAD_SIZE):(instance->remaining_len);

	prepareDataFrame(instance->buffer, instance->recipient_node_id, instance->seq_id, instance->data + instance->data_pos, data_len);
	if (NRF24_PAYLOAD_SIZE_MAX == nrf24Send(instance->buffer, NRF24_PAYLOAD_SIZE_MAX, false, true))
		instance->state = FSM_SENDBLK_WAIT_FOR_DATAACK;
	else
	{
		nrf24comm_debug("sending failed\n");
		instance->state = FSM_SENDBLK_END;
	}
}
// --------------------------------------------------------------------------
static void fsm_sendblk_waitForDataAck(fsm_sendblk_instance_t *instance)
{
	header_content_t hdr_content;
	nrf24com_frm_dataack *frame;

	UINT8 data_len = (instance->remaining_len > DATAFRAME_PAYLOAD_SIZE)?(DATAFRAME_PAYLOAD_SIZE):(instance->remaining_len);

	do
	{
		// consume all frames that are not dataack
		if (sizeof(nrf24com_frm_dataack) == nrf24Receive(instance->buffer, sizeof(nrf24com_frm_dataack), NULL, instance->timeout_ms))
		{
			hdr_content = checkHeader((nrf24com_hdr_t*)instance->buffer, NRF24COM_FRM_DATAACK);
			if (CHECKHDR_OK == hdr_content)
			{
				frame = (nrf24com_frm_dataack*)instance->buffer;
				if (checkDataAckFrame(frame))
				{
					// update status for next data frame
					instance->data_pos += data_len;
					instance->remaining_len -= data_len;
					if (0 == instance->remaining_len)
					{
						// we're done
						instance->ok = 1;
						instance->state = FSM_SENDBLK_END;
						return;
					}
					else
					{
						// send next frame
						instance->retransmit = 0;
						instance->seq_id++;
						instance->state = FSM_SENDBLK_SEND_DATA;
						return;
					}
				}
			}
		}
		else
			instance->state = FSM_SENDBLK_END;
	} while (nrf24UnreadData(NULL));

	if (instance->retransmit < instance->retries)
	{
		instance->retransmit++;
		nrf24comm_debug("no data ack received, retransmitting frame, retrying: %d/%d\n", instance->retransmit, instance->retries);
		instance->state = FSM_SENDBLK_SEND_DATA;
	}
	else
	{
		nrf24comm_debug("giving up\n");
		instance->state = FSM_SENDBLK_END;
	}
}

// frame validity checking
// --------------------------------------------------------------------------
static header_content_t checkHeader(nrf24com_hdr_t *hdr, UINT8 frame_type)
{
	header_content_t ret = CHECKHDR_OK;

	if (hdr->proto != NRF24COM_PROTO_V1)
	{
		ret = CHECKHDR_UNKN_PROTO;
		nrf24comm_debug("unknown protocol\n");
		goto end;
	}
	if (NRF24_COMM_IGNORE_FRAME_TYPE != frame_type)
	{
		if (hdr->frmtype != frame_type)
		{
			ret = CHECKHDR_DISCARD;
			nrf24comm_debug("unexpected frame\n");
			goto end;
		}
	}
	if (!checkDestination(hdr->dst_node_id))
	{
		nrf24comm_debug("destination mismatch\n");
		ret = CHECKHDR_DISCARD;
	}

end:
	return (ret);
}

// --------------------------------------------------------------------------
static BOOL checkDestination(nrf24com_node_id_t dst_node_id)
{
	if ((dst_node_id.group == current_node_id.group) && (dst_node_id.id == current_node_id.id))
		return (true);
	else if (NRF24_COMM_BROADCAST_ID == dst_node_id.id)
	{
		if (dst_node_id.group == current_node_id.group)
			return (true);
		return (false);
	}
	return (true);
}

#ifdef NRF24COMM_DEBUG
// debug helpers
// --------------------------------------------------------------------------
static const char* nrf24comm_debugGetReceiveState(fsm_recvblk_state_t state)
{
	switch (state)
	{
		case FSM_RECVBLK_WAIT_FOR_INIT:
			return ("WAIT_FOR_INIT");
		case FSM_RECVBLK_SEND_INITACK:
			return ("SEND_INITACK");
		case FSM_RECVBLK_ENABLE_PRIVATE_PIPE:
			return ("ENABLE_PRIVATE_PIPE");
		case FSM_RECVBLK_WAIT_FOR_DATA:
			return ("WAIT_FOR_DATA");
		case FSM_RECVBLK_SEND_DATAACK:
			return ("SEND_DATAACK");
		case FSM_RECVBLK_END:
			return ("END");
		default:
			return ("UNKNOWN");
	}
}

// --------------------------------------------------------------------------
static const char* nrf24comm_debugGetSendState(fsm_sendblk_state_t state)
{
	switch (state)
	{
		case FSM_SENDBLK_SEND_INIT:
			return ("SEND_INIT");
		case FSM_SENDBLK_WAIT_FOR_INITACK:
			return ("WAIT_FOR_INITACK");
		case FSM_SENDBLK_ENABLE_PRIVATE_PIPE:
			return ("ENABLE_PRIVATE_PIPE");
		case FSM_SENDBLK_SEND_DATA:
			return ("SEND_DATA");
		case FSM_SENDBLK_WAIT_FOR_DATAACK:
			return ("WAIT_FOR_DATAACK");
		case FSM_SENDBLK_END:
			return ("END");
		default:
			return ("UNKNOWN");
	}
}
#endif // NRF24COMM_DEBUG

// END
