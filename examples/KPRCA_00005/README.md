# KPRCA-00005: RCL Database

## Author information

"Brian Pak" <brian.pak@kapricasecurity.com>

### DARPA performer group
Kaprica Security (KPRCA)

## Description:

This service implements a simple custom RCL (Row-Column Language) database interface that supports basic operations to store and retrieve data.

The database parses custom RCL queries and performs appropriate actions. 
Supported queries are:

- CREATE DATABASE <database_name>
- CREATE TABLE <table_name> <primary_column> (<column_name> <column_type>,...)
- INSERT INTO <table_name> VALUES (\<value1>,\<value2>,...)
- DELETE FROM <table_name> WHERE <column_name> \<op> \<value>
- SELECT FROM <table_name> WHERE <column_name> \<op> \<value>
- STATS

#### CREATE DATABASE

CREATE DATABASE query creates and initializes a database with the given 
`database_name`.

#### CREATE TABLE

CREATE TABLE query creates and initializes a table with the given `table_name`. 
It is required to provide `primary_column` name to specify the primary key 
column that will be used for SELECT and DELETE queries. This primary key column 
name must exist in the followed list of `column_name` and `column_type` pairs. 
Each pair is separated by a comma, without spaces in between.

Valid `column_type`s are:

- **INTEGER**: 32-bit signed integer type
- **FLOAT**: floating point number type; internally, it's `double` type
- **VARCHAR**: string type; dynamically allocated string

#### INSERT INTO

INSERT INTO query inserts a row into the table specified by `table_name` where 
the column data is specified by VALUES clause. The order of the values is the 
same as one of the column orders specified when creating the table.

#### DELETE FROM

DELETE FROM query deletes from the table any row that satisfies the condition 
specified by the WHERE clause.

Valid `op`s are **=**, **<**, **<=**, **>**, **>=**.

#### SELECT FROM

SELECT FROM query selects and displays rows that satisfy the condition 
specified by the WHERE clause. Valid `op`s are same as DELETE query's.

#### STATS

STATS query displays statistics about the current instance of the database. 
Mainly, it shows database name, table name, number of rows, number of queries 
so far, column schema, primary column index, and the highest (when sorted based 
on primary column) row in the database.

The highest row is only printed when there are more than 5 rows in the database.

#### Other notes

The service only allows one database and one table that need to be created 
before process other queries. Furthermore, DELETE and SELECT queries only allow 
`column_name` to be the `primary_column` that is specified when creating a 
table.

Maximum number of columns is limited to 10.

The rows in the database are internally managed using a *splay tree* data 
structure. 

### Feature List

This database server provides basic functionality such as row insertion, row 
deletion, and lookup, with a customizable database layout. The database 
natively supports floating point numbers, integer numbers, and strings. A 
statistsics command is provided to query information to help debug server 
performance.

## Vulnerability

The vulnerability is triggered when STATS query tries to access a row that has 
been freed. This state can be achieved by crafting a series of queries that 
will yield a dangling pointer to a freed row. One way is to insert at least 6 
rows, delete the row with the highest value (based on the primary column), then 
do STATS query. Due to a bug in row deletion code where it does not update the 
highest row pointer when the current highest row is being deleted, the pointer 
that is used by STATS query handler remains dangling. Crashing the program is 
possible by forcing STATS query handler to access invalid memory block that has 
been freed. With a carefully crafted input that yield specific memory 
allocations, it may be possible to run arbitrary code.

### Generic class of vulnerability

Use After Free
NULL Dereference

### CWE Classification

CWE-416: Use After Free
CWE-476: NULL Pointer Dereference

## Challenges

This service challenges some of the non-trivial, real-world aspects of the 
program analysis:

* string processing
* complex data structure and operations (splay tree, union inside a struct,
  etc.)
* a lot of dynamic memory allocation
* specific set of conditions to reach an exploitable state

The main focus of this challenge is to test if the program analysis can reason 
about memory allocations and management of them. With many complex data 
structure, the analysis needs to be able to perform well-founded structural 
analysis to understand how the data is manipulated and accessed. Use-after-free 
type of bug is one of the hardest yet most commonly found in real-world 
applications (web browsers, etc.). This challenge provides a very simple case 
of the bug to see whether the program analysis can find such type of bugs.

### CGC Note

The NULL pointer dereference bug was originally unintended by the CB author.
It was found during CB team testing and a PoV added in the interest of a
preserving unintended bugs in the corpus.
CWE-476: NULL Pointer Dereference
