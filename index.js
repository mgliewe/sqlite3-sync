
var binary = require('node-pre-gyp');
var path = require('path')
var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
var binding = require(binding_path);

class Database {

    constructor(filename, mode=0) {
        this.db = new binding.Database(filename, mode);
        this._closed = false;
    }

    close() {
        if (!this._closed)
            this.db.close();
        this._closed=true;
    }

    run(sql, param) {
        let stm = this.prepare(sql);
        let ret = stm.run(param);
        stm.finalize();
        return ret;
    }

    get(sql, param) {
        let stm = this.prepare(sql);
        let ret = stm.get(param);
        stm.finalize();
        return ret;
    }
    
    all(sql, param) {
        let stm = this.prepare(sql);
        let ret = stm.all(param);
        stm.finalize();
        return ret;
    }

    each(sql, param, callback, complete) {
        let stm = this.prepare(sql);
        let ret = stm.each(param, callback, () => {
            stm.finalize();
            complete();
        });
        return ret;
    }

    exec(sql, param) {
        let stm = this.prepare(sql);
        let ret = stm.run(param);
        stm.finalize();
        return ret;
    }

    prepare(sql, param) {
        let stmt = new Statement(this.db);
        stmt.prepare(sql);
        if (param) 
            stmt.bind(param);
        return stmt;
    }
}

class Statement {

    constructor(db) {
        this.stm = new binding.Statement(db);
    }

    prepare(sql) {
        this.stm.prepare(sql);
    }

    bind(param) {
        this.stm.reset();
        if (Array.isArray(param)) {
            this.stm.bind(param);
        }
    }

    reset() {
        this.stm.reset();
    }

    finalize() {
        this.stm.finalize();
    }

    run(param) {
        this.get(param);
        return null;
    }

    get(param) {
        this.bind(param);
        return this.stm.step();
    }

    all(param) {
        this.bind(param);
        let ret = [];
        let v;
        while (v = this.stm.step()) {
            ret.push(v);
        }
        return ret;
    }

    each(param, callback, complete) {
        this.bind(param);
        var v;
        while (v = this.stm.step()) {
            callback(v);
        }
        complete();
    }

}

Database.binding=binding;

module.exports = Database;
