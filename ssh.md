## Proxy Connection with SSH
### Instruction for a Linux machine

1. Open a shell and connect to a CUNIX server using ssh. Example: ssh -D 9999 hkucs@202.45.128.160
   ```
   -D [bind_address:]port
           Specifies a local "dynamic" application-level port forwarding. This works by allocating a socket
           to listen to port on the local side, optionally bound to the specified bind_address. Whenever a
           connection is made to this port, the connection is forwarded over the secure channel, and the
           application protocol is then used to determine where to connect to from the remote machine.
   ```
2. In your web browser, you need to setup your proxy to point to "localhost", your port (for our example, our port is 9999), and that you are using SOCKS5. For our example, we will use Firefox.

   2.1 Run Firefox.
   
   2.2 Select `Edit->Preferences` from the menu.
   
   2.3 Select the `Advanced` icon.
   
   2.4 Select the `Network` tab.
   
   2.5 Click on the `Settings` button.
   
   2.6 Select the `Manual Proxy Settings` checkbox.
   
   2.7 On the `SOCKS Host"` field, type localhost. Then, for the Port field, fill in the port you are using. For our example, the port number is 9999.
   
   2.8 Make sure `SOCKS v5` is checked.
   
   2.9 Click `OK`.
