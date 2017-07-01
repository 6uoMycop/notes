## RDMA-Aware Programming Overview
### Transport Modes

Operation | UD | UC | RC
------------ | ------------- | ------------- | -------------
Send (with immediate) | X | X | X
Receive | X | X | X
RDMA Write (with immediate) | | X | X
RDMA Read | | | X

#### Reliable Connection (RC)
Queue Pair is associated with only one other QP.

Messages transmitted by the send queue of one QP are reliably delivered to receive queue of the other QP.

Packets are delivered in order.

A RC connection is very similar to a TCP connection.

#### Unreliable Connection (UC)
A Queue Pair is associated with only one other QP.

The connection is not reliable so packets may be lost.

#### Unreliable Datagram (UD)
A Queue Pair may transmit and receive single-packet messages to/from any other UD QP.

Ordering and delivery are not guaranteed, and delivered packets may be dropped by the receiver.

Multicast messages are supported (one to many).

A UD connection is very similar to a UDP connection.

### Key Concepts
#### Global Routing Header (GRH)
The GRH is used for routing between subnets. When using RoCE, the GRH is used for routing inside the subnet and therefore is a mandatory. The use of the GRH is mandatory in order for an application to support both IB and RoCE.

When global routing is used on UD QPs, there will be a GRH contained in the first 40 bytes of the receive buffer.

## VPI Verbs API
### Verb Context Operations
#### ibv_create_cq
**Template:**

```
struct ibv_cq *ibv_create_cq(struct ibv_context *context, int cqe, void *cq_context, struct ibv_comp_channel *channel, int comp_vector)
```

**Description:**

The parameter cqe defines the minimum size of the queue. The actual size of the queue may be larger than the specified value.

### Protection Domain Operations
#### ibv_reg_mr
**Template:**

```
struct ibv_mr *ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, enum ibv_access_flags access)
```

**Description:**

`ibv_reg_mr` registers a memory region (MR), associates it with a protection domain (PD), and assigns it local and remote keys (lkey, rkey).

struct ibv_mr is defined as follows:
```
struct ibv_mr
{
    struct ibv_context *context;
    struct ibv_pd *pd;
    void *addr;
    size_t length;
    uint32_t handle;
    uint32_t lkey;
    uint32_t rkey;
};
```

### Queue Pair Bringup (ibv_modify_qp)
#### INIT to RTR
**Required Attributes**

```
*** Connected QPs only ***
rq_psn / IBV_QP_RQ_PSN     starting receive packet sequence number (should match remote QP's sq_psn)
```

**Effect of transition**

Once the QP is transitioned into the RTR state, the QP begins receive processing.

#### RTR to RTS
**Required Attributes**

```
*** Connected QPs only ***
sq_psn / IBV_SQ_PSN        send queue starting packet sequence number (should match remote QP's rq_psn)
```

**Effect of transition**

Once the QP is transitioned into the RTS state, the QP begins send processing and is fully operational. The user may now post send requests with the `ibv_post_send` command.

### Active Queue Pair Operations
#### ibv_poll_cq
**Template:**

```
int ibv_poll_cq(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc)
```

**Description**

`ibv_poll_cq` retrieves CQEs from a completion queue (CQ). The user should allocate an array of struct ibv_wc and pass it to the call in wc.

## Events
### IBV WC Events
#### IBV_WC_SUCCESS
The Work Request completed successfully.

> In RC QP, a CQE with successful status of a Send Request means that an ACK was received by the sender. However, In UC QP, a CQE with successful status of a Send Request means that the message was sent; it isn't indicate that the message was received by the remote side ... (You will get a successful CQE even if the remote side didn't receive this message)
