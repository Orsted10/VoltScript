#pragma once
#include "interpreter/value.h"
#include <string>
#include <vector>
#include <stdexcept>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <ucontext.h>
#endif

namespace claw {

class Interpreter;

enum class CoroutineStatus { Suspended, Running, Dead };

/**
 * Coroutine — stackful coroutine using platform fibers.
 *
 * Windows: Windows Fibers (ConvertThreadToFiber / CreateFiber / SwitchToFiber)
 * POSIX:   ucontext_t  (getcontext / makecontext / swapcontext)
 *
 * Usage from ClawScript:
 *   let co = coroutine(fn() { yield 1; yield 2; return 3; });
 *   co.resume()   // → 1
 *   co.resume()   // → 2
 *   co.resume()   // → 3
 *   co.status()   // → "dead"
 */
class Coroutine {
public:
    explicit Coroutine(Value fn);
    ~Coroutine();

    // Resume the coroutine (or start it on first call).
    // Returns the value passed to yield / return.
    Value resume(Interpreter& interp, const std::vector<Value>& args);

    CoroutineStatus status() const { return status_; }
    std::string     statusString() const;

    // Called from within the coroutine body to yield a value.
    // Returns the value passed to the next resume() call.
    static Value doYield(Value value);

    // Thread-local pointer to the currently executing coroutine (nullptr if none).
    static thread_local Coroutine* current_;

private:
    // ---- platform fiber entry point ----
#ifdef _WIN32
    static void WINAPI fiberEntry(LPVOID param);
    LPVOID mainFiber_ = nullptr;
    LPVOID coFiber_   = nullptr;
#else
    static void contextEntry(uint32_t hi, uint32_t lo);
    ucontext_t mainCtx_{};
    ucontext_t coCtx_{};
    static constexpr size_t STACK_SIZE = 1024 * 1024; // 1 MiB
    std::vector<char> stack_;
#endif

    Value           fn_;
    CoroutineStatus status_       = CoroutineStatus::Suspended;
    Value           yieldedValue_ = 0;
    Value           resumeValue_  = 0;
    bool            hasError_     = false;
    std::string     error_;
    Interpreter*    interp_       = nullptr;
    std::vector<Value> resumeArgs_;
    bool            firstResume_  = true;

    // Saved environments for scope isolation across fiber switches
    std::shared_ptr<class Environment> savedCallerEnv_;  // caller's env (restored after yield)
    std::shared_ptr<class Environment> savedCoroEnv_;    // coroutine's env (restored on resume)
};

} // namespace claw
