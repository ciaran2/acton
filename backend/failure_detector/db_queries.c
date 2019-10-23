/*
 * db_queries.c
 *
 *      Author: aagapi
 */

#include "db_queries.h"
#include "db_messages.pb-c.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

// Remote DB API:

remote_db_t * get_remote_db()
{
	remote_db_t * db = (remote_db_t *) malloc(sizeof(remote_db_t));

	db->servers = create_skiplist_long();
	db->txn_state = create_skiplist_uuid();

	return db;
}

int free_remote_db(remote_db_t * db)
{
	skiplist_free(db->servers);
	skiplist_free(db->txn_state);
	free(db);
	return 0;
}


// Write Query:

write_query * init_write_query(cell * cell, int msg_type, uuid_t * txnid, long nonce)
{
	write_query * ca = (write_query *) malloc(sizeof(write_query));
	ca->cell = cell;
	ca->msg_type = msg_type;
	ca->txnid = txnid;
	ca->nonce = nonce;
	return ca;
}

write_query * init_write_query_copy(cell * cell, int msg_type, uuid_t * txnid, long nonce)
{
	write_query * ca = (write_query *) malloc(sizeof(write_query));
	ca->cell = (cell != NULL)?(init_cell_copy(cell->table_key, cell->keys, cell->no_keys, cell->columns, cell->no_columns, cell->version)):(NULL);
	ca->msg_type = msg_type;
	if(txnid != NULL)
	{
		ca->txnid = malloc(sizeof(uuid_t));
		memcpy(ca->txnid, txnid, sizeof(uuid_t));
	}
	else
	{
		ca->txnid = NULL;
	}
	ca->nonce = nonce;
	return ca;
}

write_query * build_insert_in_txn(WORD * column_values, int no_cols, int no_primary_keys, int no_clustering_keys, WORD table_key, uuid_t * txnid, long nonce)
{
	int no_keys = no_primary_keys + no_clustering_keys;
	assert(no_cols - no_keys > 0);
	cell * c = init_cell((long) table_key, (long *) column_values, no_keys, ((long *) column_values + no_keys), no_cols - no_keys, NULL);
	return init_write_query_copy(c, RPC_TYPE_WRITE, txnid, nonce);
}

write_query * build_delete_row_in_txn(WORD* primary_keys, int no_primary_keys, WORD table_key, uuid_t * txnid, long nonce)
{
	cell * c = init_cell((long) table_key, (long *) primary_keys, no_primary_keys, NULL, 0, NULL);
	return init_write_query_copy(c, RPC_TYPE_DELETE, txnid, nonce);
}

write_query * build_delete_cell_in_txn(WORD* keys, int no_primary_keys, int no_clustering_keys, WORD table_key, uuid_t * txnid, long nonce)
{
	cell * c = init_cell((long) table_key, (long *) keys, no_primary_keys + no_clustering_keys, NULL, 0, NULL);
	return init_write_query_copy(c, RPC_TYPE_DELETE, txnid, nonce);
}

write_query * build_delete_by_index_in_txn(WORD index_key, int idx_idx, WORD table_key, uuid_t * txnid, long nonce)
{
	assert (0); // Not supported
	return 0;
}

write_query * build_update_in_txn(int * col_idxs, int no_cols, WORD * column_values, WORD table_key, uuid_t * txnid, long nonce)
{
	assert (0); // Not supported
	return 0;
}

void free_write_query(write_query * ca)
{
	free_cell(ca->cell);
	free(ca);
}

void init_write_query_msg(WriteQueryMessage * msg, write_query * ca, VersionedCellMessage * vcell_msg)
{
	if(ca->txnid != NULL)
	{
		msg->txnid.len = sizeof(uuid_t);
		msg->txnid.data = malloc(sizeof(uuid_t));
		memcpy(msg->txnid.data, ca->txnid, sizeof(uuid_t));
	}
	else
	{
		msg->txnid.data = NULL;
		msg->txnid.len = 0;
	}
	msg->nonce = ca->nonce;
	msg->cell = vcell_msg;
	msg->msg_type = ca->msg_type;
}

write_query * init_write_query_from_msg(WriteQueryMessage * msg)
{
	cell * cell = init_cell_from_msg(msg->cell);
	write_query * c = init_write_query_copy(cell, msg->msg_type, (uuid_t *) msg->txnid.data, msg->nonce);
	return c;
}

void free_write_query_msg(WriteQueryMessage * msg)
{
//	if(msg->txnid.data != NULL)
//		free(msg->txnid.data);
	if(msg->cell != NULL)
		free_cell_msg(msg->cell);
}

int serialize_write_query(write_query * ca, void ** buf, unsigned * len)
{
	WriteQueryMessage msg = WRITE_QUERY_MESSAGE__INIT;
	VersionedCellMessage vcell_msg = VERSIONED_CELL_MESSAGE__INIT;
	VectorClockMessage vc_msg = VECTOR_CLOCK_MESSAGE__INIT;

	if(ca->cell != NULL)
		init_cell_msg(&vcell_msg, ca->cell, &vc_msg);
	init_write_query_msg(&msg, ca, (ca->cell != NULL)?(&vcell_msg):(NULL));
	msg.mtype = RPC_TYPE_WRITE;

	*len = write_query_message__get_packed_size (&msg);
	*buf = malloc (*len);
	write_query_message__pack (&msg, *buf);

	free_write_query_msg(&msg);

	return 0;
}

int deserialize_write_query(void * buf, unsigned msg_len, write_query ** ca)
{
	WriteQueryMessage * msg = write_query_message__unpack (NULL, msg_len, buf);

	if (msg == NULL || msg->mtype != RPC_TYPE_WRITE)
	{
		fprintf(stderr, "error unpacking write query message\n");
	    return 1;
	}

	*ca = init_write_query_from_msg(msg);

	write_query_message__free_unpacked(msg, NULL);

	return 0;
}

char * to_string_write_query(write_query * ca, char * msg_buff)
{
	char * crt_ptr = msg_buff;
	char uuid_str[37];
	if(ca->txnid != NULL)
		uuid_unparse_lower(*(ca->txnid), uuid_str);
	else
		uuid_str[0]='\0';

	sprintf(crt_ptr, "%s(txnid=%s, nonce=%ld, cell=", (ca->msg_type == RPC_TYPE_WRITE)?("WriteQuery"):("DeleteQuery"), uuid_str, ca->nonce);
	crt_ptr += strlen(crt_ptr);

	if(ca->cell != NULL)
	{
		to_string_cell(ca->cell, crt_ptr);
		crt_ptr += strlen(crt_ptr);
	}

	sprintf(crt_ptr, ")");

	return msg_buff;
}

int equals_write_query(write_query * ca1, write_query * ca2)
{
	if(ca1->nonce != ca2->nonce || ca1->msg_type != ca2->msg_type || !equals_cell(ca1->cell, ca2->cell))
		return 0;

	if(ca1->txnid != NULL && ca2->txnid && uuid_compare(*(ca1->txnid), *(ca2->txnid)))
		return 0;

	return 1;
}


// Read Query:

read_query * init_read_query(cell_address * cell_address, uuid_t * txnid, long nonce)
{
	read_query * ca = (read_query *) malloc(sizeof(read_query));
	ca->cell_address = cell_address;
	ca->txnid = txnid;
	ca->nonce = nonce;
	return ca;
}

read_query * init_read_query_copy(cell_address * cell_address, uuid_t * txnid, long nonce)
{
	read_query * ca = (read_query *) malloc(sizeof(read_query));
	ca->cell_address = init_cell_address_copy(cell_address->table_key, cell_address->keys, cell_address->no_keys);
	if(txnid != NULL)
	{
		ca->txnid = malloc(sizeof(uuid_t));
		memcpy(ca->txnid, txnid, sizeof(uuid_t));
	}
	else
	{
		ca->txnid = NULL;
	}
	ca->nonce = nonce;
	return ca;
}

