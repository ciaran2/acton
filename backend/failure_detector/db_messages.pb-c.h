/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: db_messages.proto */

#ifndef PROTOBUF_C_db_5fmessages_2eproto__INCLUDED
#define PROTOBUF_C_db_5fmessages_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003002 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _NodeStateMessage NodeStateMessage;
typedef struct _VectorClockMessage VectorClockMessage;
typedef struct _GossipMessage GossipMessage;
typedef struct _MembershipViewMessage MembershipViewMessage;
typedef struct _MembershipAgreementMessage MembershipAgreementMessage;
typedef struct _CellAddressMessage CellAddressMessage;
typedef struct _CellMessage CellMessage;
typedef struct _VersionedCellMessage VersionedCellMessage;
typedef struct _WriteQueryMessage WriteQueryMessage;
typedef struct _ReadQueryMessage ReadQueryMessage;
typedef struct _AckMessage AckMessage;
typedef struct _RangeReadQueryMessage RangeReadQueryMessage;
typedef struct _RangeReadResponseMessage RangeReadResponseMessage;
typedef struct _TxnMessage TxnMessage;
typedef struct _QueueQueryMessage QueueQueryMessage;
typedef struct _ConsumerID ConsumerID;
typedef struct _CreateQueueMessage CreateQueueMessage;
typedef struct _DeleteQueueMessage DeleteQueueMessage;
typedef struct _SubscribeQueueMessage SubscribeQueueMessage;
typedef struct _UnsubscribeQueueMessage UnsubscribeQueueMessage;
typedef struct _EnqueueMessage EnqueueMessage;
typedef struct _EnqueueResponseMessage EnqueueResponseMessage;
typedef struct _ReadQueueMessage ReadQueueMessage;
typedef struct _ReadQueueResponseMessage ReadQueueResponseMessage;
typedef struct _ConsumeQueueMessage ConsumeQueueMessage;
typedef struct _ConsumeQueueResponseMessage ConsumeQueueResponseMessage;


/* --- enums --- */


/* --- messages --- */

struct  _NodeStateMessage
{
  ProtobufCMessage base;
  /*
   * 0 - dead, 1 - ready, 2 - alive but not ready
   */
  int32_t status;
  int32_t node_id;
  int32_t rack_id;
  int32_t dc_id;
};
#define NODE_STATE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&node_state_message__descriptor) \
    , 0, 0, 0, 0 }


struct  _VectorClockMessage
{
  ProtobufCMessage base;
  size_t n_ids;
  int32_t *ids;
  size_t n_counters;
  int64_t *counters;
};
#define VECTOR_CLOCK_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&vector_clock_message__descriptor) \
    , 0,NULL, 0,NULL }


struct  _GossipMessage
{
  ProtobufCMessage base;
  NodeStateMessage *node_state;
  VectorClockMessage *vc;
};
#define GOSSIP_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gossip_message__descriptor) \
    , NULL, NULL }


struct  _MembershipViewMessage
{
  ProtobufCMessage base;
  size_t n_membership;
  NodeStateMessage **membership;
  VectorClockMessage *view_id;
};
#define MEMBERSHIP_VIEW_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&membership_view_message__descriptor) \
    , 0,NULL, NULL }


struct  _MembershipAgreementMessage
{
  ProtobufCMessage base;
  /*
   * 0 - PROPOSE, 1 - RESPONSE, 2 - NOTIFY, 3 - RETRY_LINK, 4 - NOTIFY_ACK
   */
  int32_t msg_type;
  /*
   * 0 - ACK, 1 - NACK, 2 - UNINIT
   */
  int32_t ack_status;
  MembershipViewMessage *view;
  VectorClockMessage *vc;
};
#define MEMBERSHIP_AGREEMENT_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&membership_agreement_message__descriptor) \
    , 0, 0, NULL, NULL }


struct  _CellAddressMessage
{
  ProtobufCMessage base;
  int64_t table_key;
  size_t n_keys;
  int64_t *keys;
};
#define CELL_ADDRESS_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&cell_address_message__descriptor) \
    , 0, 0,NULL }


