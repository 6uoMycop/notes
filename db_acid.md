## ACID properties
### Consistency
#### Database Consistency
- **Enterprise (Business) Rules** limit the occurrence of certain real-world events
   - Student cannot register for a course if the current number of registrants equals the maximum allowed
- Correspondingly, allowable database states are restricted
   - `Current_reg <= max_reg`
- These limitations are called **integrity constraints**: assertions that must be satisfied by the database state

- **Database is consistent** if all static integrity constraints are satisfied

#### Transaction Consistency
- A consistent database state does not necessarily model the actual state of the enterprise
   -  A deposit transaction that increments the balance by the wrong amount maintains the integrity constraint `balance >= 0`, but does not maintain the relation between the enterprise and database states
   - Dynamic Integrity Constraints: Some constraints restrict allowable state transitions
      - A transaction might transform the database from one consistent state to another, but the transition might not be permissible
      - **Example:** Students can only progress from Junior via Intermediate to the Senior year, but can never be degraded.
   - A consistent transaction maintains database consistency *and* the correspondence between the database state and the enterprise state

- A **transaction is consistent** if, assuming the database is in a consistent state initially, when the transaction completes:
   - All static integrity constraints are satisfied (but constraints might be violated in intermediate states)
   - New state satisfies specifications of transaction
   - No dynamic constraints have been violated

### Durability
- The system must ensure that once a transaction commits, its effect on the database state is not lost in spite of subsequent failures

### Atomicity
- A real-world event either happens or does not happen
   - Student either registers or does not register
- Similarly, the system must ensure that either the corresponding transaction runs to completion or, if not, it has no effect at all
   - a user can think of a transaction as always executing all its actions in one step, or not executing any actions at all.
   - Not true of ordinary programs. A crash could leave files partially updated on recovery. 

### Isolation
- **Serial** Execution: transactions execute in sequence
   - Each one starts after the previous one completes
      - Execution of one transaction if not affected by the operations of another since they do not overlap in time
   - The execution of each transaction is **isolated** from all others
- If the initial database state and all transactions are consistent, then the final database state will be consistent and will accurately reflect the real-world state, *but*
   - Serial execution is inadequate from a performance perspective
- **Concurrent execution** offers performance benefits:
   - A computer system has multiple resources capable of executing independently (e.g., cpu's, I/O devices), *but*
   - A transaction typically uses only one resource at a time
   - The net effect must be equivalent to transactions running serially