read_query * build_search_in_txn(WORD* primary_keys, int no_primary_keys, WORD table_key, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_copy((long) table_key, (long *) primary_keys, no_primary_keys);

	return init_read_query_copy(c, txnid, nonce);
}

read_query * build_search_clustering_in_txn(WORD* primary_keys, int no_primary_keys, WORD* clustering_keys, int no_clustering_keys, WORD table_key, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_copy2((long) table_key, (long *) primary_keys, no_primary_keys, (long *) clustering_keys, no_clustering_keys);

	return init_read_query_copy(c, txnid, nonce);
}

read_query * build_search_columns_in_txn(WORD* primary_keys, int no_primary_keys, WORD* clustering_keys, int no_clustering_keys, WORD* col_keys, int no_columns, WORD table_key, uuid_t * txnid, long nonce)
{
	assert(0); // Not supported
	return NULL;
}

read_query * build_search_index_in_txn(WORD index_key, int idx_idx, WORD table_key, uuid_t * txnid, long nonce)
{
	assert(0); // Not supported
	return NULL;
}


void free_read_query(read_query * ca)
{
	free_cell_address(ca->cell_address);
	free(ca);
}

void init_read_query_msg(ReadQueryMessage * msg, read_query * ca, CellAddressMessage * cell_address_msg)
{
	if(ca->txnid != NULL)
	{
		msg->txnid.len = sizeof(uuid_t);
		msg->txnid.data = malloc(sizeof(uuid_t));
		memcpy(msg->txnid.data, ca->txnid, sizeof(uuid_t));
	}
	else
	{
		msg->txnid.data = NULL;
		msg->txnid.len = 0;
	}
	msg->nonce = ca->nonce;
	msg->cell_address = cell_address_msg;
}

read_query * init_read_query_from_msg(ReadQueryMessage * msg)
{
	cell_address * cell_address = init_cell_address_from_msg(msg->cell_address);
	read_query * c = init_read_query_copy(cell_address, (uuid_t *) msg->txnid.data, msg->nonce);
	return c;
}

void free_read_query_msg(ReadQueryMessage * msg)
{
	free_cell_address_msg(msg->cell_address);
//	if(msg->txnid.data != NULL)
//		free(msg->txnid.data);
}

int serialize_read_query(read_query * ca, void ** buf, unsigned * len)
{
	ReadQueryMessage msg = READ_QUERY_MESSAGE__INIT;
	CellAddressMessage cell_address_msg = CELL_ADDRESS_MESSAGE__INIT;

	init_cell_address_msg(&cell_address_msg, ca->cell_address);
	init_read_query_msg(&msg, ca, &cell_address_msg);
	msg.mtype = RPC_TYPE_READ;

	*len = read_query_message__get_packed_size (&msg);
	*buf = malloc (*len);
	read_query_message__pack (&msg, *buf);

	free_read_query_msg(&msg);

	return 0;
}

int deserialize_read_query(void * buf, unsigned msg_len, read_query ** ca)
{
	ReadQueryMessage * msg = read_query_message__unpack (NULL, msg_len, buf);

	if (msg == NULL || msg->mtype != RPC_TYPE_READ)
	{
		fprintf(stderr, "error unpacking read query message\n");
	    return 1;
	}

	*ca = init_read_query_from_msg(msg);

	read_query_message__free_unpacked(msg, NULL);

	return 0;
}

char * to_string_read_query(read_query * ca, char * msg_buff)
{
	char * crt_ptr = msg_buff;
	char uuid_str[37];
	if(ca->txnid != NULL)
		uuid_unparse_lower(*(ca->txnid), uuid_str);
	else
		uuid_str[0]='\0';

	sprintf(crt_ptr, "ReadQuery(txnid=%s, nonce=%ld, ", uuid_str, ca->nonce);
	crt_ptr += strlen(crt_ptr);

	to_string_cell_address(ca->cell_address, crt_ptr);
	crt_ptr += strlen(crt_ptr);

	sprintf(crt_ptr, ")");

	return msg_buff;
}

int equals_read_query(read_query * ca1, read_query * ca2)
{
	if(ca1->nonce != ca2->nonce || !equals_cell_address(ca1->cell_address, ca2->cell_address))
		return 0;

	if(ca1->txnid != NULL && ca2->txnid && uuid_compare(*(ca1->txnid), *(ca2->txnid)))
		return 0;

	return 1;
}

// Range read query:

range_read_query * build_range_search_in_txn(WORD* start_primary_keys, WORD* end_primary_keys, int no_primary_keys, WORD table_key, uuid_t * txnid, long nonce)
{
	cell_address * start_c = init_cell_address_copy((long) table_key, (long *) start_primary_keys, no_primary_keys);
	cell_address * end_c = init_cell_address_copy((long) table_key, (long *) end_primary_keys, no_primary_keys);

	return init_range_read_query_copy(start_c, end_c, txnid, nonce);
}

range_read_query * build_range_search_clustering_in_txn(WORD* primary_keys, int no_primary_keys, WORD* start_clustering_keys, WORD* end_clustering_keys, int no_clustering_keys, WORD table_key, uuid_t * txnid, long nonce)
{
	cell_address * start_c = init_cell_address_copy2((long) table_key, (long *) primary_keys, no_primary_keys, (long *) start_clustering_keys, no_clustering_keys);
	cell_address * end_c = init_cell_address_copy2((long) table_key, (long *) primary_keys, no_primary_keys, (long *) end_clustering_keys, no_clustering_keys);

	return init_range_read_query_copy(start_c, end_c, txnid, nonce);
}

range_read_query * build_range_search_index_in_txn(int idx_idx, WORD start_idx_key, WORD end_idx_key, WORD table_key, uuid_t * txnid, long nonce)
{
	assert(0); // Not supported
	return NULL;
}

range_read_query * init_range_read_query(cell_address * start_cell_address, cell_address * end_cell_address, uuid_t * txnid, long nonce)
{
	range_read_query * ca = (range_read_query *) malloc(sizeof(range_read_query));
	ca->start_cell_address = start_cell_address;
	ca->end_cell_address = end_cell_address;
	ca->txnid = txnid;
	ca->nonce = nonce;
	return ca;
}

void free_range_read_query(range_read_query * ca)
{
	free_cell_address(ca->start_cell_address);
	free_cell_address(ca->end_cell_address);
	free(ca);
}

range_read_query * init_range_read_query_copy(cell_address * start_cell_address, cell_address * end_cell_address, uuid_t * txnid, long nonce)
{
	range_read_query * ca = (range_read_query *) malloc(sizeof(range_read_query));
	ca->start_cell_address = init_cell_address_copy(start_cell_address->table_key, start_cell_address->keys, start_cell_address->no_keys);
	ca->end_cell_address = init_cell_address_copy(end_cell_address->table_key, end_cell_address->keys, end_cell_address->no_keys);
	if(txnid != NULL)
	{
		ca->txnid = malloc(sizeof(uuid_t));
		memcpy(ca->txnid, txnid, sizeof(uuid_t));
	}
	else
	{
		ca->txnid = NULL;
	}
	ca->nonce = nonce;
	return ca;
}

void init_range_read_query_msg(RangeReadQueryMessage * msg, range_read_query * ca, CellAddressMessage * start_cell_address_msg, CellAddressMessage * end_cell_address_msg)
{
	if(ca->txnid != NULL)
	{
		msg->txnid.len = sizeof(uuid_t);
		msg->txnid.data = malloc(sizeof(uuid_t));
		memcpy(msg->txnid.data, ca->txnid, sizeof(uuid_t));
	}
	else
	{
		msg->txnid.data = NULL;
		msg->txnid.len = 0;
	}
	msg->nonce = ca->nonce;
	msg->start_cell_address = start_cell_address_msg;
	msg->end_cell_address = end_cell_address_msg;
}

range_read_query * init_range_read_query_from_msg(RangeReadQueryMessage * msg)
{
	cell_address * start_cell_address = init_cell_address_from_msg(msg->start_cell_address);
	cell_address * end_cell_address = init_cell_address_from_msg(msg->end_cell_address);
	range_read_query * c = init_range_read_query_copy(start_cell_address, end_cell_address, (uuid_t *) msg->txnid.data, msg->nonce);
	return c;
}

