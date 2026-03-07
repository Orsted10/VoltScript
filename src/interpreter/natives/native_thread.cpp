#include "native_thread.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <thread>
#include <future>
#include <mutex>
#include <atomic>
#include <queue>

namespace claw {

// ============================================================
// Thread Handle structure
// ============================================================
struct ThreadHandle {
    std::thread nativeThread;
    std::promise<Value> resultPromise;
    std::atomic<bool> running{false};
    
    ~ThreadHandle() {
        if (nativeThread.joinable()) {
            nativeThread.join();
        }
    }
};

// ============================================================
// Channel implementation for thread communication
// ============================================================
struct Channel {
    std::queue<Value> queue;
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> closed{false};
    
    void send(Value value) {
        std::lock_guard<std::mutex> lock(mutex);
        if (closed) {
            throw std::runtime_error("Cannot send to closed channel");
        }
        queue.push(value);
        cv.notify_one();
    }
    
    Value receive() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !queue.empty() || closed; });
        
        if (queue.empty() && closed) {
            return nilValue();
        }
        
        Value value = queue.front();
        queue.pop();
        return value;
    }
    
    void close() {
        std::lock_guard<std::mutex> lock(mutex);
        closed = true;
        cv.notify_all();
    }
};

// ============================================================
// Register threading natives
// ============================================================
void registerNativeThread(const std::shared_ptr<Environment>& globals) {

    // Thread object with methods
    auto thread = std::make_shared<ClawHashMap>();
    
    // Spawn a new thread
    thread->set("spawn", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto func = args[0];
            
            auto handle = std::make_shared<ThreadHandle>();
            handle->running = true;
            
            // Start the thread
            handle->nativeThread = std::thread([func, handle]() {
                try {
                    if (isCallable(func)) {
                        Value result = asCallable(func)->call(*nullptr, {});
                        handle->resultPromise.set_value(result);
                    } else {
                        handle->resultPromise.set_exception(
                            std::make_exception_ptr(std::runtime_error("Argument must be callable"))
                        );
                    }
                } catch (...) {
                    handle->resultPromise.set_exception(std::current_exception());
                }
                handle->running = false;
            });
            
            // Return thread handle as opaque object
            auto threadMap = std::make_shared<ClawHashMap>();
            threadMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(handle.get())));
            
            return hashMapValue(threadMap);
        },
        "thread.spawn"
    )));
    
    globals->define("thread", hashMapValue(thread));
    
    // Convenience functions
    globals->define("spawn", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto func = args[0];
            
            auto handle = std::make_shared<ThreadHandle>();
            handle->running = true;
            
            // Start the thread
            handle->nativeThread = std::thread([func, handle]() {
                try {
                    if (isCallable(func)) {
                        Value result = asCallable(func)->call(*nullptr, {});
                        handle->resultPromise.set_value(result);
                    } else {
                        handle->resultPromise.set_exception(
                            std::make_exception_ptr(std::runtime_error("Argument must be callable"))
                        );
                    }
                } catch (...) {
                    handle->resultPromise.set_exception(std::current_exception());
                }
                handle->running = false;
            });
            
            // Return thread handle as opaque object
            auto threadMap = std::make_shared<ClawHashMap>();
            threadMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(handle.get())));
            
            return hashMapValue(threadMap);
        },
        "spawn"
    )));
    
    globals->define("join", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto threadMap = asHashMap(args[0]);
            auto ptrValue = threadMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto handle = static_cast<ThreadHandle*>(ptr);
            
            if (handle->nativeThread.joinable()) {
                handle->nativeThread.join();
            }
            
            auto future = handle->resultPromise.get_future();
            return future.get();
        },
        "join"
    )));
}

} // namespace claw
