# Fast Memory State Synchronization for Virtualization-based Fault Tolerance
## Design Alternatives
### Synchronization Traffic Reduction Using Active Slave

In both XSFT and Remus, the Slave is passive in the sense that the Slave is not actively running but is ready to go when the Master dies. In the passive Slave design, a Slave never consumes any CPU resource of the physical machine on which it resides.

An alternative to the passive Slave design is the active Slave design, in which the slave runs concurrently with the Master, which is the only entity that directly interacts with external machines.

Because Master and Slave run the same code, receive the same network inputs and are frequently synchronized with respect to their responses to incoming requests, their memory states should be largely the same at the end of an epoch and accordingly the amount of data to be transferred for memory state synchronization is expected to be small.

In the active Slave configuration, at the end of each epoch, the Slave sends its dirty page list, which includes the page number and hash value of each dirty page, to the Master, which compares the received dirty page list with its own dirty page list.

## Comparative Evaluation
### Analysis of Performance Overhead
Besides the overhead due to page write-protection, the computation of MD5 hash values of each data block contributes the other significant run-time overhead. It takes on average 0.05 msec to compute the hash value of a whole 4KB page.