struct  _CellMessage
{
  ProtobufCMessage base;
  int64_t table_key;
  size_t n_keys;
  int64_t *keys;
  size_t n_columns;
  int64_t *columns;
};
#define CELL_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&cell_message__descriptor) \
    , 0, 0,NULL, 0,NULL }


struct  _VersionedCellMessage
{
  ProtobufCMessage base;
  int64_t table_key;
  size_t n_keys;
  int64_t *keys;
  size_t n_columns;
  int64_t *columns;
  /*
   *	optional int64 version_no=5;
   */
  VectorClockMessage *version;
};
#define VERSIONED_CELL_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&versioned_cell_message__descriptor) \
    , 0, 0,NULL, 0,NULL, NULL }


struct  _WriteQueryMessage
{
  ProtobufCMessage base;
  /*
   * CellMessage
   */
  VersionedCellMessage *cell;
  ProtobufCBinaryData txnid;
  int64_t nonce;
  /*
   * {RPC_TYPE_WRITE, RPC_TYPE_DELETE}
   */
  int32_t msg_type;
  int32_t mtype;
};
#define WRITE_QUERY_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&write_query_message__descriptor) \
    , NULL, {0,NULL}, 0, 0, 0 }


struct  _ReadQueryMessage
{
  ProtobufCMessage base;
  CellAddressMessage *cell_address;
  ProtobufCBinaryData txnid;
  int64_t nonce;
  int32_t mtype;
};
#define READ_QUERY_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&read_query_message__descriptor) \
    , NULL, {0,NULL}, 0, 0 }


struct  _AckMessage
{
  ProtobufCMessage base;
  CellAddressMessage *cell_address;
  /*
   * 0 - ACK, 1 - NACK
   */
  int32_t status;
  ProtobufCBinaryData txnid;
  int64_t nonce;
  int32_t mtype;
};
#define ACK_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ack_message__descriptor) \
    , NULL, 0, {0,NULL}, 0, 0 }


struct  _RangeReadQueryMessage
{
  ProtobufCMessage base;
  CellAddressMessage *start_cell_address;
  CellAddressMessage *end_cell_address;
  ProtobufCBinaryData txnid;
  int64_t nonce;
  int32_t mtype;
};
#define RANGE_READ_QUERY_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&range_read_query_message__descriptor) \
    , NULL, NULL, {0,NULL}, 0, 0 }


struct  _RangeReadResponseMessage
{
  ProtobufCMessage base;
  size_t n_cells;
  VersionedCellMessage **cells;
  ProtobufCBinaryData txnid;
  int64_t nonce;
  int32_t mtype;
};
#define RANGE_READ_RESPONSE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&range_read_response_message__descriptor) \
    , 0,NULL, {0,NULL}, 0, 0 }


struct  _TxnMessage
{
  ProtobufCMessage base;
  /*
   * BEGIN=0, VALIDATION=1, COMMIT=2, ABORT=3
   */
  int32_t type;
  size_t n_own_read_set;
  VersionedCellMessage **own_read_set;
  size_t n_own_write_set;
  VersionedCellMessage **own_write_set;
  size_t n_complete_read_set;
  VersionedCellMessage **complete_read_set;
  size_t n_complete_write_set;
  VersionedCellMessage **complete_write_set;
  ProtobufCBinaryData txnid;
  int64_t nonce;
  VectorClockMessage *version;
  int32_t mtype;
};
#define TXN_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&txn_message__descriptor) \
    , 0, 0,NULL, 0,NULL, 0,NULL, 0,NULL, {0,NULL}, 0, NULL, 0 }


struct  _QueueQueryMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  int32_t msg_type;
  int32_t app_id;
  int32_t shard_id;
  int32_t consumer_id;
  int32_t queue_index;
  int32_t status;
  size_t n_cells;
  VersionedCellMessage **cells;
  ProtobufCBinaryData txnid;
  int64_t nonce;
  int32_t mtype;
};
#define QUEUE_QUERY_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&queue_query_message__descriptor) \
    , NULL, 0, 0, 0, 0, 0, 0, 0,NULL, {0,NULL}, 0, 0 }


