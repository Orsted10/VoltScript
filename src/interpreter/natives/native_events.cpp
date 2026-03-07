#include "native_events.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "features/array.h"
#include "features/string_pool.h"
#include "interpreter/value.h"
#include "interpreter/interpreter.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>
#include <mutex>
#include <memory>

namespace claw {

extern thread_local Interpreter* tl_current_interpreter;


// ── Global event bus (singleton per process) ─────────────────────────────────

struct EventBus {
    std::mutex mtx;
    // event_name -> list of (id, handler Value)
    std::unordered_map<std::string, std::vector<std::pair<int, Value>>> listeners;
    int nextId = 1;

    static EventBus& instance() {
        static EventBus bus;
        return bus;
    }

    int on(const std::string& event, Value handler) {
        std::lock_guard<std::mutex> lk(mtx);
        int id = nextId++;
        listeners[event].emplace_back(id, handler);
        return id;
    }

    void off(int id) {
        std::lock_guard<std::mutex> lk(mtx);
        for (auto& [name, vec] : listeners) {
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                [id](const auto& p){ return p.first == id; }), vec.end());
        }
    }

    void offEvent(const std::string& event) {
        std::lock_guard<std::mutex> lk(mtx);
        listeners.erase(event);
    }

    // Returns list of handlers for the event (snapshot, no lock held during call)
    std::vector<Value> getHandlers(const std::string& event) {
        std::lock_guard<std::mutex> lk(mtx);
        std::vector<Value> out;
        auto it = listeners.find(event);
        if (it != listeners.end()) {
            for (auto& [id, h] : it->second) out.push_back(h);
        }
        return out;
    }

    void clear() {
        std::lock_guard<std::mutex> lk(mtx);
        listeners.clear();
        nextId = 1;
    }

    std::vector<std::string> eventNames() {
        std::lock_guard<std::mutex> lk(mtx);
        std::vector<std::string> names;
        for (auto& [k, _] : listeners) names.push_back(k);
        return names;
    }
};

// ── helpers ──────────────────────────────────────────────────────────────────

static std::string requireString(const Value& v, const char* fn, int idx) {
    if (!isString(v))
        throw std::runtime_error(std::string(fn) + "(): arg " + std::to_string(idx) + " must be a string");
    const char* p = asStringPtr(v);
    return p ? std::string(p) : std::string();
}

static Value makeStr(const std::string& s) {
    auto sv = StringPool::intern(s);
    return stringValue(sv.data());
}

// ── registration ─────────────────────────────────────────────────────────────

void registerNativeEvents(const std::shared_ptr<Environment>& globals) {

    // eventOn(eventName, handler) -> listenerId (number)
    // Register a handler for an event. Returns a numeric ID for later removal.
    globals->define("eventOn", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto name = requireString(args[0], "eventOn", 1);
            if (!isCallable(args[1]) && !isVMClosure(args[1]))
                throw std::runtime_error("eventOn(): arg 2 must be a function");
            int id = EventBus::instance().on(name, args[1]);
            return numberToValue((double)id);
        },
        "eventOn"
    ));

    // eventOff(listenerId) -> nil
    // Remove a specific listener by its ID.
    globals->define("eventOff", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0]))
                throw std::runtime_error("eventOff(): arg 1 must be a number (listener id)");
            int id = (int)asNumber(args[0]);
            EventBus::instance().off(id);
            return nilValue();
        },
        "eventOff"
    ));

    // eventOffAll(eventName) -> nil
    // Remove all listeners for a named event.
    globals->define("eventOffAll", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto name = requireString(args[0], "eventOffAll", 1);
            EventBus::instance().offEvent(name);
            return nilValue();
        },
        "eventOffAll"
    ));

    // eventEmit(eventName, ...args) -> number  (count of handlers called)
    // Emit an event, calling all registered handlers with the provided arguments.
    // Uses arity=-1 (variadic) — we register with arity 1 but handle extra args manually.
    globals->define("eventEmit", std::make_shared<NativeFunction>(
        -1,
        [](const std::vector<Value>& args) -> Value {
            if (args.empty())
                throw std::runtime_error("eventEmit(): requires at least 1 argument (event name)");
            auto name = requireString(args[0], "eventEmit", 1);
            // Build payload args (everything after the event name)
            std::vector<Value> payload(args.begin() + 1, args.end());
            auto handlers = EventBus::instance().getHandlers(name);
            int called = 0;
            for (auto& h : handlers) {
                if (isCallable(h)) {
                    auto fn = asCallable(h);
                    // Create a minimal interpreter reference — use a static thread-local one
                    // For now, call with empty interpreter (native-only handlers work fine)
                    // Full interpreter-backed calls require passing Interpreter& through
                    // We store a thread-local interpreter pointer set by the main loop
                    if (tl_current_interpreter) {
                        fn->call(*tl_current_interpreter, payload);
                    }
                    called++;
                }
            }
            return numberToValue((double)called);
        },
        "eventEmit"
    ));

    // eventNames() -> array<string>
    // Returns all event names that have at least one listener.
    globals->define("eventNames", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            auto names = EventBus::instance().eventNames();
            auto arr = gcAcquireArrayFromPool();
            if (!arr) arr = std::make_shared<ClawArray>();
            for (auto& n : names) {
                arr->push(makeStr(n));
            }
            return arrayValue(arr);
        },
        "eventNames"
    ));

    // eventListenerCount(eventName) -> number
    globals->define("eventListenerCount", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto name = requireString(args[0], "eventListenerCount", 1);
            auto handlers = EventBus::instance().getHandlers(name);
            return numberToValue((double)handlers.size());
        },
        "eventListenerCount"
    ));

    // eventClear() -> nil
    // Remove ALL listeners for ALL events.
    globals->define("eventClear", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            EventBus::instance().clear();
            return nilValue();
        },
        "eventClear"
    ));

    // eventOnce(eventName, handler) -> listenerId
    // Register a one-shot handler that auto-removes itself after first call.
    // Implemented as a wrapper that calls eventOff on itself after firing.
    globals->define("eventOnce", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto name = requireString(args[0], "eventOnce", 1);
            if (!isCallable(args[1]) && !isVMClosure(args[1]))
                throw std::runtime_error("eventOnce(): arg 2 must be a function");
            // We store the id in a shared_ptr<int> so the wrapper lambda can capture it
            auto idHolder = std::make_shared<int>(0);
            Value innerHandler = args[1];
            // Create a wrapper native that fires once then removes itself
            auto wrapper = std::make_shared<NativeFunction>(
                -1,
                [idHolder, innerHandler](const std::vector<Value>& callArgs) -> Value {
                    // Remove self first
                    EventBus::instance().off(*idHolder);
                    // Call inner handler
                    if (isCallable(innerHandler)) {
                            if (tl_current_interpreter) {
                            return asCallable(innerHandler)->call(*tl_current_interpreter, callArgs);
                        }
                    }
                    return nilValue();
                },
                "eventOnce_wrapper"
            );
            int id = EventBus::instance().on(name, callableValue(wrapper));
            *idHolder = id;
            return numberToValue((double)id);
        },
        "eventOnce"
    ));
}

} // namespace claw