void free_range_read_query_msg(RangeReadQueryMessage * msg)
{
	free_cell_address_msg(msg->start_cell_address);
	free_cell_address_msg(msg->end_cell_address);
//	if(msg->txnid.data != NULL)
//		free(msg->txnid.data);
}

int serialize_range_read_query(range_read_query * ca, void ** buf, unsigned * len)
{
	RangeReadQueryMessage msg = RANGE_READ_QUERY_MESSAGE__INIT;
	CellAddressMessage start_cell_address_msg = CELL_ADDRESS_MESSAGE__INIT;
	CellAddressMessage end_cell_address_msg = CELL_ADDRESS_MESSAGE__INIT;

	init_cell_address_msg(&start_cell_address_msg, ca->start_cell_address);
	init_cell_address_msg(&end_cell_address_msg, ca->end_cell_address);
	init_range_read_query_msg(&msg, ca, &start_cell_address_msg, &end_cell_address_msg);
	msg.mtype = RPC_TYPE_RANGE_READ;

	*len = range_read_query_message__get_packed_size (&msg);
	*buf = malloc (*len);
	range_read_query_message__pack (&msg, *buf);

	free_range_read_query_msg(&msg);

	return 0;
}

int deserialize_range_read_query(void * buf, unsigned msg_len, range_read_query ** ca)
{
	RangeReadQueryMessage * msg = range_read_query_message__unpack (NULL, msg_len, buf);

	if (msg == NULL || msg->mtype != RPC_TYPE_RANGE_READ)
	{
		fprintf(stderr, "error unpacking range read query message\n");
	    return 1;
	}

	*ca = init_range_read_query_from_msg(msg);

	range_read_query_message__free_unpacked(msg, NULL);

	return 0;
}

char * to_string_range_read_query(range_read_query * ca, char * msg_buff)
{
	char * crt_ptr = msg_buff;
	char uuid_str[37];
	if(ca->txnid != NULL)
		uuid_unparse_lower(*(ca->txnid), uuid_str);
	else
		uuid_str[0]='\0';

	sprintf(crt_ptr, "RangeReadQuery(txnid=%s, nonce=%ld, start_key=", uuid_str, ca->nonce);
	crt_ptr += strlen(crt_ptr);

	to_string_cell_address(ca->start_cell_address, crt_ptr);
	crt_ptr += strlen(crt_ptr);

	sprintf(crt_ptr, ", end_key=");
	crt_ptr += strlen(crt_ptr);

	to_string_cell_address(ca->end_cell_address, crt_ptr);
	crt_ptr += strlen(crt_ptr);

	sprintf(crt_ptr, ")");

	return msg_buff;
}

int equals_range_read_query(range_read_query * ca1, range_read_query * ca2)
{
	if(ca1->nonce != ca2->nonce ||
		!equals_cell_address(ca1->start_cell_address, ca2->start_cell_address) ||
		!equals_cell_address(ca1->end_cell_address, ca2->end_cell_address))
		return 0;

	if(ca1->txnid != NULL && ca2->txnid && uuid_compare(*(ca1->txnid), *(ca2->txnid)))
		return 0;

	return 1;
}


// Ack Message:

ack_message * init_ack_message(cell_address * cell_address, int status, uuid_t * txnid, long nonce)
{
	ack_message * ca = (ack_message *) malloc(sizeof(ack_message));
	ca->cell_address = cell_address;
	ca->status = status;
	ca->txnid = txnid;
	ca->nonce = nonce;
	return ca;
}

ack_message * init_ack_message_copy(cell_address * cell_address, int status, uuid_t * txnid, long nonce)
{
	ack_message * ca = (ack_message *) malloc(sizeof(ack_message));
	ca->cell_address = init_cell_address_copy(cell_address->table_key, cell_address->keys, cell_address->no_keys);
	ca->status = status;
	if(txnid != NULL)
	{
		ca->txnid = malloc(sizeof(uuid_t));
		memcpy(ca->txnid, txnid, sizeof(uuid_t));
	}
	else
	{
		ca->txnid = NULL;
	}
	ca->nonce = nonce;
	return ca;
}

void free_ack_message(ack_message * ca)
{
	free_cell_address(ca->cell_address);
	free(ca);
}

void init_ack_message_msg(AckMessage * msg, ack_message * ca, CellAddressMessage * cell_address_msg)
{
	msg->status = ca->status;
	if(ca->txnid != NULL)
	{
		msg->txnid.len = sizeof(uuid_t);
		msg->txnid.data = malloc(sizeof(uuid_t));
		memcpy(msg->txnid.data, ca->txnid, sizeof(uuid_t));
	}
	else
	{
		msg->txnid.data = NULL;
		msg->txnid.len = 0;
	}
	msg->nonce = ca->nonce;
	msg->cell_address = cell_address_msg;
}

ack_message * init_ack_message_from_msg(AckMessage * msg)
{
	cell_address * cell_address = init_cell_address_from_msg(msg->cell_address);
	ack_message * c = init_ack_message_copy(cell_address, msg->status, (uuid_t *) msg->txnid.data, msg->nonce);
	return c;
}

void free_ack_message_msg(AckMessage * msg)
{
	free_cell_address_msg(msg->cell_address);
//	if(msg->txnid.data != NULL)
//		free(msg->txnid.data);
}

int serialize_ack_message(ack_message * ca, void ** buf, unsigned * len)
{
	AckMessage msg = ACK_MESSAGE__INIT;
	CellAddressMessage cell_address_msg = CELL_ADDRESS_MESSAGE__INIT;

	init_cell_address_msg(&cell_address_msg, ca->cell_address);
	init_ack_message_msg(&msg, ca, &cell_address_msg);
	msg.mtype = RPC_TYPE_ACK;

	*len = ack_message__get_packed_size (&msg);
	*buf = malloc (*len);
	ack_message__pack (&msg, *buf);

	free_ack_message_msg(&msg);

	return 0;
}

int deserialize_ack_message(void * buf, unsigned msg_len, ack_message ** ca)
{
	AckMessage * msg = ack_message__unpack (NULL, msg_len, buf);
	char print_buff[100];

	if (msg == NULL || msg->mtype != RPC_TYPE_ACK)
	{
		fprintf(stderr, "error unpacking ack query message\n");
	    return 1;
	}

	*ca = init_ack_message_from_msg(msg);

//	to_string_ack_message(*ca, (char *) print_buff);
//	printf("Received ACK message: %s\n", print_buff);

	ack_message__free_unpacked(msg, NULL);

	return 0;
}

char * to_string_ack_message(ack_message * ca, char * msg_buff)
{
	char * crt_ptr = msg_buff;
	char uuid_str[37];
	if(ca->txnid != NULL)
		uuid_unparse_lower(*(ca->txnid), uuid_str);
	else
		uuid_str[0]='\0';

	sprintf(crt_ptr, "AckMessage(status=%d, txnid=%s, nonce=%ld, ", ca->status, uuid_str, ca->nonce);
	crt_ptr += strlen(crt_ptr);

	to_string_cell_address(ca->cell_address, crt_ptr);
	crt_ptr += strlen(crt_ptr);

	sprintf(crt_ptr, ")");

	return msg_buff;
}

int equals_ack_message(ack_message * ca1, ack_message * ca2)
{
	if(ca1->nonce != ca2->nonce || ca1->status != ca2->status ||
		!equals_cell_address(ca1->cell_address, ca2->cell_address))
		return 0;

	if(ca1->txnid != NULL && ca2->txnid && uuid_compare(*(ca1->txnid), *(ca2->txnid)))
		return 0;

	return 1;
}

// Range read response:

range_read_response_message * init_range_read_response_message(cell * cells, int no_cells, uuid_t * txnid, long nonce)
{
	range_read_response_message * ca = (range_read_response_message *) malloc(sizeof(range_read_response_message));
	ca->cells = cells;
	ca->no_cells = no_cells;
	ca->txnid = txnid;
	ca->nonce = nonce;
	return ca;
}

