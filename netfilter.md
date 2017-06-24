## Introduction
### What is netfilter?
netfilter is a framework for packet mangling, outside the normal Berkeley socket interface. It has four parts. Firstly, each protocol defines "hooks" (IPv4 defines 5) which are well defined points in a packet's traversal of that protocol stack. At each of these points, the protocol will call the netfilter framework with the packet and the hook number.

Secondly, parts of the kernel can register to listen to the different hooks for each protocol. So when a packet is passed to the netfilter framework, it checks to see if anyone has registered for that protocol and hook; if so, they each get chance to possibly examine (and possibly alter) the packet in order, then discard the packet (NF_DROP), allow it to pass (NF_ACCEPT), tell the netfilter to forget about the packet (NF_STOLEN), or ask netfilter to queue the packet for userspace (NF_QUEUE).

## Netfilter Architecture
Netfilter merely a series of hooks in various points in a protocol stack (at this stage, IPv4, IPv6 and DECnet). The (idealized) IPv4 traversal diagram looks like this:
```
A Packet Traversing the Netfilter System:

   --->[1]--->[ROUTE]--->[3]--->[4]--->
                 |            ^
                 |            |
                 |         [ROUTE]
                 v            |
                [2]          [5]
                 |            ^
                 |            |
                 v            |
```
On the left is where packets come in: having passed the simple sanity checks (i.e., not truncated, IP checksum OK, not a promiscuous receive), they are passed to the netfilter framework's NF_IP_PRE_ROUTING [1] hook.

Next, they enter the routing code, which decides whether the packet is destined for another interface, or a local process.
If it's destined for the box itself, the netfilter framework is called again for the NF_IP_LOCAL_IN [2] hook, before being passed to the process (if any).

If it's destined to another interface instead, the netfilter framework is called for the NF_IP_FORWARD [3] hook.

### Netfilter Base
Now we have an example of netfilter for IPv4, you can see when each hook is activated. This is the essence of netfilter.

Kernel modules can register to listen at any of these hooks. A module that registers a function must specify the priority of the function within the hook; then when that netfilter hook is called from the core networking code, each module is registered at that point is called in the order of priorities, and is free to manipulate the packet. The module can then tell the netfilter to do one of five things:

1. NF_ACCEPT: continue traversal as normal.

2. NF_DROP: drop the packet; don't continue traversal.

3. NF_STOLEN: I've taken over the packet; don't continue traversal.

4. NF_QUEUE: queue the packet (usually for userspace handling).

5. NF_REPEAT: call this hook again.

Upon this foundation, we can build fairly complex packet manipulations.

### Packet Selection: IP Tables
A packet selection system called IP Tables has been built over the netfilter framework. This packet selection method is used for packet filtering (the 'filter' table), Network Address Translation (the 'nat' table) and general pre-route packet mangling (the 'mangle' table).

#### Packet Filtering
This table, 'filter', should never alter packets: only filter them. It hooks into netfilter at the NF_IP_LOCAL_IN, NF_IP_FORWARD and NF_IP_LOCAL_OUT points. This means that for any given packet, there is one (and only one) possible place to filter it.

#### NAT
This is the realm of the 'nat' table, which is def packets from two netfilter hooks: for non-local packets, the NF_IP_PRE_ROUTING and NF_IP_POST_ROUTING hooks are perfect for destination and source alterations respectively.

