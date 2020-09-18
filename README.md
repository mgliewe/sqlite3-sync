# sqlite3-sync

Synchronous Napi-bindings for libsqlite.

While the sqlite3-package for node.js is fine and usefull for server applications, its use tends to become overly complicated for simple scripting tasks..

So I immplemented a simple Napi based wrapper for libsqlite. Unlike other implementations, this package does not use libffi, mainly because i got into trouble using libffi on my target plattform. And just wrapping libsqlite myself seemed to be easier than tweaking or fixing foreign code.

## Usage

Install the package via npm:

```bash
npm install sqlite3-sync
```

and use it:

```javascript
let Sqlite = require('sqlite3-sync');
let db = Sqlte('my-db.sqlite');

db.each("SELECT name FROM customer", 
    (row) => { console.log(row.customer); },
    () => { console.log("eof); }
);
```
## Api

```javascript
let database = Sqlite(dbname, mode)
```
creates a database connection.

```javascript
database.close()
```
close the database

```javascript
row = database.get(sql_statment, arg_array)
```
prepares a statement, binds an array of arguments and returns the first resulting row.

```javascript
list = database.all(sql_statment, arg_array)
```
prepares a statement, binds an array of arguments and returns the all resulting rows as an array.

```javascript
database.run(sql_statment, arg_array)
```
prepares a statement, binds an array of arguments and runs the statement. This is equivalent to get()

```javascript
database.exec(sql_statment, arg_array)
```
prepares a statement, binds an array of arguments and runs the statement. This is equivalent to get()


```javascript
database.each(sql_statment, arg_array, function callback (row) => { }, function finally() => {} )
```
prepares a statement, binds an array of arguments and and opens a cursor for the query. 

the callback is called once for each resulting row, after that the finally callback is called.

```javascript
statement = database.prepare(sql_statment, opt_arg_array)
```
create a raw statemwnt object. This might be seldomly used, refer to index.js for further documention.

