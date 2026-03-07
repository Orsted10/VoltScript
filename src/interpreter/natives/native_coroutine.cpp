#include "native_coroutine.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/coroutine.h"
#include "features/hashmap.h"
#include "features/string_pool.h"
#include <memory>
#include <string>

namespace claw {

// ============================================================
// CoroutineObject — wraps a Coroutine and exposes methods
// as a hash map so ClawScript can call co.resume(), co.status()
// ============================================================
static Value makeCoroutineObject(std::shared_ptr<Coroutine> co) {
    auto map = std::make_shared<ClawHashMap>();

    // co.resume(...args) → yielded/returned value
    map->set("resume", callableValue(std::make_shared<NativeFunction>(
        -1,
        [co](const std::vector<Value>& args) -> Value {
            // We need the interpreter — use a trampoline via NativeFunction
            // that captures the interpreter reference at call time.
            // NativeFunction lambdas don't receive the interpreter, so we
            // use a custom Callable subclass.
            (void)args;
            throw std::runtime_error("Internal: use CoroutineResumable");
            return nilValue();
        },
        "resume"
    )));

    // Replace with a proper Callable that receives the interpreter
    struct ResumeCallable : public Callable {
        std::shared_ptr<Coroutine> co;
        explicit ResumeCallable(std::shared_ptr<Coroutine> c) : co(std::move(c)) {}
        Value call(Interpreter& interp, const std::vector<Value>& args) override {
            return co->resume(interp, args);
        }
        int arity() const override { return -1; }
        std::string toString() const override { return "<coroutine.resume>"; }
    };
    map->set("resume", callableValue(std::make_shared<ResumeCallable>(co)));

    // co.status() → "suspended" | "running" | "dead"
    map->set("status", callableValue(std::make_shared<NativeFunction>(
        0,
        [co](const std::vector<Value>&) -> Value {
            auto sv = StringPool::intern(co->statusString());
            return stringValue(sv.data());
        },
        "status"
    )));

    // co.isDead() → bool
    map->set("isDead", callableValue(std::make_shared<NativeFunction>(
        0,
        [co](const std::vector<Value>&) -> Value {
            return boolValue(co->status() == CoroutineStatus::Dead);
        },
        "isDead"
    )));

    // co.isAlive() → bool
    map->set("isAlive", callableValue(std::make_shared<NativeFunction>(
        0,
        [co](const std::vector<Value>&) -> Value {
            return boolValue(co->status() != CoroutineStatus::Dead);
        },
        "isAlive"
    )));

    return hashMapValue(map);
}

// ============================================================
// Register coroutine natives
// ============================================================
void registerNativeCoroutine(const std::shared_ptr<Environment>& globals) {

    // coroutine(fn) → coroutine object
    globals->define("coroutine", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isCallable(args[0])) {
                throw std::runtime_error("coroutine() requires a function argument");
            }
            auto co = std::make_shared<Coroutine>(args[0]);
            return makeCoroutineObject(co);
        },
        "coroutine"
    )));

    // isCoroutine(val) → bool  (checks if it's a coroutine hashmap)
    // We can't distinguish easily, so just expose it as a utility
    globals->define("isCoroutine", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) return boolValue(false);
            auto map = asHashMap(args[0]);
            return boolValue(map->contains("resume") && map->contains("status"));
        },
        "isCoroutine"
    )));

    // wrap(fn) → fn that returns a coroutine each time it's called
    // Useful for generator-style iteration
    struct WrapCallable : public Callable {
        Value fn;
        explicit WrapCallable(Value f) : fn(f) {}
        Value call(Interpreter& interp, const std::vector<Value>& args) override {
            (void)interp; (void)args;
            auto co = std::make_shared<Coroutine>(fn);
            return makeCoroutineObject(co);
        }
        int arity() const override { return 0; }
        std::string toString() const override { return "<coroutine.wrap>"; }
    };

    globals->define("coWrap", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isCallable(args[0])) throw std::runtime_error("coWrap() requires a function");
            // Return a callable that creates a fresh coroutine each call
            Value fn = args[0];
            struct FactoryCallable : public Callable {
                Value fn;
                explicit FactoryCallable(Value f) : fn(f) {}
                Value call(Interpreter&, const std::vector<Value>&) override {
                    auto co = std::make_shared<Coroutine>(fn);
                    return makeCoroutineObject(co);
                }
                int arity() const override { return 0; }
                std::string toString() const override { return "<coWrap factory>"; }
            };
            return callableValue(std::make_shared<FactoryCallable>(fn));
        },
        "coWrap"
    )));
}

} // namespace claw
