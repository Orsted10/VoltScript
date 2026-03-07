#include "native_http.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace claw {

// ============================================================
// Register HTTP natives
// ============================================================
void registerNativeHTTP(const std::shared_ptr<Environment>& globals) {

    // HTTP object with methods
    auto http = std::make_shared<ClawHashMap>();
    
    // HTTP GET method (stub for now)
    http->set("get", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            // Return mock response for now
            auto result = std::make_shared<ClawHashMap>();
            result->set("status", numberToValue(200));
            result->set("body", stringValue(StringPool::intern("Mock HTTP response").data()));
            
            auto headersResult = std::make_shared<ClawHashMap>();
            headersResult->set("Content-Type", stringValue(StringPool::intern("text/plain").data()));
            result->set("headers", hashMapValue(headersResult));
            
            return hashMapValue(result);
        },
        "http.get"
    )));
    
    // HTTP POST method (stub for now)
    http->set("post", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            // Return mock response for now
            auto result = std::make_shared<ClawHashMap>();
            result->set("status", numberToValue(201));
            result->set("body", stringValue(StringPool::intern("Mock POST response").data()));
            
            auto headersResult = std::make_shared<ClawHashMap>();
            headersResult->set("Content-Type", stringValue(StringPool::intern("text/plain").data()));
            result->set("headers", hashMapValue(headersResult));
            
            return hashMapValue(result);
        },
        "http.post"
    )));
    
    globals->define("http", hashMapValue(http));
    
    // Convenience functions
    globals->define("httpGet", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto emptyHeaders = std::make_shared<ClawHashMap>();
            auto result = std::make_shared<ClawHashMap>();
            result->set("status", numberToValue(200));
            result->set("body", stringValue(StringPool::intern("Mock GET response").data()));
            return hashMapValue(result);
        },
        "httpGet"
    )));
    
    globals->define("httpPost", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto result = std::make_shared<ClawHashMap>();
            result->set("status", numberToValue(201));
            result->set("body", stringValue(StringPool::intern("Mock POST response").data()));
            return hashMapValue(result);
        },
        "httpPost"
    )));
}

} // namespace claw