range_read_response_message * init_range_read_response_message_copy(cell * cells, int no_cells, uuid_t * txnid, long nonce)
{
	range_read_response_message * ca = (range_read_response_message *) malloc(sizeof(range_read_response_message));
	ca->no_cells = no_cells;
	ca->cells = (cell *) malloc(no_cells * sizeof(cell));
	for(int i=0;i<no_cells;i++)
		copy_cell(ca->cells + i, cells[i].table_key, cells[i].keys, cells[i].no_keys, cells[i].columns, cells[i].no_columns, cells[i].version);
	if(txnid != NULL)
	{
		ca->txnid = malloc(sizeof(uuid_t));
		memcpy(ca->txnid, txnid, sizeof(uuid_t));
	}
	else
	{
		ca->txnid = NULL;
	}
	ca->nonce = nonce;
	return ca;
}

void init_range_read_response_message_msg(RangeReadResponseMessage * msg, range_read_response_message * ca)
{
	if(ca->txnid != NULL)
	{
		msg->txnid.len = sizeof(uuid_t);
		msg->txnid.data = malloc(sizeof(uuid_t));
		memcpy(msg->txnid.data, ca->txnid, sizeof(uuid_t));
	}
	else
	{
		msg->txnid.data = NULL;
		msg->txnid.len = 0;
	}
	msg->nonce = ca->nonce;
	msg->n_cells = ca->no_cells;

	msg->cells = (VersionedCellMessage **) malloc(msg->n_cells * sizeof (VersionedCellMessage*));
	VectorClockMessage ** vcs = (VectorClockMessage **) malloc(msg->n_cells * sizeof (VectorClockMessage*));

	for(int i=0;i<ca->no_cells;i++)
	{
		msg->cells[i] = malloc (sizeof (VersionedCellMessage));
	    versioned_cell_message__init(msg->cells[i]);
	    vcs[i] = malloc (sizeof (VectorClockMessage));
	    vector_clock_message__init(vcs[i]);
	    init_cell_msg(msg->cells[i], ca->cells+i, vcs[i]);
	}

	free(vcs);
}

range_read_response_message * init_range_read_response_message_from_msg(RangeReadResponseMessage * msg)
{
	cell * cells = (cell *) malloc(msg->n_cells * sizeof(cell));
	for(int i=0;i<msg->n_cells;i++)
		copy_cell_from_msg(cells + i, msg->cells[i]);

	range_read_response_message * c = init_range_read_response_message_copy(cells, msg->n_cells, (uuid_t *) msg->txnid.data, msg->nonce);
	return c;
}

void free_range_read_response_message_msg(RangeReadResponseMessage * msg)
{
	for(int i=0;i<msg->n_cells;i++)
		free_cell_msg(msg->cells[i]);

	if(msg->cells != NULL)
		free(msg->cells);

//	if(msg->txnid.data != NULL)
//		free(msg->txnid.data);
}

void free_range_read_response_message(range_read_response_message * ca)
{
	for(int i=0;i<ca->no_cells;i++)
		free_cell_ptrs(ca->cells + i);

	if(ca->cells != NULL)
		free(ca->cells);

	free(ca);
}

int serialize_range_read_response_message(range_read_response_message * ca, void ** buf, unsigned * len)
{
	RangeReadResponseMessage msg = RANGE_READ_RESPONSE_MESSAGE__INIT;

	init_range_read_response_message_msg(&msg, ca);
	msg.mtype = RPC_TYPE_RANGE_READ_RESPONSE;

	*len = range_read_response_message__get_packed_size (&msg);
	*buf = malloc (*len);
	range_read_response_message__pack (&msg, *buf);

	free_range_read_response_message_msg(&msg);

	return 0;
}

int deserialize_range_read_response_message(void * buf, unsigned msg_len, range_read_response_message ** ca)
{
	RangeReadResponseMessage * msg = range_read_response_message__unpack (NULL, msg_len, buf);

	if (msg == NULL || msg->mtype != RPC_TYPE_RANGE_READ_RESPONSE)
	{
		fprintf(stderr, "error unpacking range read response message\n");
	    return 1;
	}

	*ca = init_range_read_response_message_from_msg(msg);

	range_read_response_message__free_unpacked(msg, NULL);

	return 0;
}


char * to_string_range_read_response_message(range_read_response_message * ca, char * msg_buff)
{
	char * crt_ptr = msg_buff;
	char uuid_str[37];
	if(ca->txnid != NULL)
		uuid_unparse_lower(*(ca->txnid), uuid_str);
	else
		uuid_str[0]='\0';

	sprintf(crt_ptr, "RangeReadResponseMessage(txnid=%s, nonce=%ld", uuid_str, ca->nonce);
	crt_ptr += strlen(crt_ptr);

	sprintf(crt_ptr, ", cells={");
	crt_ptr += strlen(crt_ptr);
	for(int i=0;i<ca->no_cells;i++)
	{
		to_string_cell(ca->cells+i, crt_ptr);
		crt_ptr += strlen(crt_ptr);
		sprintf(crt_ptr, ", ");
		crt_ptr += strlen(crt_ptr);
	}

	sprintf(crt_ptr, "} )");

	return msg_buff;
}

int equals_range_read_response_message(range_read_response_message * ca1, range_read_response_message * ca2)
{
	if(ca1->nonce != ca2->nonce || ca1->no_cells != ca2->no_cells)
		return 0;

	if(ca1->txnid != NULL && ca2->txnid && uuid_compare(*(ca1->txnid), *(ca2->txnid)))
		return 0;

	for(int i=0;i<ca1->no_cells;i++)
		if(!equals_cell(ca1->cells+i, ca2->cells+i))
			return 0;

	return 1;
}


// Queue query (and response) messages:

queue_query_message * init_query_message_basic(cell_address * cell_address, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = (queue_query_message *) malloc(sizeof(queue_query_message));
	ca->cells = NULL;
	ca->no_cells = 0;
	ca->cell_address = cell_address;
	ca->queue_index = -1;
	ca->app_id = -1;
	ca->shard_id = -1;
	ca->consumer_id = -1;
	ca->status = -1;
	if(txnid != NULL)
	{
		ca->txnid = malloc(sizeof(uuid_t));
		memcpy(ca->txnid, txnid, sizeof(uuid_t));
	}
	else
	{
		ca->txnid = NULL;
	}
	ca->nonce = nonce;
	return ca;
}

queue_query_message * build_enqueue_in_txn(WORD * column_values, int no_cols, WORD table_key, WORD queue_id, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_single_key_copy((long) table_key, (long) queue_id);
	cell * entry = init_cell_copy((long) table_key, (long *) column_values, 0, (long *) column_values, no_cols, NULL);

	return init_enqueue_message(c, entry, 1, txnid, nonce);
}

queue_query_message * build_read_queue_in_txn(WORD consumer_id, WORD shard_id, WORD app_id, WORD table_key, WORD queue_id,
												int max_entries, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_single_key_copy((long) table_key, (long) queue_id);
	return init_read_queue_message(c, (long) app_id, (long) shard_id, (long) consumer_id, (long) max_entries, txnid, nonce);

}

queue_query_message * build_consume_queue_in_txn(WORD consumer_id, WORD shard_id, WORD app_id, WORD table_key, WORD queue_id,
													long new_consume_head, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_single_key_copy((long) table_key, (long) queue_id);
	return init_consume_queue_message(c, (long) app_id, (long) shard_id, (long) consumer_id, new_consume_head, txnid, nonce);
}

queue_query_message * build_create_queue_in_txn(WORD table_key, WORD queue_id, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_single_key_copy((long) table_key, (long) queue_id);
	return init_create_queue_message(c, txnid, nonce);
}

queue_query_message * build_delete_queue_in_txn(WORD table_key, WORD queue_id, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_single_key_copy((long) table_key, (long) queue_id);
	return init_delete_queue_message(c, txnid, nonce);
}

