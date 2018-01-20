## Splitting Responsibility
- We need a singly totally ordered set of Operations
- *Ordering*: If some replica processes request *a* before *b*, no replica processes *b* before *a*
- *Reliable Delivery*: Every request submitted by a client is either processed by all replicas or none
- The network fabric takes care of ordering while NOPaxos is responsible for reliable delivery

## Ordered Unreliable Multicast
### OUM Sessions and the libOUM API
A set of receivers is identified by a common IP address and called a *Group*.

**libOUM Sender Interface**
- `send(addr destination, byte[] message)` - send a message to the given OUM group

**libOUM Receiver Interface**
- `getMessage()` - returns the next message, a `DROP-NOTIFICATION`, or a `SESSION-TERMINATED` error

## OUM Design and Implementation
The basic design is straightforward: the network routes all packets destined for a given OUM group through a single sequencer, a low-latency device that serves one purpose: to add a sequence number to each packet before forwarding it to its destination. Since all packets have been marked with a sequence number, the libOUM library can ensure ordering by discarding messages that are received out of order and detect and report dropped messages by noticing gaps in the sequence number.

### Implementing the Sequencer
Even if packets are dropped (e.g., due to congestion or link failures) or reordered (e.g., due to multipath effects) in the network, receivers can use the sequence numbers to ensure that they process packets in order and deliver drop notifications for missing packets.

### Fault Tolerance
- Sequencer - Common point of failure

In our design, the network controller monitors the sequencer's availability. If it fails or no longer has a path to all OUM group members, the controller selects a different switch. It reconfigures the network to use this new sequencer by updating routes in other switches.

If the sequencer itself fails, the ordering guarantee of multicast must not be lost. To address this, a monotonically increasing session number is introduced by the controller.

## NOPaxos
- Built on top of OUM guarantees
- Only responsibility is to decide if a Request should be executed or not

### Normal operations
In the normal case when replicas receive `REQUEST`s instead of `DROP-NOTIFICATION`s, client requests are committed and executed in a single phase. Clients broadcast `<REQUEST, op, request-id>` to all replicas through libOUM, where `op` is the operation they want to execute, and `request-id` is a unique id used to match requests and their responses.

When each replica receives the client's `REQUEST`, it increments `session-msg-num` and appends `op` to the log. If the replica is the leader of the current view, it executes the `op`. Each replica then replies to the client with `<REPLY, view-id, log-slot-num, request-id, result>`, where `log-slot-num` is the index of `op` in the log. If the replica is the leader, it includes the `result` of the operation; `NULL` otherwise.

The client waits for `REPLY`s to the `REQUEST` with matching `view-id`s and `log-slot-num`s from `f + 1` replicas, where one of those replicas is the leader of the view. This indicates that the request will remain persistent even across view changes. If the client does not receive the required `REPLY`s within a timeout, it retries the request.

### Gap Agreement
NOPaxos replicas always process operations in order. When a replica receives a `DROP-NOTIFICATION` from libOUM (and increments its `session-msg-num`), it must either recover the contents of the missing request or prevent it from succeeding before moving on to subsequent requests. Non-leader replicas do this by contacting the leader for a copy of the request. If the leader itself receives a `DROP-NOTIFICATION`, it coordinates to commit a `NO-OP` operation in place of that request:

1. If the leader receives a `DROP-NOTIFICATION`, it inserts a `NO-OP` into its *log* and sends a `<GAP-COMMIT, log-slot>` to the other replicas, where `log-slot` is the slot into which the `NO-OP` was inserted.
2. When a non-leader replica receives the `GAP-COMMIT` and has filled all log slots up to the one specified by the leader, it inserts a `NO-OP` into its *log* at the specified location (possibly overwriting a `REQUEST`) and replies to the leader with a `<GAP-COMMIT-REP, log-slot>`.
3. The leader waits for `f` `GAP-COMMIT-REP`s (retrying if necessary).

Clients need not be notified explicitly when a `NO-OP` has been committed in place of one of their requests. They simply retry their requests after failing to receive a quorum of responses.