## Data Types
### Numeric Types

Name | Storage Size | Description | Range
------------ | ------------- | ------------- | -------------
serial | 4 bytes | autoincrementing integer | 1 to 2147483647

## Reliability and the Write-Ahead Log
### Write-Ahead Logging (WAL)
**Write-Ahead Logging** (WAL) is a standard method for ensuring data integrity. Briefly, WAL's central concept is that changes to data files (where tables and indexes reside) must be written only after those changes have been logged, that is, after log records describing the changes have been flushed to permanent storage. If we follow this procedure, we do not need to flush data pages to disk on every transaction commit, because we know that in the event of a crash we will be able to recover the database using the log: any changes that have not been applied to the data pages can be redone from the log records.

Using WAL results in a significantly reduced number of disk writes, because only the log file needs to be flushed to disk to guarantee that a transaction is committed, rather than every data file changed by the transaction.

### WAL Configuration

There are several WAL-related configuration parameters that affect database performance.

**Checkpoints** are points in the sequence of transactions at which it is guaranteed that the heap and index data files have been updated with all information written before that checkpoint. At checkpoint time, all dirty data pages are flushed to disk and a special checkpoint record is written to the log file. (The change records were previously flushed to the WAL files.) In the event of a crash, the crash recovery procedure looks at the latest checkpoint record to determine the point in the log from which it should start the REDO operation. Any changes made to data files before that point are guaranteed to be already on disk. Hence, after a checkpoint, log segments preceding the one containing the redo record are no longer needed and can be recycled or removed.

The checkpoint requirement of flushing all dirty data pages to disk can cause a significant I/O load.

The server's checkpointer process automatically performs a checkpoint every so often. A checkpoint is begun every checkpoint_segments log segments, or every checkpoint_timeout seconds, whichever comes first.

## SQL Commands
### CREATE TABLE
#### Parameters
##### Storage Parameters
- fillfactor (integer)
  - The fillfactor for a table is a percentage between 10 and 100. 100 (complete packing) is the default. When a smaller fillfactor is specified, INSERT operations pack table pages only to the indicated percentage; the remaining space on each page is reserved for updating rows on that page. This gives UPDATE a chance to place the updated copy of a row on the same page as the original, which is more efficient than placing it on a different page. For a table whose entries are never updated, complete packing is the best choice, but in heavily updated tables smaller fillfactors are appropriate.

> Assume that we delete a few records from a table. PostgresSQL does not immediately remove the deleted tuples from the data files. These are marked as deleted. Similarly, when a record is updated, it's roughly equivalent to one delete and one insert. The previous version of the record continues to be in the data file. The reason is simple: there can be active transactions, which want to see the data as it was before. As a result of this activity, there will be a lot of unusable space in the data files. After some time, these dead records become irrelevant as there are no transactions still around to see the old data. 

### COPY
#### Description
COPY moves data between PostgreSQL tables and standard file-system files.

> Example 4-19 shows the contents of a file that was output in ASCII format by PostgreSQL. The file in Example 4-19 is comma-delimited and uses `\null` to represent `NULL` values. It contains row data from the Book Town `subjects` table.
>
> ```
> 1,Business,Productivity Ave
> 2,Children's Books,Kids Ct
> 3,Classics,Academic Rd
> 4,Computers,Productivity Ave
> 5,Cooking,Creativity St
> 12,Religion,\null
> 8,History,Academic Rd
> 9,Horror,Black Raven Dr
> 10,Mystery,Black Raven Dr
> 11,Poetry,Sunset Dr
> 13,Romance,Main St
> 14,Science,Productivity Ave
> 15,Science Fiction,Main St
> 0,Arts,Creativity St
> 6,Drama,Main St
> 7,Entertainment,Main St
> ```
>
> The statement in Example 4-20 copies the file (`/tmp/subjects.sql`) into a table within the `booktown` database’s `subjects` table.
>
> ```
> booktown=# COPY subjects FROM '/tmp/subjects.sql'
> booktown-#               USING DELIMITERS ',' WITH NULL AS '\null';
> COPY
> ```

## Additional Supplied Programs
### pgbench
##### What is the "Transaction" Actually Performed in pgbench?
The default transaction script issues seven commands per transaction:

1. BEGIN;
2. UPDATE pgbench_accounts SET abalance = abalance + :delta WHERE aid = :aid;
3. SELECT abalance FROM pgbench_accounts WHERE aid = :aid;
4. UPDATE pgbench_tellers SET tbalance = tbalance + :delta WHERE tid = :tid;
5. UPDATE pgbench_branches SET bbalance = bbalance + :delta WHERE bid = :bid;
6. INSERT INTO pgbench_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP);
7. END;

##### Custom Scripts
pgbench has support for running custom benchmark scenarios by replacing the default transaction script (described above) with a transaction script read from a file (`-f` option). In this case a "transaction" counts as one execution of a script file. You can even specify multiple scripts (multiple `-f` options), in which case a random one of the scripts is chosen each time a client session starts a new transaction.

The format of a script file is one SQL command per line; multiline SQL commands are not supported. Script file lines can also be "meta commands", which are interpreted by pgbench itself, as described below.

Script file meta commands begin with a backslash.

- `\setrandom varname min max`
   - Sets variable ***varname*** to a random integer value between the limits ***min*** and ***max*** inclusive.

As an example, the full definition of the built-in TPC-B-like transaction is:
```
\set nbranches :scale
\set ntellers 10 * :scale
\set naccounts 100000 * :scale
\setrandom aid 1 :naccounts
\setrandom bid 1 :nbranches
\setrandom tid 1 :ntellers
\setrandom delta -5000 5000
BEGIN;
UPDATE pgbench_accounts SET abalance = abalance + :delta WHERE aid = :aid;
SELECT abalance FROM pgbench_accounts WHERE aid = :aid;
UPDATE pgbench_tellers SET tbalance = tbalance + :delta WHERE tid = :tid;
UPDATE pgbench_branches SET bbalance = bbalance + :delta WHERE bid = :bid;
INSERT INTO pgbench_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP);
END;
```
This script allows each iteration of the transaction to reference different, randomly-chosen rows.

## PostgreSQL Client Applications
### psql
#### Name
psql --  PostgreSQL interactive terminal
#### Options
```
-c command
     Specifies that psql is to execute one command string, command, and then exit. 

-d dbname
     Specifies the name of the database to connect to. 
```

## Frontend/Backend Protocol
### Overview
The protocol has separate phases for startup and normal operation. In the startup phase, the frontend opens a connection to the server and authenticates itself to the satisfaction of the server. (This might involve a single message, or multiple messages depending on the authentication method being used.) If all goes well, the server then sends status information to the frontend, and finally enters normal operation.

### Message Flow
#### Simple Query
A simple query cycle is initiated by the frontend sending a Query message to the backend. The message includes an SQL command (or commands) expressed as a text string. The backend then sends one or more response messages depending on the contents of the query command string, and finally a ReadyForQuery response message. ReadyForQuery informs the frontend that it can safely send a new command. (It is not actually necessary for the frontend to wait for ReadyForQuery before issuing another command, but the frontend must then take responsibility for figuring out what happens if the earlier command fails and already-issued later commands succeed.)

The possible response messages from the backend are:

- CommandComplete
  - An SQL command completed normally.
- RowDescription
  - Indicates that rows are about to be returned in response to a `SELECT`, `FETCH`, etc query. The contents of this message describe the column layout of the rows. This will be followed by a DataRow message for each row being returned to the frontend.
- DataRow
  - One of the set of rows returned by a `SELECT`, `FETCH`, etc query.
- ReadyForQuery
  - Processing of the query string is complete. A separate message is sent to indicate this because the query string might contain multiple SQL commands. (CommandComplete marks the end of processing one SQL command, not the whole string.) ReadyForQuery will always be sent, whether processing terminates successfully or with an error.

The response to a `SELECT` query (or other queries that return row sets, such as `EXPLAIN` or `SHOW`) normally consists of RowDescription, zero or more DataRow messages, and then CommandComplete. All other query types normally produce only a CommandComplete message.

Since a query string could contain several queries (separated by semicolons), there might be several such response sequences before the backend finishes processing the query string. ReadyForQuery is issued when the entire string has been processed and the backend is ready to accept a new query string.

## Database Physical Storage
### Database File Layout

Item | Description
------------ | -------------
base | Subdirectory containing per-database subdirectories

For each database in the cluster there is a subdirectory within `PGDATA/base`, named after the database's OID in `pg_database`. This subdirectory is the default location for the database's files; in particular, its system catalogs are stored there.

Each table and index is stored in a separate file. For ordinary relations, these files are named after the table or index's *filenode* number, which can be found in `pg_class.relfilenode`.