queue_query_message * build_subscribe_queue_in_txn(WORD consumer_id, WORD shard_id, WORD app_id, WORD table_key, WORD queue_id, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_single_key_copy((long) table_key, (long) queue_id);
	return init_subscribe_queue_message(c, (long) app_id, (long) shard_id, (long) consumer_id, txnid, nonce);
}

queue_query_message * build_unsubscribe_queue_in_txn(WORD consumer_id, WORD shard_id, WORD app_id, WORD table_key, WORD queue_id, uuid_t * txnid, long nonce)
{
	cell_address * c = init_cell_address_single_key_copy((long) table_key, (long) queue_id);
	return init_unsubscribe_queue_message(c, (long) app_id, (long) shard_id, (long) consumer_id, txnid, nonce);
}


queue_query_message * init_create_queue_message(cell_address * cell_address, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = init_query_message_basic(cell_address, txnid, nonce);
	ca->msg_type = QUERY_TYPE_CREATE_QUEUE;
	return ca;
}

queue_query_message * init_delete_queue_message(cell_address * cell_address, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = init_query_message_basic(cell_address, txnid, nonce);
	ca->msg_type = QUERY_TYPE_DELETE_QUEUE;
	return ca;
}

queue_query_message * init_subscribe_queue_message(cell_address * cell_address, int app_id, int shard_id, int consumer_id, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = init_query_message_basic(cell_address, txnid, nonce);
	ca->msg_type = QUERY_TYPE_SUBSCRIBE_QUEUE;
	ca->app_id = app_id;
	ca->shard_id = shard_id;
	ca->consumer_id = consumer_id;
	return ca;
}

queue_query_message * init_unsubscribe_queue_message(cell_address * cell_address, int app_id, int shard_id, int consumer_id, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = init_query_message_basic(cell_address, txnid, nonce);
	ca->msg_type = QUERY_TYPE_UNSUBSCRIBE_QUEUE;
	ca->app_id = app_id;
	ca->shard_id = shard_id;
	ca->consumer_id = consumer_id;
	return ca;
}

queue_query_message * init_enqueue_message(cell_address * cell_address, cell * cells, int no_cells, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = init_query_message_basic(cell_address, txnid, nonce);
	ca->msg_type = QUERY_TYPE_ENQUEUE;
	ca->cells = cells;
	ca->no_cells = no_cells;
	return ca;
}

queue_query_message * init_read_queue_message(cell_address * cell_address, int app_id, int shard_id, int consumer_id, long max_entries, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = init_query_message_basic(cell_address, txnid, nonce);
	ca->msg_type = QUERY_TYPE_READ_QUEUE;
	ca->queue_index = max_entries;
	ca->app_id = app_id;
	ca->shard_id = shard_id;
	ca->consumer_id = consumer_id;
	return ca;
}

queue_query_message * init_consume_queue_message(cell_address * cell_address, int app_id, int shard_id, int consumer_id, long new_consume_head, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = init_query_message_basic(cell_address, txnid, nonce);
	ca->msg_type = QUERY_TYPE_CONSUME_QUEUE;
	ca->queue_index = new_consume_head;
	ca->app_id = app_id;
	ca->shard_id = shard_id;
	ca->consumer_id = consumer_id;
	return ca;
}


queue_query_message * init_read_queue_response(cell_address * cell_address, cell * cells, int no_cells, int app_id, int shard_id, int consumer_id, long new_read_head, short status, uuid_t * txnid, long nonce)
{
	queue_query_message * ca = init_query_message_basic(cell_address, txnid, nonce);
	ca->msg_type = QUERY_TYPE_READ_QUEUE_RESPONSE;
	ca->queue_index = new_read_head;
	ca->app_id = app_id;
	ca->shard_id = shard_id;
	ca->consumer_id = consumer_id;
	ca->cells = cells;
	ca->no_cells = no_cells;
	return ca;

}


void free_queue_message(queue_query_message * ca)
{
	for(int i=0;i<ca->no_cells;i++)
		free_cell_ptrs(ca->cells + i);

	if(ca->cells != NULL)
		free(ca->cells);

	free(ca);
}

void init_queue_message_msg(QueueQueryMessage * msg, queue_query_message * ca, CellAddressMessage * cell_address_msg)
{
	VectorClockMessage ** vcs = NULL;

	msg->msg_type = ca->msg_type;
	if(ca->txnid != NULL)
	{
		msg->txnid.len = sizeof(uuid_t);
		msg->txnid.data = malloc(sizeof(uuid_t));
		memcpy(msg->txnid.data, ca->txnid, sizeof(uuid_t));
	}
	else
	{
		msg->txnid.data = NULL;
		msg->txnid.len = 0;
	}
	msg->nonce = ca->nonce;
	msg->n_cells = ca->no_cells;

	msg->queue_address = cell_address_msg;

	msg->app_id = ca->app_id;
	msg->shard_id = ca->shard_id;
	msg->consumer_id = ca->consumer_id;
	msg->queue_index = ca->queue_index;

	if(ca->no_cells > 0)
	{
		msg->cells = (VersionedCellMessage **) malloc(msg->n_cells * sizeof (VersionedCellMessage*));
		vcs = (VectorClockMessage **) malloc(msg->n_cells * sizeof (VectorClockMessage*));

		for(int i=0;i<ca->no_cells;i++)
		{
			msg->cells[i] = malloc (sizeof (VersionedCellMessage));
			versioned_cell_message__init(msg->cells[i]);
			vcs[i] = malloc (sizeof (VectorClockMessage));
			vector_clock_message__init(vcs[i]);
			init_cell_msg(msg->cells[i], ca->cells+i, vcs[i]);
		}

		free(vcs);
	}
}

queue_query_message * init_queue_message_from_msg(QueueQueryMessage * msg)
{
	cell * cells = NULL;
	cell_address * cell_address = init_cell_address_from_msg(msg->queue_address);

	switch(msg->msg_type)
	{
		case QUERY_TYPE_CREATE_QUEUE:
		{
			return init_create_queue_message(cell_address, (uuid_t *) msg->txnid.data, msg->nonce);
		}
		case QUERY_TYPE_DELETE_QUEUE:
		{
			return init_delete_queue_message(cell_address, (uuid_t *) msg->txnid.data, msg->nonce);
		}
		case QUERY_TYPE_SUBSCRIBE_QUEUE:
		{
			return init_subscribe_queue_message(cell_address, msg->app_id, msg->shard_id, msg->consumer_id, (uuid_t *) msg->txnid.data, msg->nonce);
		}
		case QUERY_TYPE_UNSUBSCRIBE_QUEUE:
		{
			return init_unsubscribe_queue_message(cell_address, msg->app_id, msg->shard_id, msg->consumer_id, (uuid_t *) msg->txnid.data, msg->nonce);
		}
		case QUERY_TYPE_ENQUEUE:
		{
			if(msg->n_cells > 0)
			{
				cells = (cell *) malloc(msg->n_cells * sizeof(cell));
				for(int i=0;i<msg->n_cells;i++)
					copy_cell_from_msg(cells + i, msg->cells[i]);
			}

			return init_enqueue_message(cell_address, cells, msg->n_cells, (uuid_t *) msg->txnid.data, msg->nonce);
		}
		case QUERY_TYPE_READ_QUEUE:
		{
			return init_read_queue_message(cell_address, msg->app_id, msg->shard_id, msg->consumer_id, msg->queue_index, (uuid_t *) msg->txnid.data, msg->nonce);
		}
		case QUERY_TYPE_CONSUME_QUEUE:
		{
			return init_consume_queue_message(cell_address, msg->app_id, msg->shard_id, msg->consumer_id, msg->queue_index, (uuid_t *) msg->txnid.data, msg->nonce);
		}
		case QUERY_TYPE_READ_QUEUE_RESPONSE:
		{
			if(msg->n_cells > 0)
			{
				cells = (cell *) malloc(msg->n_cells * sizeof(cell));
				for(int i=0;i<msg->n_cells;i++)
					copy_cell_from_msg(cells + i, msg->cells[i]);
			}

			return init_read_queue_response(cell_address, cells, msg->n_cells, msg->app_id, msg->shard_id, msg->consumer_id, msg->queue_index, msg->status, (uuid_t *) msg->txnid.data, msg->nonce);
		}
	}

	return NULL;
}

