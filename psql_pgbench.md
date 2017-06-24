## psql --  PostgreSQL interactive terminal
### Options
```
-c command
     Specifies that psql is to execute one command string, command, and then exit. 

-d dbname
     Specifies the name of the database to connect to. 
```

## pgbench
### Notes
#### What is the "Transaction" Actually Performed in pgbench?
The default transaction script issues seven commands per transaction:

1. BEGIN;

2. UPDATE pgbench_accounts SET abalance = abalance + :delta WHERE aid = :aid;

3. SELECT abalance FROM pgbench_accounts WHERE aid = :aid;

4. UPDATE pgbench_tellers SET tbalance = tbalance + :delta WHERE tid = :tid;

5. UPDATE pgbench_branches SET bbalance = bbalance + :delta WHERE bid = :bid;

6. INSERT INTO pgbench_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP);

7. END;

#### Custom Scripts
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