struct  _ConsumerID
{
  ProtobufCMessage base;
  int32_t app_id;
  int32_t shard_id;
  int32_t consumer_id;
};
#define CONSUMER_ID__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&consumer_id__descriptor) \
    , 0, 0, 0 }


struct  _CreateQueueMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define CREATE_QUEUE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&create_queue_message__descriptor) \
    , NULL, {0,NULL}, 0 }


struct  _DeleteQueueMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define DELETE_QUEUE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&delete_queue_message__descriptor) \
    , NULL, {0,NULL}, 0 }


struct  _SubscribeQueueMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  ConsumerID *consumer_id;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define SUBSCRIBE_QUEUE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&subscribe_queue_message__descriptor) \
    , NULL, NULL, {0,NULL}, 0 }


struct  _UnsubscribeQueueMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  ConsumerID *consumer_id;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define UNSUBSCRIBE_QUEUE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&unsubscribe_queue_message__descriptor) \
    , NULL, NULL, {0,NULL}, 0 }


struct  _EnqueueMessage
{
  ProtobufCMessage base;
  /*
   * CellMessage
   */
  size_t n_cells;
  VersionedCellMessage **cells;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define ENQUEUE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&enqueue_message__descriptor) \
    , 0,NULL, {0,NULL}, 0 }


struct  _EnqueueResponseMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  int64_t last_item_id;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define ENQUEUE_RESPONSE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&enqueue_response_message__descriptor) \
    , NULL, 0, {0,NULL}, 0 }


struct  _ReadQueueMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  ConsumerID *consumer_id;
  int64_t max_items;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define READ_QUEUE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&read_queue_message__descriptor) \
    , NULL, NULL, 0, {0,NULL}, 0 }


struct  _ReadQueueResponseMessage
{
  ProtobufCMessage base;
  size_t n_queue_entries;
  VersionedCellMessage **queue_entries;
  ConsumerID *consumer_id;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define READ_QUEUE_RESPONSE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&read_queue_response_message__descriptor) \
    , 0,NULL, NULL, {0,NULL}, 0 }


struct  _ConsumeQueueMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  ConsumerID *consumer_id;
  int64_t new_consume_head;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define CONSUME_QUEUE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&consume_queue_message__descriptor) \
    , NULL, NULL, 0, {0,NULL}, 0 }


struct  _ConsumeQueueResponseMessage
{
  ProtobufCMessage base;
  CellAddressMessage *queue_address;
  ConsumerID *consumer_id;
  /*
   * 0 - ACK, 1 - NACK
   */
  int32_t status;
  int64_t new_consume_head;
  ProtobufCBinaryData txnid;
  int64_t nonce;
};
#define CONSUME_QUEUE_RESPONSE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&consume_queue_response_message__descriptor) \
    , NULL, NULL, 0, 0, {0,NULL}, 0 }


/* NodeStateMessage methods */
void   node_state_message__init
                     (NodeStateMessage         *message);
size_t node_state_message__get_packed_size
                     (const NodeStateMessage   *message);
size_t node_state_message__pack
                     (const NodeStateMessage   *message,
                      uint8_t             *out);
size_t node_state_message__pack_to_buffer
                     (const NodeStateMessage   *message,
                      ProtobufCBuffer     *buffer);
NodeStateMessage *
       node_state_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   node_state_message__free_unpacked
                     (NodeStateMessage *message,
                      ProtobufCAllocator *allocator);
/* VectorClockMessage methods */
void   vector_clock_message__init
                     (VectorClockMessage         *message);
size_t vector_clock_message__get_packed_size
                     (const VectorClockMessage   *message);
size_t vector_clock_message__pack
                     (const VectorClockMessage   *message,
                      uint8_t             *out);
size_t vector_clock_message__pack_to_buffer
                     (const VectorClockMessage   *message,
                      ProtobufCBuffer     *buffer);