void free_queue_message_msg(QueueQueryMessage * msg)
{
//	if(msg->txnid.data != NULL)
//		free(msg->txnid.data);

	for(int i=0;i<msg->n_cells;i++)
		free_cell_msg(msg->cells[i]);

	if(msg->cells != NULL)
		free(msg->cells);
}


int serialize_queue_message(queue_query_message * ca, void ** buf, unsigned * len)
{
	QueueQueryMessage msg = QUEUE_QUERY_MESSAGE__INIT;
	CellAddressMessage cell_address_msg = CELL_ADDRESS_MESSAGE__INIT;

	init_cell_address_msg(&cell_address_msg, ca->cell_address);
	init_queue_message_msg(&msg, ca, &cell_address_msg);
	msg.mtype = RPC_TYPE_QUEUE;

	*len = queue_query_message__get_packed_size (&msg);
	*buf = malloc (*len);
	queue_query_message__pack (&msg, *buf);

	free_queue_message_msg(&msg);

	return 0;

}

int deserialize_queue_message(void * buf, unsigned msg_len, queue_query_message ** ca)
{
	QueueQueryMessage * msg = queue_query_message__unpack (NULL, msg_len, buf);

	if (msg == NULL || msg->mtype != RPC_TYPE_QUEUE)
	{
		fprintf(stderr, "error unpacking queue query message\n");
	    return 1;
	}

	*ca = init_queue_message_from_msg(msg);

	queue_query_message__free_unpacked(msg, NULL);

	return 0;
}

char * to_string_queue_message(queue_query_message * ca, char * msg_buff)
{
	char * crt_ptr = msg_buff;
	char uuid_str[37];
	if(ca->txnid != NULL)
		uuid_unparse_lower(*(ca->txnid), uuid_str);
	else
		uuid_str[0]='\0';

	switch(ca->msg_type)
	{
		case QUERY_TYPE_CREATE_QUEUE:
		{
			sprintf(crt_ptr, "CreateQueue(txnid=%s, nonce=%ld, ", uuid_str, ca->nonce);
			break;
		}
		case QUERY_TYPE_DELETE_QUEUE:
		{
			sprintf(crt_ptr, "DeleteQueue(txnid=%s, nonce=%ld, ", uuid_str, ca->nonce);
			break;
		}
		case QUERY_TYPE_SUBSCRIBE_QUEUE:
		{
			sprintf(crt_ptr, "SubscribeQueue(txnid=%s, nonce=%ld, app_id=%d, shard_id=%d, consumer_id=%d, ", uuid_str, ca->nonce, ca->app_id, ca->shard_id, ca->consumer_id);
			break;
		}
		case QUERY_TYPE_UNSUBSCRIBE_QUEUE:
		{
			sprintf(crt_ptr, "UnsubscribeQueue(txnid=%s, nonce=%ld, app_id=%d, shard_id=%d, consumer_id=%d, ", uuid_str, ca->nonce, ca->app_id, ca->shard_id, ca->consumer_id);
			break;
		}
		case QUERY_TYPE_ENQUEUE:
		{
			sprintf(crt_ptr, "Enqueue(txnid=%s, nonce=%ld, no_entries=%d, ", uuid_str, ca->nonce, ca->no_cells);
			break;
		}
		case QUERY_TYPE_READ_QUEUE:
		{
			sprintf(crt_ptr, "ReadQueue(txnid=%s, nonce=%ld, app_id=%d, shard_id=%d, consumer_id=%d, max_items=%ld, ", uuid_str, ca->nonce, ca->app_id, ca->shard_id, ca->consumer_id, ca->queue_index);
			break;
		}
		case QUERY_TYPE_CONSUME_QUEUE:
		{
			sprintf(crt_ptr, "ConsumeQueue(txnid=%s, nonce=%ld, app_id=%d, shard_id=%d, consumer_id=%d, new_consume_head=%ld, ", uuid_str, ca->nonce, ca->app_id, ca->shard_id, ca->consumer_id, ca->queue_index);
			break;
		}
		case QUERY_TYPE_READ_QUEUE_RESPONSE:
		{
			sprintf(crt_ptr, "ReadQueueResponse(txnid=%s, nonce=%ld, app_id=%d, shard_id=%d, consumer_id=%d, no_entries=%d, new_read_head=%ld, ", uuid_str, ca->nonce, ca->app_id, ca->shard_id, ca->consumer_id, ca->no_cells, ca->queue_index);
			break;
		}
	}
	crt_ptr += strlen(crt_ptr);

	to_string_cell_address(ca->cell_address, crt_ptr);
	crt_ptr += strlen(crt_ptr);

	if(ca->no_cells > 0)
	{
		sprintf(crt_ptr, ", cells={");
		crt_ptr += strlen(crt_ptr);
		for(int i=0;i<ca->no_cells;i++)
		{
			to_string_cell(ca->cells+i, crt_ptr);
			crt_ptr += strlen(crt_ptr);
			sprintf(crt_ptr, ", ");
			crt_ptr += strlen(crt_ptr);
		}

		sprintf(crt_ptr, "} )");
		crt_ptr += strlen(crt_ptr);
	}

	sprintf(crt_ptr, ")");

	return msg_buff;
}

int equals_queue_message(queue_query_message * ca1, queue_query_message * ca2)
{
	if(ca1->nonce != ca2->nonce ||
		ca1->msg_type != ca2->msg_type || ca1->queue_index != ca2->queue_index ||
		ca1->no_cells != ca2->no_cells || !equals_cell_address(ca1->cell_address, ca2->cell_address))
		return 0;

	if(ca1->txnid != NULL && ca2->txnid && uuid_compare(*(ca1->txnid), *(ca2->txnid)))
		return 0;

	return 1;
}

// Txn Message:

txn_state * get_client_txn_state(uuid_t * txnid, remote_db_t * db)
{
	snode_t * txn_node = (snode_t *) skiplist_search(db->txn_state, (WORD) txnid);

	return (txn_node != NULL)? (txn_state *) txn_node->value : NULL;
}

uuid_t * new_client_txn(remote_db_t * db, unsigned int * seedptr)
{
	txn_state * ts = NULL, * previous = NULL;

	while(ts == NULL)
	{
		ts = init_txn_state();
		previous = get_client_txn_state(&(ts->txnid), db);
		if(previous != NULL)
		{
			free_txn_state(ts);
			ts = NULL;
		}
	}

	skiplist_insert(db->txn_state, (WORD) &(ts->txnid), (WORD) ts, seedptr);

	return &(ts->txnid);
}

txn_message * build_new_txn(uuid_t * txnid, long nonce)
{
	return init_txn_message_copy(DB_TXN_BEGIN,
			NULL, 0, // own_read_set, no_own_read_set,
			NULL, 0, // own_write_set, no_own_write_set,
			NULL, 0, // complete_read_set, no_complete_read_set,
			NULL, 0, // complete_write_set, no_complete_write_set,
			txnid, NULL, nonce);
}

txn_message * build_validate_txn(uuid_t * txnid, vector_clock * version, long nonce)
{
	// In the current implementation, the server mirrors the txn's complete read and write sets, so there is no reason to re-send them from client:

	return init_txn_message_copy(DB_TXN_VALIDATION,
			NULL, 0, // own_read_set, no_own_read_set,
			NULL, 0, // own_write_set, no_own_write_set,
			NULL, 0, // complete_read_set, no_complete_read_set,
			NULL, 0, // complete_write_set, no_complete_write_set,
			txnid, version, nonce);
}

