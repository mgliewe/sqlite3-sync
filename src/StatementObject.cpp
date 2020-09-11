
#include "sqlite.h"

#include <assert.h>
#include <stdio.h>

napi_ref StatementObject::constructor;

StatementObject::StatementObject(DatabaseObject *db) 
    : Statement(db), env_(nullptr), wrapper_(nullptr) {
}

StatementObject::~StatementObject() {
    napi_delete_reference(env_, wrapper_);
}


static char * get_string(napi_env env, napi_value value) {
    char *str;
    size_t sz;
    napi_status status;
    
    napi_valuetype valuetype;
    status = napi_typeof(env, value, &valuetype);
    assert(status == napi_ok);
    if (valuetype != napi_string) {
        return nullptr;
    }

    status = napi_get_value_string_utf8(env, value, nullptr, 0, &sz);
    if (status != napi_ok) {
        return nullptr;
    }
    
    str = new char [sz+2];
    status = napi_get_value_string_utf8(env, value, str, sz+1, &sz);
    assert(status == napi_ok);
    str[sz+1] = 0;

    return str;
}

#define DECLARE_NAPI_METHOD(name, func)                                        \
  { name, 0, func, 0, 0, 0, napi_default, 0 }


napi_value StatementObject::Init(napi_env env, napi_value exports) {
    napi_status status;

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("step", __step),
        DECLARE_NAPI_METHOD("bind", __bind),
        DECLARE_NAPI_METHOD("finalize", __finalize),
        DECLARE_NAPI_METHOD("reset", __reset),
        DECLARE_NAPI_METHOD("prepare", __prepare),
    };

    napi_value cons;
    status = napi_define_class(env, "Statement", NAPI_AUTO_LENGTH, New, nullptr, 5, properties, &cons);
    assert(status == napi_ok);

    // save constructor..
    status = napi_create_reference(env, cons, 1, &constructor);
    assert(status == napi_ok);

    return cons;
}

void StatementObject::Destructor(napi_env env, void* nativeObject, void* finalize_hint) {
    StatementObject* obj = static_cast<StatementObject *>(nativeObject);
    delete obj;

}

napi_value StatementObject::New(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value target;
    status = napi_get_new_target(env, info, &target);
    assert(status == napi_ok);

    bool is_constructor = target != nullptr;
    if (is_constructor) {
        // Invoked as constructor: `new MyObject(...)`
        size_t argc = 2;
        napi_value args[2];
        napi_value jsthis;
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        assert(status == napi_ok);

        if (argc!=1) {
            napi_throw_error(env, "EINVAL", "Statement constructor must have 1 argment.");
             return nullptr;
        }

        DatabaseObject *db = 0;
        status = napi_unwrap(env, args[0], reinterpret_cast<void**>(&db));
        assert(status == napi_ok);

        StatementObject* obj = new StatementObject(db);

        obj->env_ = env;
        status = napi_wrap(env,
            jsthis,
            reinterpret_cast<void*>(obj),
            StatementObject::Destructor,
            nullptr,  // finalize_hint
            &obj->wrapper_);
        assert(status == napi_ok);

        return jsthis;

    } else {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        size_t argc_ = 2;
        napi_value args[2];
        status = napi_get_cb_info(env, info, &argc_, args, nullptr, nullptr);
        assert(status == napi_ok);

        const size_t argc = 2;
        napi_value argv[argc] = {args[0], args[1]};

        napi_value instance, ctor;
        napi_get_reference_value(env, constructor, &ctor);
        assert(status == napi_ok);

        status = napi_new_instance(env, ctor, argc, argv, &instance);
        assert(status == napi_ok);

        return instance;
    }
}

napi_value StatementObject::__prepare(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &jsthis, nullptr);
    assert(status == napi_ok);

    char *sql = get_string(env, argv[0]);
    assert(sql!=nullptr);

    StatementObject* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    bool ret = obj->prepare(sql);
    delete [] sql;

    if (!ret) {
        napi_throw_error(env, "SQLITE", obj->error());
    }

    return nullptr;
}

napi_value StatementObject::__reset(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    StatementObject* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    bool ret = obj->StatementObject::reset();
    if (!ret) {
        napi_throw_error(env, "SQLITE", obj->error());
    }

    return nullptr;
}

napi_value StatementObject::__finalize(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    StatementObject* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    bool ret = obj->StatementObject::finalize();
    if (!ret) {
        napi_throw_error(env, "SQLITE", obj->error());
    }

    return nullptr;
}