VectorClockMessage *
       vector_clock_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   vector_clock_message__free_unpacked
                     (VectorClockMessage *message,
                      ProtobufCAllocator *allocator);
/* GossipMessage methods */
void   gossip_message__init
                     (GossipMessage         *message);
size_t gossip_message__get_packed_size
                     (const GossipMessage   *message);
size_t gossip_message__pack
                     (const GossipMessage   *message,
                      uint8_t             *out);
size_t gossip_message__pack_to_buffer
                     (const GossipMessage   *message,
                      ProtobufCBuffer     *buffer);
GossipMessage *
       gossip_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gossip_message__free_unpacked
                     (GossipMessage *message,
                      ProtobufCAllocator *allocator);
/* MembershipViewMessage methods */
void   membership_view_message__init
                     (MembershipViewMessage         *message);
size_t membership_view_message__get_packed_size
                     (const MembershipViewMessage   *message);
size_t membership_view_message__pack
                     (const MembershipViewMessage   *message,
                      uint8_t             *out);
size_t membership_view_message__pack_to_buffer
                     (const MembershipViewMessage   *message,
                      ProtobufCBuffer     *buffer);
MembershipViewMessage *
       membership_view_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   membership_view_message__free_unpacked
                     (MembershipViewMessage *message,
                      ProtobufCAllocator *allocator);
/* MembershipAgreementMessage methods */
void   membership_agreement_message__init
                     (MembershipAgreementMessage         *message);
size_t membership_agreement_message__get_packed_size
                     (const MembershipAgreementMessage   *message);
size_t membership_agreement_message__pack
                     (const MembershipAgreementMessage   *message,
                      uint8_t             *out);
size_t membership_agreement_message__pack_to_buffer
                     (const MembershipAgreementMessage   *message,
                      ProtobufCBuffer     *buffer);
MembershipAgreementMessage *
       membership_agreement_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   membership_agreement_message__free_unpacked
                     (MembershipAgreementMessage *message,
                      ProtobufCAllocator *allocator);
/* CellAddressMessage methods */
void   cell_address_message__init
                     (CellAddressMessage         *message);
size_t cell_address_message__get_packed_size
                     (const CellAddressMessage   *message);
size_t cell_address_message__pack
                     (const CellAddressMessage   *message,
                      uint8_t             *out);
size_t cell_address_message__pack_to_buffer
                     (const CellAddressMessage   *message,
                      ProtobufCBuffer     *buffer);
CellAddressMessage *
       cell_address_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cell_address_message__free_unpacked
                     (CellAddressMessage *message,
                      ProtobufCAllocator *allocator);
/* CellMessage methods */
void   cell_message__init
                     (CellMessage         *message);
size_t cell_message__get_packed_size
                     (const CellMessage   *message);
size_t cell_message__pack
                     (const CellMessage   *message,
                      uint8_t             *out);
size_t cell_message__pack_to_buffer
                     (const CellMessage   *message,
                      ProtobufCBuffer     *buffer);
CellMessage *
       cell_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cell_message__free_unpacked
                     (CellMessage *message,
                      ProtobufCAllocator *allocator);
/* VersionedCellMessage methods */
void   versioned_cell_message__init
                     (VersionedCellMessage         *message);
size_t versioned_cell_message__get_packed_size
                     (const VersionedCellMessage   *message);
size_t versioned_cell_message__pack
                     (const VersionedCellMessage   *message,
                      uint8_t             *out);
size_t versioned_cell_message__pack_to_buffer
                     (const VersionedCellMessage   *message,
                      ProtobufCBuffer     *buffer);
VersionedCellMessage *
       versioned_cell_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   versioned_cell_message__free_unpacked
                     (VersionedCellMessage *message,
                      ProtobufCAllocator *allocator);
/* WriteQueryMessage methods */
void   write_query_message__init
                     (WriteQueryMessage         *message);
size_t write_query_message__get_packed_size
                     (const WriteQueryMessage   *message);
size_t write_query_message__pack
                     (const WriteQueryMessage   *message,
                      uint8_t             *out);