txn_message * build_commit_txn(uuid_t * txnid, vector_clock * version, long nonce)
{
	// In the current implementation, the server mirrors the txn's complete read and write sets, so there is no reason to re-send them from client:

	return init_txn_message_copy(DB_TXN_COMMIT,
			NULL, 0, // own_read_set, no_own_read_set,
			NULL, 0, // own_write_set, no_own_write_set,
			NULL, 0, // complete_read_set, no_complete_read_set,
			NULL, 0, // complete_write_set, no_complete_write_set,
			txnid, version, nonce);
}

txn_message * build_abort_txn(uuid_t * txnid, long nonce)
{
	return init_txn_message_copy(DB_TXN_ABORT,
			NULL, 0, // own_read_set, no_own_read_set,
			NULL, 0, // own_write_set, no_own_write_set,
			NULL, 0, // complete_read_set, no_complete_read_set,
			NULL, 0, // complete_write_set, no_complete_write_set,
			txnid, NULL, nonce);
}

txn_message * init_txn_message(int type,
		cell * own_read_set, int no_own_read_set,
		cell * own_write_set, int no_own_write_set,
		cell * complete_read_set, int no_complete_read_set,
		cell * complete_write_set, int no_complete_write_set,
		uuid_t * txnid, vector_clock * version, long nonce)
{
	txn_message * ca = (txn_message *) malloc(sizeof(txn_message));
	ca->type = type;
	ca->own_read_set = own_read_set;
	ca->no_own_read_set = no_own_read_set;
	ca->own_write_set = own_write_set;
	ca->no_own_write_set = no_own_write_set;
	ca->complete_read_set = complete_read_set;
	ca->no_complete_read_set = no_complete_read_set;
	ca->complete_write_set = complete_write_set;
	ca->no_complete_write_set = no_complete_write_set;
	ca->txnid = txnid;
	ca->version = version;
	ca->nonce = nonce;
	return ca;
}

txn_message * init_txn_message_copy(int type,
		cell * own_read_set, int no_own_read_set,
		cell * own_write_set, int no_own_write_set,
		cell * complete_read_set, int no_complete_read_set,
		cell * complete_write_set, int no_complete_write_set,
		uuid_t * txnid, vector_clock * version, long nonce)
{
	txn_message * ca = (txn_message *) malloc(sizeof(txn_message));

	ca->type = type;
	if(txnid != NULL)
	{
		ca->txnid = malloc(sizeof(uuid_t));
		memcpy(ca->txnid, txnid, sizeof(uuid_t));
	}
	else
	{
		ca->txnid = NULL;
	}

	ca->version = (version != NULL)?copy_vc(version):NULL;
	ca->nonce = nonce;
	ca->no_own_read_set = no_own_read_set;
	ca->no_own_write_set = no_own_write_set;
	ca->no_complete_read_set = no_complete_read_set;
	ca->no_complete_write_set = no_complete_write_set;

	ca->own_read_set = (cell *) malloc (no_own_read_set * sizeof(cell));
	for(int i=0;i<no_own_read_set;i++)
		ca->own_read_set[i] = own_read_set[i];

	ca->own_write_set = (cell *) malloc (no_own_write_set * sizeof(cell));
	for(int i=0;i<no_own_write_set;i++)
		ca->own_write_set[i] = own_write_set[i];

	ca->complete_read_set = (cell *) malloc (no_complete_read_set * sizeof(cell));
	for(int i=0;i<no_complete_read_set;i++)
		ca->complete_read_set[i] = complete_read_set[i];

	ca->complete_write_set = (cell *) malloc (no_complete_write_set * sizeof(cell));
	for(int i=0;i<no_complete_write_set;i++)
		ca->complete_write_set[i] = complete_write_set[i];

	return ca;
}

void free_txn_message(txn_message * ca)
{
	for(int i=0;i<ca->no_own_read_set;i++)
		free_cell_ptrs(ca->own_read_set+i);
	free(ca->own_read_set);

	for(int i=0;i<ca->no_own_write_set;i++)
		free_cell_ptrs(ca->own_write_set+i);
	free(ca->own_write_set);

	for(int i=0;i<ca->no_complete_read_set;i++)
		free_cell_ptrs(ca->complete_read_set+i);
	free(ca->complete_read_set);

	for(int i=0;i<ca->no_complete_write_set;i++)
		free_cell_ptrs(ca->complete_write_set+i);
	free(ca->complete_write_set);

	if(ca->version != NULL)
		free_vc(ca->version);

	free(ca);
}

void init_txn_message_msg(TxnMessage * msg, txn_message * ca, VectorClockMessage * vc_msg)
{
	msg->n_own_read_set = ca->no_own_read_set;
	msg->n_own_write_set = ca->no_own_write_set;
	msg->n_complete_read_set = ca->no_complete_read_set;
	msg->n_complete_write_set = ca->no_complete_write_set;

	VersionedCellMessage **own_read_set = (VersionedCellMessage **) malloc(msg->n_own_read_set * sizeof (VersionedCellMessage*));
	VectorClockMessage ** vc_msgs_own_read_set = (VectorClockMessage **) malloc(msg->n_own_read_set * sizeof (VectorClockMessage*));

	for(int i = 0; i < msg->n_own_read_set; i++)
	{
		own_read_set[i] = malloc (sizeof (VersionedCellMessage));
	    versioned_cell_message__init(own_read_set[i]);
	    vc_msgs_own_read_set[i] = malloc (sizeof (VectorClockMessage));
	    vector_clock_message__init(vc_msgs_own_read_set[i]);
	    init_cell_msg(own_read_set[i], ca->own_read_set+i, vc_msgs_own_read_set[i]);
	}
	free(vc_msgs_own_read_set);

	VersionedCellMessage **own_write_set = (VersionedCellMessage **) malloc(msg->n_own_write_set * sizeof (VersionedCellMessage*));
	VectorClockMessage ** vc_msgs_own_write_set = (VectorClockMessage **) malloc(msg->n_own_write_set * sizeof (VectorClockMessage*));

	for(int i = 0; i < msg->n_own_write_set; i++)
	{
		own_write_set[i] = malloc (sizeof (VersionedCellMessage));
	    versioned_cell_message__init(own_write_set[i]);
	    vc_msgs_own_write_set[i] = malloc (sizeof (VectorClockMessage));
	    vector_clock_message__init(vc_msgs_own_write_set[i]);
	    init_cell_msg(own_write_set[i], ca->own_write_set+i, vc_msgs_own_write_set[i]);
	}
	free(vc_msgs_own_write_set);

	VersionedCellMessage **complete_read_set = (VersionedCellMessage **) malloc(msg->n_complete_read_set * sizeof (VersionedCellMessage*));
	VectorClockMessage ** vc_msgs_complete_read_set = (VectorClockMessage **) malloc(msg->n_complete_read_set * sizeof (VectorClockMessage*));

	for(int i = 0; i < msg->n_complete_read_set; i++)
	{
		complete_read_set[i] = malloc (sizeof (VersionedCellMessage));
	    versioned_cell_message__init(complete_read_set[i]);
	    vc_msgs_complete_read_set[i] = malloc (sizeof (VectorClockMessage));
	    vector_clock_message__init(vc_msgs_complete_read_set[i]);
	    init_cell_msg(complete_read_set[i], ca->complete_read_set+i, vc_msgs_complete_read_set[i]);
	}
	free(vc_msgs_complete_read_set);

	VersionedCellMessage **complete_write_set = (VersionedCellMessage **) malloc(msg->n_complete_write_set * sizeof (VersionedCellMessage*));
	VectorClockMessage ** vc_msgs_complete_write_set = (VectorClockMessage **) malloc(msg->n_complete_write_set * sizeof (VectorClockMessage*));

	for(int i = 0; i < msg->n_complete_write_set; i++)
	{
		complete_write_set[i] = malloc (sizeof (VersionedCellMessage));
	    versioned_cell_message__init(complete_write_set[i]);
	    vc_msgs_complete_write_set[i] = malloc (sizeof (VectorClockMessage));
	    vector_clock_message__init(vc_msgs_complete_write_set[i]);
	    init_cell_msg(complete_write_set[i], ca->complete_write_set+i, vc_msgs_complete_write_set[i]);
	}
	free(vc_msgs_complete_write_set);

	msg->own_read_set = own_read_set;
	msg->own_write_set = own_write_set;
	msg->complete_read_set = complete_read_set;
	msg->complete_write_set = complete_write_set;
	msg->type = ca->type;
	if(ca->txnid != NULL)
	{
		msg->txnid.len = sizeof(uuid_t);
		msg->txnid.data = malloc(sizeof(uuid_t));
		memcpy(msg->txnid.data, ca->txnid, sizeof(uuid_t));
	}
	else
	{
		msg->txnid.data = NULL;
		msg->txnid.len = 0;
	}
	if(ca->version != NULL)
	{
		init_vc_msg(vc_msg, ca->version);

//		msg->has_version = 1;
		msg->version = vc_msg;
	}
	else
	{
//		msg->has_version = 0;
	}

	msg->nonce = ca->nonce;
}

