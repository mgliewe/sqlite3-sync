
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "sqlite.h"

const bool DBG=true;

Database::Database(const char *filename, uint16_t mode) : db(0), _error_msg(nullptr) {
    mode |= SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    sqlite3_open_v2(filename, &db, mode, 0);
}

Database::~Database() {
    close();
}

void Database::close() {
    clear_error();
    if(db) {
        sqlite3_close_v2(db);
        db=0;
    }
}

const char *Database::error() {
    return _error_msg ? _error_msg: "";
}

void Database::clear_error() {
    _error_msg = nullptr;
}

void Database::set_error(const char *msg) {
    _error_msg = msg;
}

Statement::Statement(Database *db) : db(db), stmt(0) {
}

Statement::~Statement() {
    finalize();
}

const char *Statement::error() {
    return db->error();
}

void Statement::clear_error() {
    db->clear_error();
}

void Statement::set_error(const char *msg) {
    db->set_error(msg);
}

bool Statement::prepare(const char *sql) {
    clear_error();
    if (stmt) {
        finalize();
    }
    const char *tail;
    size_t sz = strlen(sql);
    int result;
    result = sqlite3_prepare_v2(db->db, sql, sz, &stmt, &tail);

    if (result==SQLITE_OK) {
        return true;
    } else {
        set_error(sqlite3_errmsg(db->db));
        return false;
    }
}

bool Statement::reset() {
    if (stmt) {
        clear_error();
        int result = sqlite3_reset(stmt);
        if (result==SQLITE_OK) {
            return true;
        } else {
            set_error(sqlite3_errmsg(db->db));
            return false;
        }
    } else {
        set_error("statement is finalized");
        return false;
    }
}

bool Statement::bind_number(int column, double value) {
    if (stmt) {
        clear_error();
        int result = sqlite3_bind_double(stmt, column, value);
        if (result==SQLITE_OK) {
            return true;
        } else {
            printf("bind: %s\n", error());
            set_error(sqlite3_errmsg(db->db));
            return false;
        }
    } else {
        set_error("statement is finalized");
        return false;
    }
}

bool Statement::bind_text(int column, char *text) {
    if (stmt) {
        clear_error();
        size_t sz = strlen(text);
        char *txt = (char *) sqlite3_malloc(sz+1);
        memcpy(txt, text, sz);
        txt[sz] = 0;
        int result = sqlite3_bind_text(stmt, column, txt, sz, sqlite3_free);
        if (result==SQLITE_OK) {
            return true;
        } else {
            printf("bind: %s\n", error());
            set_error(sqlite3_errmsg(db->db));
            return false;
        }
    } else {
        set_error("statement is finalized");
        return false;
    }
}

bool Statement::bind_blob(int column, void *buf, size_t sz) {
    if (stmt) {
        clear_error();
        void *blob= sqlite3_malloc(sz);
        memcpy(blob, buf, sz);
        int result = sqlite3_bind_blob(stmt, column, blob, sz, sqlite3_free);
        if (result==SQLITE_OK) {
            return true;
        } else {
            printf("bind: %s\n", error());
            set_error(sqlite3_errmsg(db->db));
            return false;
        }
    } else {
        set_error("statement is finalized");
        return false;
    }
}

bool Statement::bind_null(int column) {
    if (stmt) {
        clear_error();
        int result = sqlite3_bind_null(stmt, column);
        if (result==SQLITE_OK) {
            return true;
        } else {
            printf("bind: %s\n", error());
            set_error(sqlite3_errmsg(db->db));
            return false;
        }
    } else {
        set_error("statement is finalized");
        return false;
    }
}

int Statement::step() {
    if (stmt) {
        clear_error();
        int ret = sqlite3_step(stmt);
        if (ret == SQLITE_ROW) {
            return 1;
        }
        else if (ret == SQLITE_DONE) {
            return 0;
        } else {
            set_error(sqlite3_errmsg(db->db));
            return -1;
        }
                        
    } else {
        set_error("statement is finalized");
        return false;
    }
}

bool Statement::finalize() {
    if (stmt) {
        clear_error();
        sqlite3_finalize(stmt);
    }
    stmt = nullptr;
    return true;
}

int Statement::column_count() {
    if (stmt) {
        clear_error();
        return sqlite3_column_count(stmt);
    } else {
        set_error("statement is finalized");
    }
    return 0;
}

const char *Statement::column_name(int column) {
    if (stmt) {
        clear_error();
        return sqlite3_column_name(stmt, column);
    } else {
        set_error("statement is finalized");
    }
    return "";
}

int Statement::column_type(int column) {
    if (stmt) {
        clear_error();
        return sqlite3_column_type(stmt, column);
    } else {
        set_error("statement is finalized");
    }
    return SQLITE_NULL;
}

double Statement::get_double(int column) {
    if (stmt) {
        clear_error();
        return sqlite3_column_double(stmt, column);
    } else {
        set_error("statement is finalized");
    }
    return 0.0;
}

int Statement::get_int(int column) {
    if (stmt) {
        return sqlite3_column_int(stmt, column);
    } else {
        set_error("statement is finalized");
    }
    return 0;
}

const char * Statement::get_text(int column, size_t &sz) {
    if (stmt) {
        clear_error();
        const char *result = (const char *)sqlite3_column_text(stmt, column);
        sz = strlen(result);
        return result;
    } else {
        set_error("statement is finalized");
        sz=0;
        return "";
    }
}

const void * Statement::get_blob(int column, size_t &sz) {
    if (stmt) {
        clear_error();
        const void *result = sqlite3_column_blob(stmt, column);
        sz = sqlite3_column_bytes(stmt, column);
        return result;
    } else {
        set_error("statement is finalized");
        sz=0;
        return "";
    }
}