size_t write_query_message__pack_to_buffer
                     (const WriteQueryMessage   *message,
                      ProtobufCBuffer     *buffer);
WriteQueryMessage *
       write_query_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   write_query_message__free_unpacked
                     (WriteQueryMessage *message,
                      ProtobufCAllocator *allocator);
/* ReadQueryMessage methods */
void   read_query_message__init
                     (ReadQueryMessage         *message);
size_t read_query_message__get_packed_size
                     (const ReadQueryMessage   *message);
size_t read_query_message__pack
                     (const ReadQueryMessage   *message,
                      uint8_t             *out);
size_t read_query_message__pack_to_buffer
                     (const ReadQueryMessage   *message,
                      ProtobufCBuffer     *buffer);
ReadQueryMessage *
       read_query_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   read_query_message__free_unpacked
                     (ReadQueryMessage *message,
                      ProtobufCAllocator *allocator);
/* AckMessage methods */
void   ack_message__init
                     (AckMessage         *message);
size_t ack_message__get_packed_size
                     (const AckMessage   *message);
size_t ack_message__pack
                     (const AckMessage   *message,
                      uint8_t             *out);
size_t ack_message__pack_to_buffer
                     (const AckMessage   *message,
                      ProtobufCBuffer     *buffer);
AckMessage *
       ack_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ack_message__free_unpacked
                     (AckMessage *message,
                      ProtobufCAllocator *allocator);
/* RangeReadQueryMessage methods */
void   range_read_query_message__init
                     (RangeReadQueryMessage         *message);
size_t range_read_query_message__get_packed_size
                     (const RangeReadQueryMessage   *message);
size_t range_read_query_message__pack
                     (const RangeReadQueryMessage   *message,
                      uint8_t             *out);
size_t range_read_query_message__pack_to_buffer
                     (const RangeReadQueryMessage   *message,
                      ProtobufCBuffer     *buffer);
RangeReadQueryMessage *
       range_read_query_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   range_read_query_message__free_unpacked
                     (RangeReadQueryMessage *message,
                      ProtobufCAllocator *allocator);
/* RangeReadResponseMessage methods */
void   range_read_response_message__init
                     (RangeReadResponseMessage         *message);
size_t range_read_response_message__get_packed_size
                     (const RangeReadResponseMessage   *message);
size_t range_read_response_message__pack
                     (const RangeReadResponseMessage   *message,
                      uint8_t             *out);
size_t range_read_response_message__pack_to_buffer
                     (const RangeReadResponseMessage   *message,
                      ProtobufCBuffer     *buffer);
RangeReadResponseMessage *
       range_read_response_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   range_read_response_message__free_unpacked
                     (RangeReadResponseMessage *message,
                      ProtobufCAllocator *allocator);
/* TxnMessage methods */
void   txn_message__init
                     (TxnMessage         *message);
size_t txn_message__get_packed_size
                     (const TxnMessage   *message);
size_t txn_message__pack
                     (const TxnMessage   *message,
                      uint8_t             *out);
size_t txn_message__pack_to_buffer
                     (const TxnMessage   *message,
                      ProtobufCBuffer     *buffer);
TxnMessage *
       txn_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   txn_message__free_unpacked
                     (TxnMessage *message,
                      ProtobufCAllocator *allocator);
/* QueueQueryMessage methods */
void   queue_query_message__init
                     (QueueQueryMessage         *message);
size_t queue_query_message__get_packed_size
                     (const QueueQueryMessage   *message);
size_t queue_query_message__pack
                     (const QueueQueryMessage   *message,
                      uint8_t             *out);
size_t queue_query_message__pack_to_buffer
                     (const QueueQueryMessage   *message,
                      ProtobufCBuffer     *buffer);
QueueQueryMessage *
       queue_query_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   queue_query_message__free_unpacked
                     (QueueQueryMessage *message,
                      ProtobufCAllocator *allocator);
/* ConsumerID methods */
void   consumer_id__init
                     (ConsumerID         *message);
size_t consumer_id__get_packed_size
                     (const ConsumerID   *message);