txn_message * init_txn_message_from_msg(TxnMessage * msg)
{
	cell * own_read_set = (cell *) malloc(msg->n_own_read_set * sizeof(cell));
	cell * own_write_set = (cell *) malloc(msg->n_own_write_set * sizeof(cell));
	cell * complete_read_set = (cell *) malloc(msg->n_complete_read_set * sizeof(cell));
	cell * complete_write_set = (cell *) malloc(msg->n_complete_write_set * sizeof(cell));

	for(int i=0;i<msg->n_own_read_set;i++)
		copy_cell_from_msg(own_read_set+i, msg->own_read_set[i]);
	for(int i=0;i<msg->n_own_write_set;i++)
		copy_cell_from_msg(own_write_set+i, msg->own_write_set[i]);
	for(int i=0;i<msg->n_complete_read_set;i++)
		copy_cell_from_msg(complete_read_set+i, msg->complete_read_set[i]);
	for(int i=0;i<msg->n_complete_write_set;i++)
		copy_cell_from_msg(complete_write_set+i, msg->complete_write_set[i]);

	txn_message * c = init_txn_message_copy(msg->type,
			own_read_set, msg->n_own_read_set,
			own_write_set, msg->n_own_write_set,
			complete_read_set, msg->n_complete_read_set,
			complete_write_set, msg->n_complete_write_set,
			(uuid_t *) msg->txnid.data, (msg->version != NULL)?init_vc_from_msg(msg->version):NULL, msg->nonce); // msg->has_version

	return c;
}

void free_txn_message_msg(TxnMessage * msg)
{
	for(int i=0;i<msg->n_own_read_set;i++)
		free_cell_msg(msg->own_read_set[i]);
	free(msg->own_read_set);

	for(int i=0;i<msg->n_own_write_set;i++)
		free_cell_msg(msg->own_write_set[i]);
	free(msg->own_write_set);

	for(int i=0;i<msg->n_complete_read_set;i++)
		free_cell_msg(msg->complete_read_set[i]);
	free(msg->complete_read_set);

	for(int i=0;i<msg->n_complete_write_set;i++)
		free_cell_msg(msg->complete_write_set[i]);
	free(msg->complete_write_set);

	if(msg->version != NULL) // msg->has_version
		free_vc_msg(msg->version);
}

int serialize_txn_message(txn_message * ca, void ** buf, unsigned * len)
{
	TxnMessage msg = TXN_MESSAGE__INIT;
	VectorClockMessage vc_msg = VECTOR_CLOCK_MESSAGE__INIT;

	init_txn_message_msg(&msg, ca, &vc_msg);
	msg.mtype = RPC_TYPE_TXN;

	*len = txn_message__get_packed_size (&msg);
	*buf = malloc (*len);
	txn_message__pack (&msg, *buf);

	free_txn_message_msg(&msg);

	return 0;
}

int deserialize_txn_message(void * buf, unsigned msg_len, txn_message ** ca)
{
	TxnMessage * msg = txn_message__unpack (NULL, msg_len, buf);

	if (msg == NULL || msg->mtype != RPC_TYPE_TXN)
	{
		fprintf(stderr, "error unpacking read query message\n");
	    return 1;
	}

	*ca = init_txn_message_from_msg(msg);

	txn_message__free_unpacked(msg, NULL);

	return 0;
}

char * to_string_txn_message(txn_message * ca, char * msg_buff)
{
	char * crt_ptr = msg_buff;
	char uuid_str[37];
	if(ca->txnid != NULL)
		uuid_unparse_lower(*(ca->txnid), uuid_str);
	else
		uuid_str[0]='\0';

	sprintf(crt_ptr, "TxnMessage(type=%d, txnid=%s, nonce=%ld", ca->type, uuid_str, ca->nonce);
	crt_ptr += strlen(crt_ptr);

	sprintf(crt_ptr, ", own_read_set={");
	crt_ptr += strlen(crt_ptr);
	for(int i=0;i<ca->no_own_read_set;i++)
	{
		to_string_cell(ca->own_read_set+i, crt_ptr);
		crt_ptr += strlen(crt_ptr);
		sprintf(crt_ptr, ", ");
		crt_ptr += strlen(crt_ptr);
	}

	sprintf(crt_ptr, "}, own_write_set={");
	crt_ptr += strlen(crt_ptr);
	for(int i=0;i<ca->no_own_write_set;i++)
	{
		to_string_cell(ca->own_write_set+i, crt_ptr);
		crt_ptr += strlen(crt_ptr);
		sprintf(crt_ptr, ", ");
		crt_ptr += strlen(crt_ptr);
	}

	sprintf(crt_ptr, "}, complete_read_set={");
	crt_ptr += strlen(crt_ptr);
	for(int i=0;i<ca->no_complete_read_set;i++)
	{
		to_string_cell(ca->complete_read_set+i, crt_ptr);
		crt_ptr += strlen(crt_ptr);
		sprintf(crt_ptr, ", ");
		crt_ptr += strlen(crt_ptr);
	}

	sprintf(crt_ptr, "}, complete_write_set={");
	crt_ptr += strlen(crt_ptr);
	for(int i=0;i<ca->no_complete_write_set;i++)
	{
		to_string_cell(ca->complete_write_set+i, crt_ptr);
		crt_ptr += strlen(crt_ptr);
		sprintf(crt_ptr, "}, ");
		crt_ptr += strlen(crt_ptr);
	}

	sprintf(crt_ptr, "}, version=");
	crt_ptr += strlen(crt_ptr);

	if(ca->version != NULL)
	{
		to_string_vc(ca->version, crt_ptr);
		crt_ptr += strlen(crt_ptr);
	}

	sprintf(crt_ptr, ")");

	return msg_buff;
}

int equals_txn_message(txn_message * ca1, txn_message * ca2)
{
	if(ca1->nonce != ca2->nonce || ca1->type != ca2->type ||
		ca1->no_own_read_set != ca2->no_own_read_set ||
		ca1->no_own_write_set != ca2->no_own_write_set ||
		ca1->no_complete_read_set != ca2->no_complete_read_set ||
		ca1->no_complete_write_set != ca2->no_complete_write_set)
		return 0;

	if(ca1->txnid != NULL && ca2->txnid && uuid_compare(*(ca1->txnid), *(ca2->txnid)))
		return 0;

	if(compare_vc(ca1->version, ca2->version))
		return 0;

	for(int i=0;i<ca1->no_own_read_set;i++)
		if(!equals_cell(ca1->own_read_set+i, ca2->own_read_set+i))
			return 0;

	for(int i=0;i<ca1->no_own_write_set;i++)
		if(!equals_cell(ca1->own_write_set+i, ca2->own_write_set+i))
			return 0;

	for(int i=0;i<ca1->no_complete_read_set;i++)
		if(!equals_cell(ca1->complete_read_set+i, ca2->complete_read_set+i))
			return 0;

	for(int i=0;i<ca1->no_complete_write_set;i++)
		if(!equals_cell(ca1->complete_write_set+i, ca2->complete_write_set+i))
			return 0;

	return 1;
}




