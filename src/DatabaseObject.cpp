
#include "sqlite.h"

#include <assert.h>
#include <string.h>


napi_ref DatabaseObject::constructor;

DatabaseObject::DatabaseObject(const char *filename, uint16_t mode) 
    : Database(filename, mode), env_(nullptr), wrapper_(nullptr) {
}

DatabaseObject::~DatabaseObject() {
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


napi_value DatabaseObject::Init(napi_env env, napi_value exports) {
    napi_status status;

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_METHOD("error", __error),
        DECLARE_NAPI_METHOD("close", __close),
    };

    napi_value cons;
    status = napi_define_class(env, "Database", NAPI_AUTO_LENGTH, New, 
                               nullptr, 2, properties, &cons);
    assert(status == napi_ok);

    // save constructor..
    status = napi_create_reference(env, cons, 1, &constructor);
    assert(status == napi_ok);

    return cons;
}


void DatabaseObject::Destructor(napi_env env, void* nativeObject, void* finalize_hint) {
    DatabaseObject* obj = static_cast<DatabaseObject *>(nativeObject);
    delete obj;
}

napi_value DatabaseObject::New(napi_env env, napi_callback_info info) {
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

        if (argc!=2) {
            napi_throw_error(env, "EINVAL", "Database constructor must have 2 argments.");
             return nullptr;
        }

        char *filename = get_string(env, args[0]);
        if (!filename) {
            napi_throw_error(env, "EINVAL", "Database constructor must be called with a filename.");
            return nullptr;
        }

        napi_valuetype valuetype;
        status = napi_typeof(env, args[1], &valuetype);
        assert(status == napi_ok);
        if (valuetype != napi_number) {
            napi_throw_error(env, "EINVAL", "Database constructor:  mode must be a number.");
            delete [] filename;
            return nullptr;
        }
        int32_t mode;
        napi_get_value_int32(env, args[1], &mode);
        assert(status == napi_ok);

        DatabaseObject* obj = new DatabaseObject(filename, mode);
        delete [] filename;

        obj->env_ = env;
        status = napi_wrap(env,
            jsthis,
            reinterpret_cast<void*>(obj),
            DatabaseObject::Destructor,
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

napi_value DatabaseObject::__close(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    DatabaseObject* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    obj->close();

    return nullptr;
}

napi_value DatabaseObject::__error(napi_env env, napi_callback_info info) {
    napi_status status;

    napi_value jsthis;
    status = napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    assert(status == napi_ok);

    DatabaseObject* obj;
    status = napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj));
    assert(status == napi_ok);

    const char *errmsg = obj->error();

    napi_value result;
    napi_create_string_utf8(env, errmsg, strlen(errmsg), &result);
    assert(status == napi_ok);

    return result;
}