size_t consumer_id__pack
                     (const ConsumerID   *message,
                      uint8_t             *out);
size_t consumer_id__pack_to_buffer
                     (const ConsumerID   *message,
                      ProtobufCBuffer     *buffer);
ConsumerID *
       consumer_id__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   consumer_id__free_unpacked
                     (ConsumerID *message,
                      ProtobufCAllocator *allocator);
/* CreateQueueMessage methods */
void   create_queue_message__init
                     (CreateQueueMessage         *message);
size_t create_queue_message__get_packed_size
                     (const CreateQueueMessage   *message);
size_t create_queue_message__pack
                     (const CreateQueueMessage   *message,
                      uint8_t             *out);
size_t create_queue_message__pack_to_buffer
                     (const CreateQueueMessage   *message,
                      ProtobufCBuffer     *buffer);
CreateQueueMessage *
       create_queue_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   create_queue_message__free_unpacked
                     (CreateQueueMessage *message,
                      ProtobufCAllocator *allocator);
/* DeleteQueueMessage methods */
void   delete_queue_message__init
                     (DeleteQueueMessage         *message);
size_t delete_queue_message__get_packed_size
                     (const DeleteQueueMessage   *message);
size_t delete_queue_message__pack
                     (const DeleteQueueMessage   *message,
                      uint8_t             *out);
size_t delete_queue_message__pack_to_buffer
                     (const DeleteQueueMessage   *message,
                      ProtobufCBuffer     *buffer);
DeleteQueueMessage *
       delete_queue_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   delete_queue_message__free_unpacked
                     (DeleteQueueMessage *message,
                      ProtobufCAllocator *allocator);
/* SubscribeQueueMessage methods */
void   subscribe_queue_message__init
                     (SubscribeQueueMessage         *message);
size_t subscribe_queue_message__get_packed_size
                     (const SubscribeQueueMessage   *message);
size_t subscribe_queue_message__pack
                     (const SubscribeQueueMessage   *message,
                      uint8_t             *out);
size_t subscribe_queue_message__pack_to_buffer
                     (const SubscribeQueueMessage   *message,
                      ProtobufCBuffer     *buffer);
SubscribeQueueMessage *
       subscribe_queue_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   subscribe_queue_message__free_unpacked
                     (SubscribeQueueMessage *message,
                      ProtobufCAllocator *allocator);
/* UnsubscribeQueueMessage methods */
void   unsubscribe_queue_message__init
                     (UnsubscribeQueueMessage         *message);
size_t unsubscribe_queue_message__get_packed_size
                     (const UnsubscribeQueueMessage   *message);
size_t unsubscribe_queue_message__pack
                     (const UnsubscribeQueueMessage   *message,
                      uint8_t             *out);
size_t unsubscribe_queue_message__pack_to_buffer
                     (const UnsubscribeQueueMessage   *message,
                      ProtobufCBuffer     *buffer);
UnsubscribeQueueMessage *
       unsubscribe_queue_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   unsubscribe_queue_message__free_unpacked
                     (UnsubscribeQueueMessage *message,
                      ProtobufCAllocator *allocator);
/* EnqueueMessage methods */
void   enqueue_message__init
                     (EnqueueMessage         *message);
size_t enqueue_message__get_packed_size
                     (const EnqueueMessage   *message);
size_t enqueue_message__pack
                     (const EnqueueMessage   *message,
                      uint8_t             *out);
size_t enqueue_message__pack_to_buffer
                     (const EnqueueMessage   *message,
                      ProtobufCBuffer     *buffer);
EnqueueMessage *
       enqueue_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   enqueue_message__free_unpacked
                     (EnqueueMessage *message,
                      ProtobufCAllocator *allocator);
/* EnqueueResponseMessage methods */
void   enqueue_response_message__init
                     (EnqueueResponseMessage         *message);
size_t enqueue_response_message__get_packed_size
                     (const EnqueueResponseMessage   *message);
size_t enqueue_response_message__pack
                     (const EnqueueResponseMessage   *message,
                      uint8_t             *out);
