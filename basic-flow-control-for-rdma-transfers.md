## RDMA Write With Immediate

There are many ways we could orchestrate the transfer of an entire file from client to server. For instance:
- Load the entire file into client memory, connect to the server, wait for the server to post a set of receives, then issue a send operation (on the client) to copy the contents to the server.
- Load the entire file into client memory, register the memory, pass the region details to the server, let it issue an RDMA read to copy the entire file into its memory, then write the contents to disk.
- As above, but issue an RDMA write to copy the file contents into server memory, then signal it to write to disk.
- Open the file on the client, read one chunk, wait for the server to post a receive, then post a send operation on the client side, and loop until the entire file is sent.
- As above, but use RDMA reads.
- As above, but use RDMA writes.

Loading the entire file into memory can be impractical for large files, so we will skip the first three options. Of the remaining three, I will focus on using RDMA writes so that I can illustrate the use of the RDMA-write-with-immediate-data operation, something that I have been intending to discuss for some time. This operation is similar to a regular RDMA write except that the initiator can "attach" a 32-bit value to the write operation. Unlike regular RDMA writes, RDMA writes with immediate data require that a receive operation be posted on the target's receive queue. The 32-bit value will be available when the completion is pulled from the target's queue.
Now that we decided that we are going to break up the file into chunks, and write the chunks one at a time into the server's memory, we must find a way to ensure that we do not write chunks faster than the server can process them. We will do this by instructing the server to send explicit messages to the client when it is ready to receive data. The client, on the other hand, will use writes with immediate data to signal the server. The sequence looks like this:

1. Server starts listening for connections.
2. Client posts a receive operation for a flow-control message and initiates a connection to the server.
3. Server posts a receive operation for an RDMA write with immediate data and accepts the connection from the client.
4. Server sends the client its target memory region details.
5. Client re-posts a receive operation then responds by writing the name of the file to the server's memory region. The immediate data field contains the length of the file name.
6. Server opens a file descriptor, re-posts a receive operation, then responds with a message indicating that it is ready to receive data.
7. Client re-posts a receive operation, reads a chunk from the input file, then writes the chunk to the server memory region. The immediate data contains the size of the chunk in bytes.
8. Server writes the chunk to disk, re-posts a receive operation, then responds with a message indicating that it is ready to receive data.
9. Repeat steps 7 and 8 until there are no data left to send.
10. Client re-posts a receive operation, then initializes a zero-byte write to the server's memory. The immediate data field is set to zero.
11. Client closes the connection.
12. Server closes the file descriptor.


This sequence lends itself well to an event-driven implementation - the client acts in response to a message from the server, and vice versa. Because the client only processes three types of messages from the server, and the server only processes the immediate data from incoming RDMA writes, our completion processing functions, where we do the bulk of work, will be relatively simple.

### Server
```
static void post_receive (struct rdma_cm_id *id)
{
	struct ibv_recv_wr wr , *bad_wr = NULL;

	memset(&wr , 0, sizeof(wr));
	wr.wr_id = (uintptr_t )id;
	wr.sg_list = NULL;
	wr.num_sge = 0;

	TEST_NZ(ibv_post_recv(id->qp, &wr ,&bad_wr));
}
```

```
static void on_completion(struct ibv_wc *wc)
{
	struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)wc->wr_id;
	struct conn_context *ctx = (struct conn_context *)id->context;

	if (wc->opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
		uint32_t size = ntohl(wc->imm_data);

		if (size == 0) {
			ctx->msg->id = MSG_DONE; send_message(id);
			send_message(id);
		} else if (ctx->file_name[0]) {
			ssize_t ret;
			
			printf("received %i bytes.\n", size);

			ret = write(ctx->fd, ctx->buffer, size);

			if (ret != size)
				rc_die("write() failed");

			post_receive(id);

			ctx->msg->id = MSG_READY;
			send_message(id);

		} else {
			memcpy(ctx->file_name, ctx->buffer, (size > MAX_FILE_NAME) ? MAX_FILE_NAME : size);
			ctx->file_name[size - 1] = ’\0’;

			printf("opening file %s\n", ctx->file_name);

			ctx->fd = open(ctx->file_name, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);	

			if (ctx->fd == -1)
				rc_die("open() failed");

			post_receive(id);
			ctx->msg->id = MSG_READY;
			send_message(id); 
		}
	}
}
```
We retrieve the immediate data field in line 7 and convert it from network byte order to host byte order. We then test three possible conditions:

1. If size == 0, the client has finished writing data (lines 9 - 11). We acknowledge this with MSG_DONE.
2. If the first byte of ctx->filename is set, we already have the file name and have an open file descriptor. We call write() to append the client's data to our open file then reply with MSG_READY, indicating that we are ready to accept more data.
3. Otherwise, we have yet to receive the file name. We copy it from the incoming buffer, open a descriptor, then reply with MSG_READY to indicate that we are ready to receive data.

```
static void send_message(struct rdma_cm_id *id)
{
	...
	
	wr.opcode = IBV_WR_SEND;
	
	...
	sge.addr = (uintptr_t)ctx->msg;
	sge.length = sizeof(*ctx->msg);
	sge.lkey = ctx->msg_mr->lkey;

	TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
}
```

### Client
```
static void post_receive(struct rdma_cm_id *id) {
	struct client_context *ctx = (struct client_context *)id->context;

	struct ibv_recv_wr wr, *bad_wr = NULL;
	struct ibv_sge sge;

	memset(&wr, 0, sizeof(wr));

	wr.wr_id = (uintptr_t)id;
	wr.sg_list = &sge;
	wr.num_sge = 1;

	sge.addr = (uintptr_t)ctx->msg;
	sge.length = sizeof(*ctx->msg);
	sge.lkey = ctx->msg_mr->lkey;

	TEST_NZ(ibv_post_recv(id->qp, &wr, &bad_wr));
}
```

```
static void on_completion(struct ibv_wc *wc) {
	struct rdma_cm_id *id = (struct rdma_cm_id *)(uintptr_t)(wc->wr_id);
	struct client_context *ctx = (struct client_context *)id->context;

	if (wc->opcode & IBV_WC_RECV) {
		if (ctx->msg->id == MSG_MR) {
			ctx->peer_addr = ctx->msg->data.mr.addr;
			ctx->peer_rkey = ctx->msg->data.mr.rkey;

			printf("received MR, sending file name\n");
			send_file_name(id);
		} else if (ctx->msg->id == MSG_READY) {
			printf("received READY , sending chunk\n");
			send_next_chunk(id);
		} else if (ctx->msg->id == MSG_DONE) {
			printf("received DONE, disconnecting\n");
			rc_disconnect(id);
			return;
		}

		post_receive(id);
	}
}
```

```
static void write_remote(struct rdma_cm_id *id, uint32_t len) {
	struct client_context *ctx = (struct client_context *)id->context;

	struct ibv_send_wr wr, *bad_wr = NULL;
	struct ibv_sge sge;

	memset(&wr, 0, sizeof(wr));

	wr.wr_id = (uintptr_t)id;
	wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
	wr.send_flags = IBV_SEND_SIGNALED;
	wr.imm_data = htonl(len);
	wr.wr.rdma.remote_addr = ctx->peer_addr;
	wr.wr.rdma.rkey = ctx->peer_rkey;

	if (len) {
		wr.sg_list = &sge; wr.num_sge = 1;
		sge.addr = (uintptr_t)ctx->buffer;
		sge.length = len;
		sge.lkey = ctx->buffer_mr->lkey;
	}

	TEST_NZ(ibv_post_send(id->qp, &wr, &bad_wr));
}
```