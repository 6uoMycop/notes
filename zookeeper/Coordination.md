# ZooKeeper: Distributed Process Coordination

## Programming with ZooKeeper
### The C client
#### Setting Up the Development Environment

In the directory where you unpacked the ZooKeeper distribution, there is a file called *build.xml*. This file has the instructions required for *ant* to build everything.

## Administering ZooKeeper

### ZooKeeper Internals
#### Zab: Broadcasting State Updates
Upon receiving a write request, a follower forwards it to the leader. The leader executes the request speculatively and broadcasts the result of the execution as a state update, in the form of a transaction. A transaction comprises the exact set of changes that a server must apply to the data tree when the transaction is committed. The data tree is the data structure holding the ZooKeeper state (see `DataTree`).

The next question to answer is how a server determines that a transaction has been committed. Assuming that there is an active leader and it has a quorum of followers supporting its leadership, the protocol to commit a transaction is very simple, resembling a two-phase commit:

1. The leader sends a `PROPOSAL message`, *p*, to all followers.
2. Upon receiving *p*, a follower responds to the leader with an `ACK`, informing the leader that it has accepted the proposal.
3. Upon receiving acknowledgments from a quorum (the quorum includes the leader itself), the leader sends a message informing the followers to `COMMIT` it.

#### The Skeleton of a Server
##### Leader Servers
The first processor is still `PrepRequestProcessor`, but the following processor now becomes `ProposalRequestProcessor`. It prepares proposals and sends them to the followers. `ProposalRequestProcessor` forwards all requests to `CommitRequestProcessor`, and additionally forwards the write request to `SyncRequestProcessor`. `SyncRequestProcessor` works the same as it does for the standalone server, and persists transactions to disk. It ends by triggering `AckRequestProcessor`, a simple request processor that generates an acknowledgment back to itself. As we mentioned earlier, the leader expects acknowledgments from every server in the quorum, including itself. `AckRequestProcessor` takes care of this.

The other processor following `ProposalRequestProcessor` is `CommitRequestProcessor`. `CommitRequestProcessor` commits proposals that have received enough acknowledgments. The acknowledgments are actually processed in the `Leader` class (the `Leader.processAck()` method), which adds committed requests to a queue in `CommitRequestProcessor`. The request processor thread processes this queue.

##### Follower and Observer Servers

When the leader receives a new write request, directly or through a learner, it generates a proposal and forwards it to followers. Upon receiving a proposal, a follower sends it to `SyncRequestProcessor`. `SyncRequestProcessor` processes the request, logging it to disk, and forwards it to `SendAckRequestProcessor`. `SendAckRequestProcessor` acknowledges the proposal to the leader. After the leader receives enough acknowledgments to commit a proposal, the leader sends commit messages to the followers (and sends `INFORM` messages to the observers). Upon receiving a commit message, a follower processes it with `CommitRequestProcessor`.

#### Local Storage
##### Logs an Disk Use
Recall that servers use the transaction log to persist transactions. Before accepting a proposal, a server (follower or leader) persists the transaction in the proposal to the transaction log, a file on the local disk of the server to which transactions are appended in order.

##### Snapshots
Snapshots are copies of the ZooKeeper data tree. Each server frequently takes a snapshot of the data tree by serializing the whole data tree and writing it to a file. The servers do not need to coordinate to take snapshots, nor do they have to stop processing requests. Because servers keep executing requests while taking a snapshot, the data tree changes as the snapshot is taken. We call such snapshots *fuzzy*, because they do not necessarily reflect the exact state of the data tree at any particular point in time.

Let's walk through an example to illustrate this. Say that a data tree has only two znodes: `/z` and `/z'`. Initially, the data of both `/z` and `/z'` is the integer 1. Now consider the following sequence of steps:

1. Start a snapshot.
2. Serialize and write `/z = 1` to the snapshot.
3. Set the data of `/z` to 2 (transaction *T*).
4. Set the data of `/z'` to 2 (transaction *T'*).
5. Serialize and write `/z'` = 2 to the snapshot.

This snapshot contains `/z = 1` and `/z' = 2`. However, there has never been a point in time in which the values of both znodes were like that. This is not a problem, though, because the server replays transactions. It tags each snapshot with the last transaction that has been committed when the snapshot starts - call it *TS*. If the server eventually loads the snapshot, it replays all transactions in the transaction log that come after *TS*. In this case, they are *T* and *T'*. After replaying *T* and *T'* on top of the snapshot, the server obtains `/z = 2` and `/z' = 2`, which is a valid state.

### Running ZooKeeper
#### Configuring a ZooKeeper Server
##### Unsafe Options
- `forceSync`
   
   A "yes" or "no" option that controls whether data should be synced to storage (`zookeeper.forceSync`).

   By default, and when `forceSync` is set to `yes`, transactions will not be acknowledged until they have been synced to storage. The sync system call is expensive and is the cause of one of the biggest delays in transaction processing. If `forceSync` is set to `no`, transactions will be acknowledged as soon as they have been written to the operating system, which usually caches them in memory before writing them to disk. Setting `forceSync` to `no` will yield an increase in performance at the cost of recoverability in the case of a server crash or power outage.
