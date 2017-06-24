## Pipes
Before pipes, Wireshark could read the captured packets to display either from a file (which had been previously created) or for a network interface (in real time). Since pipes are supported, Wireshark can also read captured packets from another application in real time. This is useful if you want to watch a network in real time, and Wireshark cannot capture from that network.

### Named pipes

A named pipe looks like a file, but it is really just a buffer for interprocess communication. One process can send data to it, and another process can read it. There are two main ways to create a named pipe: with mkfifo or using special syntax of the bash shell.

Way 1: mkfifo on UN*X

If you have a capture file in the right format (from Wireshark or tcpdump), you can do the following:
```
$ mkfifo /tmp/sharkfin
$ wireshark -k -i /tmp/sharkfin &
$ cat capture.cap > /tmp/sharkfin &
```

### Remote Capture
This can be worked around by combining SSH with a FIFO.
```
$ mkfifo /tmp/sharkfin
$ wireshark -k -i /tmp/sharkfin &
$ ssh user@remote-host "dumpcap -i eth0 -P -w - -f 'not tcp port 22'" > /tmp/sharkfin
```
As soon as Wireshark starts "listening" on the pipe, SSH will prompt for the password and allow you to continue.