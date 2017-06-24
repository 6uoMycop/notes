nfnetlink is a netlink based kernel/userspace transport layer. It provides a unified kernel/userspace interface for the various netfilter
subsystems, such as connection tracking.
```
struct nfnetlink_subsystem {
	.
	.
	__u8 subsys_id;			/* nfnetlink subsystem ID */
	__u8 cb_count;			/* number of callbacks */
	const struct nfnl_callback *cb;	/* callback for individual types */
	.
	.
};
```
For example, nfnetlink_colo module is for communication with the userspace tools like QEMU.
```
static const struct nfnl_callback nfnl_colo_cb[NFCOLO_MSG_MAX] = {
	[NFCOLO_KERNEL_NOTIFY] = { .call   = NULL,
		.policy = NULL,
		.attr_count = 0, },
	[NFCOLO_DO_CHECKPOINT] = { .call   = colo_do_checkpoint,
		.policy = nfnl_colo_policy,
		.attr_count = NFNL_COLO_MAX, },
	[NFCOLO_DO_FAILOVER] = { .call   = colo_do_failover,
		.policy = nfnl_colo_policy,
		.attr_count = NFNL_COLO_MAX, },
	[NFCOLO_PROXY_INIT] = { .call   = colo_init_proxy,
		.policy = nfnl_colo_policy,
		.attr_count = NFNL_COLO_MAX, },
	[NFCOLO_PROXY_RESET] = { .call   = colo_reset_proxy,
		.policy = nfnl_colo_policy,
		.attr_count = NFNL_COLO_MAX,},
};

static const struct nfnetlink_subsystem nfulnl_subsys = {
	.name		= "colo",
	.subsys_id	= NFNL_SUBSYS_COLO,
	.cb_count	 = NFCOLO_MSG_MAX,
	.cb		= nfnl_colo_cb,
};

static int __init nfnetlink_colo_init(void)
{
	.
	.
	status = nfnetlink_subsys_register(&nfulnl_subsys);
	if (status < 0) {
		pr_err("log: failed to create netlink socket\n");
		goto cleanup_netlink_notifier;
	}
	return status;
	.
	.
}
```
generic library for communication with netfilter
```
/**
 * nfnl_open - open a nfnetlink handler
 *
 * This function creates a nfnetlink handler, this is required to establish
 * a communication between the userspace and the nfnetlink system.
 *
 * On success, a valid address that points to a nfnl_handle structure
 * is returned. On error, NULL is returned and errno is set approapiately.
 */
struct nfnl_handle *nfnl_open(void);

/**
 * nfnl_subsys_open - open a netlink subsystem
 * @nfnlh: libnfnetlink handle
 * @subsys_id: which nfnetlink subsystem we are interested in
 * @cb_count: number of callbacks that are used maximum.
 *
 * This function creates a subsystem handler that contains the set of 
 * callbacks that handle certain types of messages coming from a netfilter
 * subsystem. Initially the callback set is empty, you can register callbacks
 * via nfnl_callback_register().
 */
struct nfnl_subsys_handle *
nfnl_subsys_open(struct nfnl_handle *nfnlh, u_int8_t subsys_id,
		 u_int8_t cb_count, u_int32_t subscriptions);

/**
 * nfnl_callback_register - register a callback for a certain message type
 * @ssh: nfnetlink subsys handler
 * @type: subsys call
 * @cb: nfnetlink callback to be registered
 *
 * On success, 0 is returned. On error, -1 is returned and errno is set
 * appropiately.
 */
int nfnl_callback_register(struct nfnl_subsys_handle *ssh,
			   u_int8_t type, struct nfnl_callback *cb);

```
Netlink Messages and Attributes Interface
```
/*
 * ------------------------------------------------------------------------
 *                          Messages Interface
 * ------------------------------------------------------------------------
 *
 * Message Format:
 *    <--- nlmsg_total_size(payload)  --->
 *   +----------+- - -+-------------+- - -+-------- - -
 *   | nlmsghdr | Pad |   Payload   | Pad | nlmsghdr
 *   +----------+- - -+-------------+- - -+-------- - -
 *   nlmsg_data(nlh)---^
 *
 * Payload Format:
 *   +----------------------+- - -+--------------------------------+
 *   |     Family Header    | Pad |           Attributes           |
 *   +----------------------+- - -+--------------------------------+
 *
 * Data Structures:
 *   struct nlmsghdr                    netlink message header
 *
 * Message Construction:
 *   nlmsg_put()                        add a netlink message to an skb
 *
 * Message Length Calculations:
 *   nlmsg_total_size(payload)          length of message w/ padding
 *
 * Message Payload Access:
 *   nlmsg_data(nlh)                    head of message payload
 *
 * ------------------------------------------------------------------------
 *                          Attributes Interface
 * ------------------------------------------------------------------------
 *
 * Attribute Format:
 *    <------- nla_total_size(payload) ------->
 *   +----------+- - -+- - - - - - - - - +- - -+-------- - -
 *   |  Header  | Pad |     Payload      | Pad |  Header
 *   +----------+- - -+- - - - - - - - - +- - -+-------- - -
 *
 * Attribute Construction for Basic Types:
 *   nla_put_s32(skb, type, value)      add s32 attribute to skb
 */

int colo_send_checkpoint_req(struct colo_primary *colo)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	.
	.
	struct nfgenmsg *nfmsg;

	.
	.
	skb = nfnetlink_alloc_skb(&init_net,
				nlmsg_total_size(sizeof(struct nfgenmsg)) +
				nla_total_size(sizeof(int32_t)), portid, GFP_ATOMIC);
	if (skb == NULL)
		return -ENOMEM;

	nlh = nlmsg_put(skb, portid, 0, NFNL_SUBSYS_COLO << 8 | NFCOLO_KERNEL_NOTIFY,
					sizeof(struct nfgenmsg), 0);
	if (!nlh)
		goto nla_put_failure;

	nfmsg = nlmsg_data(nlh);
	nfmsg->nfgen_family = AF_UNSPEC;
	nfmsg->version = NFNETLINK_V0;
	nfmsg->res_id = 0;

	nfcm.different = 1;
	if (nla_put_s32(skb, NFNL_COLO_COMPARE_RESULT,
					 htonl(nfcm.different))) {
		goto nla_put_failure;
	}
	nlh->nlmsg_len = skb->len; /* Don't forget to update this value */

	ret = nfnetlink_unicast(skb, node->net, portid, MSG_DONTWAIT);
	return ret;
	.
	.
}
```