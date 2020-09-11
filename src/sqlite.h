
#ifndef __SQLITE_H__
#define __SQLITE_H__

#include <stdint.h>
#include <node_api.h>
#include <sqlite3.h>

class Database {
    friend class Statement;
    sqlite3 *db;
    const char *_error_msg;
public:
    Database(const char *filename, uint16_t mode=0);
    ~Database();
    
    void close();

    const char *error();
    void clear_error();
    void set_error(const char *);
};

class Statement {
    Database *db;
    sqlite3_stmt *stmt;

public:
    Statement(Database *db);
    ~Statement();

    const char *error();
    void clear_error();
    void set_error(const char *);

    bool prepare(const char *sql);

    bool reset();

    bool bind_number(int column, double value);
    bool bind_text(int column, char *value);
    bool bind_blob(int column, void *value, size_t sz);
    bool bind_null(int column);

    int step();
    bool finalize();

    int column_count();

    int column_type(int col);
    const char *column_name(int col);

    int get_int(int col);
    double get_double(int col);

    const char *get_text(int col, size_t &sz);
    const void *get_blob(int col, size_t &sz);

};


class DatabaseObject : public  Database {
public:
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

private:
    napi_env env_;
    napi_ref wrapper_;
    static napi_ref constructor;

    explicit DatabaseObject(const char *filename, uint16_t mode);
    ~DatabaseObject();

    static napi_value New(napi_env env, napi_callback_info info);

    static napi_value __error(napi_env env, napi_callback_info info);
    static napi_value __close(napi_env env, napi_callback_info info);
};



class StatementObject : public Statement {
public:
    static napi_value Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);
    static napi_status NewInstance(napi_env env, int argc, napi_value *argv, napi_value *instance);
private:
    napi_env env_;
    napi_ref wrapper_;
    static napi_ref constructor;

    explicit StatementObject(DatabaseObject *db);
    ~StatementObject();

    static napi_value New(napi_env env, napi_callback_info info);

    static napi_value __prepare(napi_env env, napi_callback_info info);
    static napi_value __reset(napi_env env, napi_callback_info info);
    static napi_value __bind(napi_env env, napi_callback_info info);
    static napi_value __finalize(napi_env env, napi_callback_info info);
    static napi_value __step(napi_env env, napi_callback_info info);
};

#endif

