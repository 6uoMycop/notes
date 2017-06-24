## System run levels
Services and daemons are executed depending on the "runlevel" that you machine is operating on, which corresponds to the links in the directories /etc/rc.d/rc#.d/ where # is a number from 0 to 6. These six run levels are:

- 0 - Halt
- 1 - Single User Mode
- 2 - Multiuser, without NFS (The same as 3, if you do not have networking)
- 3 - Full Multiuser Mode
- 4 - unused, can be setup for networking in a different location, etc.
- 5 - Full Multiuser Mode with XWindows (foundation of GUI)
- 6 - Reboot (your machine switches to this one when you use ctrl-alt-delete)

Typically your machine runs at runlevel 3 or runlevel 5 for normal operation.

## init.d scripts
The `/etc/init.d` directory contains the scripts executed by `init` at boot time and when the init state (or "runlevel") is changed.

These scripts are referenced by symbolic links in the `/etc/rcn.d` directories. When changing runlevels, `init` looks in the directory `/etc/rcn.d` for the scripts it should execute, where `n` is the runlevel that is being changed to, or `S` for the boot-up scripts.

The names of the links all have the form `Smmscript` or `Kmmscript` where `mm` is a two-digit number and `script` is the name of the script (this should be the same as the name of the actual script in `/etc/init.d`).

When `init` changes runlevel first the targets of the links whose names start with a `K` are executed, each with the single argument `stop`, followed by the scripts prefixed with an `S`, each with the single argument `start`. (The links are those in the `/etc/rcn.d` directory corresponding to the new runlevel.) The `K` links are responsible for killing services and the `S` link for starting services upon entering the runlevel.

For example, if we are changing from runlevel 2 to runlevel 3, init will first execute all of the `K` prefixed scripts it finds in `/etc/rc3.d`, and then all of the `S` prefixed scripts in that directory. The links starting with `K` will cause the referred-to file to be executed with an argument of `stop`, and the `S` links with an argument of `start`.

The two-digit number `mm` is used to determine the order in which to run the scripts: low-numbered links have their scripts run first. For example, the `K20` scripts will be executed before the `K30` scripts. This is used when a certain service must be started before another. For example, the name server `bind` might need to be started before the news server `inn` so that inn can set up its access lists. In this case, the script that starts `bind` would have a lower number than the script that starts `inn` so that it runs first:
```
     /etc/rc2.d/S17bind
     /etc/rc2.d/S70inn
```