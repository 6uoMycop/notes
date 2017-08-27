# QEMU Networking
## Network Backends
### How to create a network backend?
There are a number of network backends to choose from depending on your environment. Create a network backend like this:
```
-netdev TYPE,id=NAME,...
```
The `id` option gives the name by which the virtual network device and the network backend are associated with each other.

### Network backend types
#### Tap
The tap networking backend makes use of a tap networking device in the host. It offers very good performance and can be configured to create virtually any type of network topology.

> The TAP is a Virtual Ethernet network device.<br>
> TAP driver was designed as low level kernel support for Ethernet tunneling. It provides to userland application two interfaces:
> - /dev/tapX - character device;
> - tapX - virtual Ethernet interface.<br>
>
> Userland application (QEMU) can write Ethernet frame to /dev/tapX and kernel will receive this frame from tapX interface. In the same time every frame that kernel writes to tapX interface can be read by userland application from /dev/tapX device.

```
-netdev tap,id=mynet0
```

> Qemu is a permanent memory process, which keeps reading the corresponding file descriptor in the kernel space by calling the function `tap_send` to check whether there is any frame received by the tap virtual device.

## Virtual Network Devices
### How to create a virtual network device?
The virtual network device that you choose depends on your needs and the guest environment (i.e. the hardware that you are emulating).

On machines that have PCI bus, there are a wider range of options. The e1000 is the default network adapter in qemu.

Use the `-device` option to add a particular virtual network device to your virtual machine:
```
-device TYPE,netdev=NAME
```
The `netdev` is the name of a previously defined `-netdev`. The virtual network device will be associated with this network backend.

```
e1000_receive_iov() {
    ...
    //Invoking DMA function
    ...
    //Fire interrupt
}
```

## Misc
### The legacy -net option

QEMU previously used the `-net nic` option instead of `-device DEVNAME` and `-net TYPE` instead of `-netdev TYPE`. This is considered obsolete since QEMU 0.12, although it continues to work.

The legacy syntax to create virtual network devices is:
```
-net nic,model=MODEL
```

#### QEMU "VLANs"
The obsolete `-net` syntax automatically created an emulated hub (called a QEMU "VLAN", for virtual LAN) that forwards traffic from any device connected to it to every other device on the "VLAN".

> Qemu VLANs are numbered starting with 0, and it's possible to connect one or more devices (either host side, like -net tap, or guest side, like -net nic) to each VLAN, and, in particular, it's possible to connect more than 2 devices to a VLAN. Each device in a VLAN gets all traffic received by every device in it.
