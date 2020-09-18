# sqlite3-sync

Synchronous Napi-bindings for libsqlite.

While the sqlite3-package for node.js is fine and usefull for server applications, its use tends to become overly complicated for simple scripting purposes..

So I immplemented a simple Napi based wrapper to libsqlite. Unlike othe rimplementation this package does not use libffi, mainly because i got into trouble using libffi on my target platform, and just wrapping libsqlite seemed to be easier than tweaking fixing foreign code.

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




