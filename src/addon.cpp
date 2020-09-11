#include "sqlite.h"

#include <assert.h>

napi_value Init(napi_env env, napi_value exports) {
    napi_status status;

    status = napi_set_named_property(env, exports, "Database", DatabaseObject::Init(env, exports));
    assert(status == napi_ok);

    status = napi_set_named_property(env, exports, "Statement", StatementObject::Init(env, exports));
    assert(status == napi_ok);

    return exports;
}

NAPI_MODULE(sqlite, Init)