size_t enqueue_response_message__pack_to_buffer
                     (const EnqueueResponseMessage   *message,
                      ProtobufCBuffer     *buffer);
EnqueueResponseMessage *
       enqueue_response_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   enqueue_response_message__free_unpacked
                     (EnqueueResponseMessage *message,
                      ProtobufCAllocator *allocator);
/* ReadQueueMessage methods */
void   read_queue_message__init
                     (ReadQueueMessage         *message);
size_t read_queue_message__get_packed_size
                     (const ReadQueueMessage   *message);
size_t read_queue_message__pack
                     (const ReadQueueMessage   *message,
                      uint8_t             *out);
size_t read_queue_message__pack_to_buffer
                     (const ReadQueueMessage   *message,
                      ProtobufCBuffer     *buffer);
ReadQueueMessage *
       read_queue_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   read_queue_message__free_unpacked
                     (ReadQueueMessage *message,
                      ProtobufCAllocator *allocator);
/* ReadQueueResponseMessage methods */
void   read_queue_response_message__init
                     (ReadQueueResponseMessage         *message);
size_t read_queue_response_message__get_packed_size
                     (const ReadQueueResponseMessage   *message);
size_t read_queue_response_message__pack
                     (const ReadQueueResponseMessage   *message,
                      uint8_t             *out);
size_t read_queue_response_message__pack_to_buffer
                     (const ReadQueueResponseMessage   *message,
                      ProtobufCBuffer     *buffer);
ReadQueueResponseMessage *
       read_queue_response_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   read_queue_response_message__free_unpacked
                     (ReadQueueResponseMessage *message,
                      ProtobufCAllocator *allocator);
/* ConsumeQueueMessage methods */
void   consume_queue_message__init
                     (ConsumeQueueMessage         *message);
size_t consume_queue_message__get_packed_size
                     (const ConsumeQueueMessage   *message);
size_t consume_queue_message__pack
                     (const ConsumeQueueMessage   *message,
                      uint8_t             *out);
size_t consume_queue_message__pack_to_buffer
                     (const ConsumeQueueMessage   *message,
                      ProtobufCBuffer     *buffer);
ConsumeQueueMessage *
       consume_queue_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   consume_queue_message__free_unpacked
                     (ConsumeQueueMessage *message,
                      ProtobufCAllocator *allocator);
/* ConsumeQueueResponseMessage methods */
void   consume_queue_response_message__init
                     (ConsumeQueueResponseMessage         *message);
size_t consume_queue_response_message__get_packed_size
                     (const ConsumeQueueResponseMessage   *message);
size_t consume_queue_response_message__pack
                     (const ConsumeQueueResponseMessage   *message,
                      uint8_t             *out);
size_t consume_queue_response_message__pack_to_buffer
                     (const ConsumeQueueResponseMessage   *message,
                      ProtobufCBuffer     *buffer);
ConsumeQueueResponseMessage *
       consume_queue_response_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   consume_queue_response_message__free_unpacked
                     (ConsumeQueueResponseMessage *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*NodeStateMessage_Closure)
                 (const NodeStateMessage *message,
                  void *closure_data);
typedef void (*VectorClockMessage_Closure)
                 (const VectorClockMessage *message,
                  void *closure_data);
typedef void (*GossipMessage_Closure)
                 (const GossipMessage *message,
                  void *closure_data);
typedef void (*MembershipViewMessage_Closure)
                 (const MembershipViewMessage *message,
                  void *closure_data);
typedef void (*MembershipAgreementMessage_Closure)
                 (const MembershipAgreementMessage *message,
                  void *closure_data);
typedef void (*CellAddressMessage_Closure)
                 (const CellAddressMessage *message,
                  void *closure_data);
typedef void (*CellMessage_Closure)
                 (const CellMessage *message,
                  void *closure_data);
typedef void (*VersionedCellMessage_Closure)
                 (const VersionedCellMessage *message,
                  void *closure_data);
typedef void (*WriteQueryMessage_Closure)
                 (const WriteQueryMessage *message,
                  void *closure_data);
