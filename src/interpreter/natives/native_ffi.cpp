#include "native_ffi.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include <memory>
#include <string>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace claw {

// ============================================================
// FFI Library Handle - wraps dynamic library
// ============================================================
struct FfiLibrary {
    void* handle = nullptr;
    
    ~FfiLibrary() {
        if (handle) {
#ifdef _WIN32
            FreeLibrary((HMODULE)handle);
#else
            dlclose(handle);
#endif
        }
    }
};

// ============================================================
// Register FFI natives
// ============================================================
void registerNativeFFI(const std::shared_ptr<Environment>& globals) {

    // FFI object with methods
    auto ffi = std::make_shared<ClawHashMap>();
    
    // Load a dynamic library
    ffi->set("load", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string libName = valueToString(args[0]);
            
            auto lib = std::make_shared<FfiLibrary>();
            
#ifdef _WIN32
            lib->handle = LoadLibraryA((libName + ".dll").c_str());
#else
            lib->handle = dlopen(("lib" + libName + ".so").c_str(), RTLD_LAZY);
#endif
            
            if (!lib->handle) {
                throw std::runtime_error("Failed to load library: " + libName);
            }
            
            // Return library handle as opaque object
            auto libMap = std::make_shared<ClawHashMap>();
            libMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(lib.get())));
            
            return hashMapValue(libMap);
        },
        "ffi.load"
    )));
    
    globals->define("ffi", hashMapValue(ffi));
    
    // Convenience functions
    globals->define("ffiLoad", callableValue(std::make_shared<NativeFunction>(
        1,
        [ffi](const std::vector<Value>& args) -> Value {
            return ffi->get("load");
        },
        "ffiLoad"
    )));
}

} // namespace claw
