### File Organazation and Indexes
- If we know what we will generally be reading/writing, we can try to minimize the number of block accesses for "frequent" queries
- Tools:
   - Indexes (structures showing where records are located)

### Indexes as Access Paths
- A single-level index is an auxiliary file that makes it more efficient to search for a record in the data file.
- The index is usually specified on one field of the file.
- One form of an index is a file of entries **<field value, pointer to record>**, which is ordered by field value.
- The index is called an access path on the field.
- The index file usually occupies considerably less disk blocks than the data file because its entries are much smaller.
- A binary search on the index yields a pointer to the record field.

- Example: Given the following data file EMPLOYEE(NAME, SSN, ADDRESS, JOB, SAL, ...)
- Suppose that:
   - record size R = 150 bytes
   - block size B = 512 bytes
   - r = 30000 records
- Then, we get:
   - blocking factor Bfr = B div R = 512 div 150 = 3 records/block
   - number of file blocks b = (r/Bfr) = (30000/3) = 10000 blocks

- For an index on the SSN field, assume the field size V<sub>SSN</sub> = 9 bytes, assume the record pointer size P<sub>R</sub>= 7 bytes. Then:
   - index entry size R<sub>I</sub> = (V<sub>SSN</sub> + P<sub>R</sub>) = (9 + 7) = 16 bytes
   - index block factor Bfr<sub>I</sub> = B div R<sub>I</sub> = 512 div 16 = 32 entries/block
   - number of index blocks b = (r / Bfr<sub>I</sub>) = (30000 / 32) = 938 blocks
   - binary search needs log<sub>2</sub>bI = log<sub>2</sub>938 = 10 block accesses
   - This is compared to an average linear search cost of:
      - (b / 2) = 30000 / 2 = 15000 block accesses
   - **If the file records are ordered**, the binary search cost would be:
      - log<sub>2</sub>b = log<sub>2</sub>30000 = 15 block accesses

## Stored Procedures
A stored procedure is a set of SQL statements that can be stored in the server. Once this has been done, clients don't need to keep reissuing the individual statements but can refer to the stored procedure instead.

### CREATE PROCEDURE Example
```
CREATE PROCEDURE p1() SELECT * FROM t; //
```
The first part of the SQL statement that creates a stored procedure is the words "CREATE PROCEDURE". The second part is the procedure name. The name of this new procedure will be p1.

```
mysql> delimiter //

mysql> CREATE PROCEDURE simpleproc (OUT param1 INT)
    -> BEGIN
    ->   SELECT COUNT(*) INTO param1 FROM t;
    -> END//
Query OK, 0 rows affected (0.00 sec)

mysql> delimiter ;
```
The example uses the mysql client delimiter command to change the string that mysql interprets as the separator between SQL statements. The default is the semicolon character (`;`). This enables the `;` delimiter used in the procedure body to be passed through to the server rather than being interpreted by mysql itself. 

### Call the Procedure
1. Now, to call a procedure, all you have to do is enter the word CALL and then the name of the procedure and then the parentheses.

When you do this for our example procedure p1, the result will be that you see the contents of the table t on your screen.
```
mysql> CALL p1() //
+----------+
| s1       |
+----------+
|       5  |
+----------+
```

## Pattern Matching with SQL patterns

### Problem
You want to perform a pattern match, not a literal comparison.

### Solution

Use the `LIKE` operator and an SQL pattern. Or use a regular-expression pattern match.

### Discussion

The example here uses a table named `metal` that contains the following rows:
```
+----------+
| name     |
+----------+
| gold     |
| iron     |
| lead     |
| mercury  |
| platinum |
| tin      |
+----------+
```

- Strings that begin with a particular substring:
   ```
   mysql > SELECT name FROM metal WHERE name LIKE 'me%';
   +----------+
   | name     |
   +----------+
   | mercury  |
   +-----------
   ```
- String that end with a particular substring:
   ```
   mysql > SELECT name FROM metal WHERE name LIKE '%d';
   +----------+
   | name     |
   +----------+
   | gold     |
   | lead     |
   +-----------
   ```
- Strings that contain a particular substring at any position:
   ```
   mysql > SELECT name FROM metal WHERE name LIKE '%in%';
   +----------+
   | name     |
   +----------+
   | platinum |
   | tin      |
   +-----------
   ```

## Pattern Matching with Regular Expressions
`REGEXP` matching uses the pattern elements shown in the following table:

Pattern | What the pattern matches
------------ | -------------
\* | Zero or more instances of preceding element