typedef void (*ReadQueryMessage_Closure)
                 (const ReadQueryMessage *message,
                  void *closure_data);
typedef void (*AckMessage_Closure)
                 (const AckMessage *message,
                  void *closure_data);
typedef void (*RangeReadQueryMessage_Closure)
                 (const RangeReadQueryMessage *message,
                  void *closure_data);
typedef void (*RangeReadResponseMessage_Closure)
                 (const RangeReadResponseMessage *message,
                  void *closure_data);
typedef void (*TxnMessage_Closure)
                 (const TxnMessage *message,
                  void *closure_data);
typedef void (*QueueQueryMessage_Closure)
                 (const QueueQueryMessage *message,
                  void *closure_data);
typedef void (*ConsumerID_Closure)
                 (const ConsumerID *message,
                  void *closure_data);
typedef void (*CreateQueueMessage_Closure)
                 (const CreateQueueMessage *message,
                  void *closure_data);
typedef void (*DeleteQueueMessage_Closure)
                 (const DeleteQueueMessage *message,
                  void *closure_data);
typedef void (*SubscribeQueueMessage_Closure)
                 (const SubscribeQueueMessage *message,
                  void *closure_data);
typedef void (*UnsubscribeQueueMessage_Closure)
                 (const UnsubscribeQueueMessage *message,
                  void *closure_data);
typedef void (*EnqueueMessage_Closure)
                 (const EnqueueMessage *message,
                  void *closure_data);
typedef void (*EnqueueResponseMessage_Closure)
                 (const EnqueueResponseMessage *message,
                  void *closure_data);
typedef void (*ReadQueueMessage_Closure)
                 (const ReadQueueMessage *message,
                  void *closure_data);
typedef void (*ReadQueueResponseMessage_Closure)
                 (const ReadQueueResponseMessage *message,
                  void *closure_data);
typedef void (*ConsumeQueueMessage_Closure)
                 (const ConsumeQueueMessage *message,
                  void *closure_data);
typedef void (*ConsumeQueueResponseMessage_Closure)
                 (const ConsumeQueueResponseMessage *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor node_state_message__descriptor;
extern const ProtobufCMessageDescriptor vector_clock_message__descriptor;
extern const ProtobufCMessageDescriptor gossip_message__descriptor;
extern const ProtobufCMessageDescriptor membership_view_message__descriptor;
extern const ProtobufCMessageDescriptor membership_agreement_message__descriptor;
extern const ProtobufCMessageDescriptor cell_address_message__descriptor;
extern const ProtobufCMessageDescriptor cell_message__descriptor;
extern const ProtobufCMessageDescriptor versioned_cell_message__descriptor;
extern const ProtobufCMessageDescriptor write_query_message__descriptor;
extern const ProtobufCMessageDescriptor read_query_message__descriptor;
extern const ProtobufCMessageDescriptor ack_message__descriptor;
extern const ProtobufCMessageDescriptor range_read_query_message__descriptor;
extern const ProtobufCMessageDescriptor range_read_response_message__descriptor;
extern const ProtobufCMessageDescriptor txn_message__descriptor;
extern const ProtobufCMessageDescriptor queue_query_message__descriptor;
extern const ProtobufCMessageDescriptor consumer_id__descriptor;
extern const ProtobufCMessageDescriptor create_queue_message__descriptor;
extern const ProtobufCMessageDescriptor delete_queue_message__descriptor;
extern const ProtobufCMessageDescriptor subscribe_queue_message__descriptor;
extern const ProtobufCMessageDescriptor unsubscribe_queue_message__descriptor;
extern const ProtobufCMessageDescriptor enqueue_message__descriptor;
extern const ProtobufCMessageDescriptor enqueue_response_message__descriptor;
extern const ProtobufCMessageDescriptor read_queue_message__descriptor;
extern const ProtobufCMessageDescriptor read_queue_response_message__descriptor;
extern const ProtobufCMessageDescriptor consume_queue_message__descriptor;
extern const ProtobufCMessageDescriptor consume_queue_response_message__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_db_5fmessages_2eproto__INCLUDED */