## Information for Programmers
### Understanding ip_tables
iptables simply provides a named array of rules in memory, and such information as where packets from each hook should begin traversal.
#### ip_tables From Userspace
```
$ iptables --table filter --list
Chain INPUT (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     udp  --  anywhere             anywhere             udp dpt:domain
ACCEPT     tcp  --  anywhere             anywhere             tcp dpt:domain
ACCEPT     udp  --  anywhere             anywhere             udp dpt:bootps
ACCEPT     tcp  --  anywhere             anywhere             tcp dpt:bootps
ACCEPT     tcp  --  anywhere             anywhere             tcp dpt:domain
ACCEPT     udp  --  anywhere             anywhere             udp dpt:domain
ACCEPT     tcp  --  anywhere             anywhere             tcp dpt:bootps
ACCEPT     udp  --  anywhere             anywhere             udp dpt:bootps

Chain FORWARD (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     all  --  anywhere             192.168.122.0/24     ctstate RELATED,ESTABLISHED
ACCEPT     all  --  192.168.122.0/24     anywhere            
ACCEPT     all  --  anywhere             anywhere            
REJECT     all  --  anywhere             anywhere             reject-with icmp-port-unreachable
REJECT     all  --  anywhere             anywhere             reject-with icmp-port-unreachable
ACCEPT     all  --  anywhere             anywhere            
ACCEPT     all  --  anywhere             anywhere            

Chain OUTPUT (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     udp  --  anywhere             anywhere             udp dpt:bootpc
```
`iptables` is used to set up, maintain, and inspect the tables of IPv4 packet filter rules in the Linux kernel. Several different tables may be defined. Each table contains a number of built-in chains and may also contain user-defined chains.

Each chain is a list of rules which can match a set of packets. Each rule specifies what to do with a packet that matches. This is called a 'target', which may be a jump to a user-defined chain in the same table.

A firewall rule specifies criteria for a packet and a target. If the packet does not match, the next rule in the chain is examined; if it does match, then the next rule is specified by the value of the target, which can be the name of a user-defined chain, one of the targets described in `iptables-extensions`, or one of the special values ACCEPT, DROP or RETURN.

ACCEPT means to let the packet through. DROP means to drop the packet on the floor. RETURN means stop traversing this chain and resume at the next rule in the previous (calling) chain. If the end of a built-in chain is reached or a rule in a built-in chain with target RETURN is matched, the target specified by the chain policy determines the fate of the packet.

## Understanding NAT
### Connection Tracking
Connection tracking hooks into high-priority NF_IP_LOCAL_OUT and NF_IP_PRE_ROUTING hooks, in order to see packets before they enter the system.

Basically, the connection tracking system stores information about the state of a connection in a memory structure that contains the source and destination IP address, port number pairs, protocol types, state, and timeout.

Keep in mind that the connection tracking system just tracks packets; it does not filter.

#### The big picture
This article focuses mainly in the layer-3 independent connection tracking system implementation `nf_conntrack`, based on the IPv4 dependent `ip_conn_track`.

#### Implementation Issues

##### Basic Structure
The connection tracking system is an optional modular loadable subsystem, although it is always required by the NAT subsystem. It is implemented with a hash table to perform efficient lookups.

![CONNECTION TRACKING STRUCTURE](connection_tracking_structure.png)

Each bucket has a double-linked list of hash tuples. There are two hash tuples for every connection: one for the original direction (i.e., packets coming from the point that started the connection) and one for the reply direction (i.e., reply packets going to the point that started the connection).

A tuple represents the relevant information of a connection, IP source and IP destination, as well as layer-4 protocol information. Such tuples are embedded in a hash tuple.

The two hash tuples are embedded in the structure `nf_conn`, from this point onward referred to as *conntrack*, which is the structure that stores the state of a given connection. Therefore, a conntrack is the container of two hash tuples, and every hash tuple is the container of a tuple. This results in three layers of embedded structures.

A hash function is used to calculate the position where the hash tuple that represents the connection is supposed to be.
##### The conntrack creation and lookup process
The callback `nf_conntrack_in` is registered in the *PREROUTING* hook. Some sanity checks are done at this stage to ensure that the packet is correct.

Afterward, checks take place during the conntrack lookup process. The system tries to lookup a conntrack that matches with the packet received. If no conntrack is found, it will be created.

If the packet belongs to a new connection, the conntrack just created will have the flag `confirmed` unset. The flag `confirmed` is set if such a conntrack is already in the hash table. This means that at this point no new conntracks are inserted. Such an insertion will happen once the packet leaves the framework successfully (i.e., when it arrives at the last hook without being dropped).
