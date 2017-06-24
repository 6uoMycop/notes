## Write-Ahead Logging (WAL)
**Write-Ahead Logging** (WAL) is a standard method for ensuring data integrity. Briefly, WAL's central concept is that changes to data files (where tables and indexes reside) must be written only after those changes have been logged, that is, after log records describing the changes have been flushed to permanent storage. If we follow this procedure, we do not need to flush data pages to disk on every transaction commit, because we know that in the event of a crash we will be able to recover the database using the log: any changes that have not been applied to the data pages can be redone from the log records.

Using WAL results in a significantly reduced number of disk writes, because only the log file needs to be flushed to disk to guarantee that a transaction is committed, rather than every data file changed by the transaction.

## WAL Configuration

There are several WAL-related configuration parameters that affect database performance.

**Checkpoints** are points in the sequence of transactions at which it is guaranteed that the heap and index data files have been updated with all information written before that checkpoint. At checkpoint time, all dirty data pages are flushed to disk and a special checkpoint record is written to the log file. (The change records were previously flushed to the WAL files.) In the event of a crash, the crash recovery procedure looks at the latest checkpoint record to determine the point in the log from which it should start the REDO operation. Any changes made to data files before that point are guaranteed to be already on disk. Hence, after a checkpoint, log segments preceding the one containing the redo record are no longer needed and can be recycled or removed.

The checkpoint requirement of flushing all dirty data pages to disk can cause a significant I/O load.

The server's checkpointer process automatically performs a checkpoint every so often. A checkpoint is begun every checkpoint_segments log segments, or every checkpoint_timeout seconds, whichever comes first.

## VACUUM
Assume that we delete a few records from a table. PostgresSQL does not immediately remove the deleted tuples from the data files. These are marked as deleted.

Similarly, when a record is updated, it's roughly equivalent to one delete and one insert. The previous version of the record continues to be in the data file. The fillfactor for a table is a percentage between 10 and 100. 100 (complete packing) is the default. When a smaller is specified, INSERT operations pack table pages only to the indicated percentage; **the remaining space on each page is reserved for updating rows on that page.** This gives UPDATE a chance to place the updated copy of a row on the same page as the original, which is more efficient than placing it on a different page. For a table whose entries are never updated, complete packing is the best choice, but in heavily updated tables smaller fillfactors are appropriate.

The reason is simple: there can be active transactions, which want to see the data as it was before. As a result of this activity, there will be a lot of unusable space in the data files. After some time, these dead records become irrelevant as there are no transactions still around to see the old data. 