napi_value StatementObject::__bind(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    size_t argc = 1;
    napi_value argv[1];
    status = napi_get_cb_info(env, info, &argc, argv, &jsthis, nullptr);
    assert(status == napi_ok);

    StatementObject* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);


    bool is_array;
    status = napi_is_array(env, argv[0], &is_array);
    assert(status == napi_ok);
    if (!is_array) {
        napi_throw_error(env, "EINVAL", "bind() array expected.");
        return nullptr;
    }

    uint32_t length;
    status = napi_get_array_length(env, argv[0], &length); 
    assert(status == napi_ok);

    for (uint32_t n=0; n<length; ++n) {
        napi_value element;
        status = napi_get_element(env, argv[0], n, &element);
        assert(status == napi_ok);
    
        napi_valuetype type;
        status = napi_typeof(env, element, &type);
        assert(status == napi_ok);

        napi_value value;
        char *str;
        double number;

        switch(type) {
            case napi_undefined: 
            case napi_null:
                obj->bind_null(n);
                break;
            case napi_boolean:
            case napi_number:
                status = napi_coerce_to_number(env, element, &value);
                assert(status == napi_ok);
                status = napi_get_value_double(env, value, &number);
                assert(status == napi_ok);
                obj->bind_number(n+1, number);
                break;
            case napi_string:
            case napi_symbol:
            bind_string:
                status = napi_coerce_to_string(env, element, &value);
                assert(status == napi_ok);
                str = get_string(env, value);
                if (!str) {
                    napi_throw_error(env, "EINVAL", "bind(): cant fetch string value.");
                    return nullptr;
                }

                obj->bind_text(n+1, str);
                delete [] str;
                break;

            case napi_object:
                napi_value global, date, buffer;
                status = napi_get_global(env, &global);
                assert(status == napi_ok);

                status = napi_get_named_property(env, global, "Date", &date);
                assert(status == napi_ok);
                bool is_instance;
                status = napi_instanceof(env, element, date, &is_instance);
                assert(status == napi_ok);

                if (is_instance) {
                    napi_value to_iso_func;
                    status = napi_get_named_property(env, element, "toISOString", &to_iso_func);
                    assert(status == napi_ok);
                    status = napi_call_function(env, element, to_iso_func, 0, nullptr, &element);
                    assert(status == napi_ok);
                    goto bind_string;
                }

                status = napi_get_named_property(env, global, "Buffer", &buffer);
                assert(status == napi_ok);

                status = napi_instanceof(env, element, buffer, &is_instance);
                assert(status == napi_ok);

                if (is_instance) {
                    void *bufptr;
                    size_t sz;
                    status = napi_get_buffer_info(env, element, &bufptr, &sz);
                    assert(status == napi_ok);
                    obj->bind_blob(n+1, bufptr, sz);
                    break;
                }

                napi_throw_error(env, "EINVAL", "cant bind() value.");
                return nullptr;

            case napi_function:
            case napi_external:
            default:
                napi_throw_error(env, "EINVAL", "cant bind() value.");
                return nullptr;
        }
    }
    return nullptr;
}

napi_value StatementObject::__step(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    StatementObject* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    int ret = obj->step();
    if (ret == 0) {
        return nullptr;
    } else if (ret<0) {
        const char *c_msg = obj->error();
        napi_throw_error(env, "EINVAL", c_msg);
        return nullptr;
    }

    napi_value row;
    status = napi_create_object(env, &row);
    assert(status == napi_ok);

    int length = obj->column_count();
    for (int n=0; n<length; ++n) {
        int type = obj->column_type(n);
        size_t str_sz;
        const char *str;
        size_t blob_sz;
        const void *blob;
        void *buffer;
        napi_value value;
        switch(type) {
            case SQLITE_INTEGER:
                status = napi_create_double(env, obj->get_int(n), &value);
                assert(status == napi_ok);
                break;

            case SQLITE_FLOAT:
                status = napi_create_double(env, obj->get_double(n), &value);
                assert(status == napi_ok);
                break;

            case SQLITE_TEXT:
                str = obj->get_text(n, str_sz);
                assert(str != nullptr);
                status = napi_create_string_utf8(env, str, str_sz, &value);
                assert(status == napi_ok);
                break;

            case SQLITE_BLOB:
                blob = obj->get_blob(n, blob_sz);
                assert(blob != nullptr);
                status = napi_create_buffer_copy(env, blob_sz, blob, &buffer, &value);
                assert(status == napi_ok);
                break;

            case SQLITE_NULL:
            default:
                value = nullptr;
        }
        napi_set_named_property(env, row, obj->column_name(n), value);
    }

    return row;
}
