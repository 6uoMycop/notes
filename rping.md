## ping-pong using RDMA SEND/RECV
### 3 phases in using reliable connections
- Setup Phase
  - obtain and convert addressing information
  - create and configure local endpoints for communication
  - setup local memory to be used in transfer
  - establish the connection with the remote side
- Use Phase
  - actually transfer data to/from the remote side
- Break-down Phase
  - basically "undo" the setup phase
  - close connection, free memory and communication resources

### RDMA access model
- Asynchronous data transfer
  - Posting
    - term used to mark the initiation of a data transfer
    - done by adding a work request to a work queue
  - Completion
    - term used to mark the end of a data transfer
    - done by removing a work completion from completion queue
- No data copying into system buffers
  - order and timing of send() and recv() are **relevant**
    - recv() must be waiting before issuing send()
  - Important note: memory involved in transfer is **untouchable** between start and completion of transfer

### Client use phase

1. mark start time for statistics
2. start of transfer loop
3. post receive to catch agent's pong data<br>
   rdma_post_recv()
4. post send to start transfer of ping data to agent<br>
   rdma_post_send()
5. wait for send to complete<br>
   ibv_poll_cq()
6. wait for receive to complete<br>
   ibv_poll_cq()
7. optionally verify pong data is ok<br>
   memcmp()
8. end of transfer loop
9. mark stop time and print statistics

### Server participants
- Listener
  - waits for connection requests from client
  - gets new system-provided connection to client
  - hands-off new connection to agent
  - never transfers any data to/from client

- Agent
  - creates control structures to deal with one client
  - allocates memory to deal with one client
  - performs all data transfers with one client
  - disconnects from client when transfers all finished


#### Listener setup and use phases
1. process command-line options
2. convert DNS name and port no.<br>
   rdma_getaddrinfo()
3. define properties of new queue pair<br>
   struct ibv_qp_init_attr
4. create and bind local end point<br>
   rdma_create_ep()
5. establish socket as listener<br>
   rdma_listen()
6. start loop
7. get connection request from client<br>
   rdma_get_request()
8. hand connection over to agent
9. end loop

#### Agent setup phase
1. make copy of listener's options and new cm_id for client
2. allocate user virtual memory<br>
   malloc()
3. register user virtual memory with CA<br>
   rdma_reg_msgs()
4. post first receive of ping data from client<br>
   rdma_post_recv()

5. define properties of new connection<br>
   struct rdma_conn_param

#### Agent use phase
1. start of transfer loop
2. wait to receive ping data from client<br>
   ibv_poll_cq()
3. If first time through loop<br>
   mark start time for statistics
4. post next receive for ping data from client<br>
   rdma_post_recv()
5. post send to start transfer of pong data to client<br>
   rdma_post_send()
6. wait for send to complete<br>
   ibv_poll_cq()
7. end of transfer loop