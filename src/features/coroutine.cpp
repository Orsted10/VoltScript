#include "coroutine.h"
#include "interpreter/interpreter.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include <stdexcept>
#include <cstdint>

namespace claw {

thread_local Coroutine* Coroutine::current_ = nullptr;

// ============================================================
// Constructor / Destructor
// ============================================================

Coroutine::Coroutine(Value fn) : fn_(fn) {
#ifndef _WIN32
    stack_.resize(STACK_SIZE);
#endif
}

Coroutine::~Coroutine() {
#ifdef _WIN32
    if (coFiber_) {
        DeleteFiber(coFiber_);
        coFiber_ = nullptr;
    }
#endif
}

std::string Coroutine::statusString() const {
    switch (status_) {
        case CoroutineStatus::Suspended: return "suspended";
        case CoroutineStatus::Running:   return "running";
        case CoroutineStatus::Dead:      return "dead";
    }
    return "unknown";
}

// ============================================================
// Windows Fiber implementation
// ============================================================
#ifdef _WIN32

void WINAPI Coroutine::fiberEntry(LPVOID param) {
    Coroutine* co = static_cast<Coroutine*>(param);
    try {
        if (isCallable(co->fn_)) {
            Value result = asCallable(co->fn_)->call(*co->interp_, co->resumeArgs_);
            co->yieldedValue_ = result;
        } else {
            co->hasError_ = true;
            co->error_    = "Coroutine body is not callable";
        }
    } catch (const std::exception& e) {
        co->hasError_ = true;
        co->error_    = e.what();
    } catch (...) {
        co->hasError_ = true;
        co->error_    = "Unknown error in coroutine";
    }
    co->status_ = CoroutineStatus::Dead;
    SwitchToFiber(co->mainFiber_);
    // Should never reach here
}

Value Coroutine::resume(Interpreter& interp, const std::vector<Value>& args) {
    if (status_ == CoroutineStatus::Dead) {
        throw std::runtime_error("Cannot resume a dead coroutine");
    }
    if (status_ == CoroutineStatus::Running) {
        throw std::runtime_error("Cannot resume a running coroutine");
    }

    interp_      = &interp;
    resumeArgs_  = args;
    resumeValue_ = args.empty() ? nilValue() : args[0];

    if (firstResume_) {
        firstResume_ = false;
        // Convert main thread to fiber (no-op if already a fiber)
        mainFiber_ = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
        if (!mainFiber_) mainFiber_ = GetCurrentFiber();
        // Create coroutine fiber
        coFiber_ = CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, fiberEntry, this);
        if (!coFiber_) throw std::runtime_error("Failed to create coroutine fiber");
    }

    // ── Scope isolation ──────────────────────────────────────────────────────
    savedCallerEnv_ = interp.getEnvironment();
    if (savedCoroEnv_) {
        interp.setEnvironment(savedCoroEnv_);
        savedCoroEnv_ = nullptr;
    }
    // ─────────────────────────────────────────────────────────────────────────

    status_ = CoroutineStatus::Running;
    Coroutine* prev = current_;
    current_ = this;
    SwitchToFiber(coFiber_);
    current_ = prev;

    // ── Restore caller scope ─────────────────────────────────────────────────
    interp.setEnvironment(savedCallerEnv_);
    savedCallerEnv_ = nullptr;
    // ─────────────────────────────────────────────────────────────────────────

    if (hasError_) throw std::runtime_error(error_);
    return yieldedValue_;
}

Value Coroutine::doYield(Value value) {
    if (!current_) throw std::runtime_error("yield called outside of a coroutine");
    Coroutine* co = current_;
    co->yieldedValue_ = value;
    co->status_       = CoroutineStatus::Suspended;

    // Save the coroutine's current environment so resume() can restore it.
    co->savedCoroEnv_ = co->interp_->getEnvironment();

    SwitchToFiber(co->mainFiber_);
    // Execution resumes here after next resume() — environment already restored.
    return co->resumeValue_;
}

// ============================================================
// POSIX ucontext implementation
// ============================================================
#else

void Coroutine::contextEntry(uint32_t hi, uint32_t lo) {
    // Reconstruct pointer from two 32-bit halves (portable on 32/64-bit)
    uintptr_t ptr = (static_cast<uintptr_t>(hi) << 32) | static_cast<uintptr_t>(lo);
    Coroutine* co = reinterpret_cast<Coroutine*>(ptr);
    try {
        if (isCallable(co->fn_)) {
            Value result = asCallable(co->fn_)->call(*co->interp_, co->resumeArgs_);
            co->yieldedValue_ = result;
        } else {
            co->hasError_ = true;
            co->error_    = "Coroutine body is not callable";
        }
    } catch (const std::exception& e) {
        co->hasError_ = true;
        co->error_    = e.what();
    } catch (...) {
        co->hasError_ = true;
        co->error_    = "Unknown error in coroutine";
    }
    co->status_ = CoroutineStatus::Dead;
    swapcontext(&co->coCtx_, &co->mainCtx_);
}

Value Coroutine::resume(Interpreter& interp, const std::vector<Value>& args) {
    if (status_ == CoroutineStatus::Dead) {
        throw std::runtime_error("Cannot resume a dead coroutine");
    }
    if (status_ == CoroutineStatus::Running) {
        throw std::runtime_error("Cannot resume a running coroutine");
    }

    interp_      = &interp;
    resumeArgs_  = args;
    resumeValue_ = args.empty() ? nilValue() : args[0];

    if (firstResume_) {
        firstResume_ = false;
        getcontext(&coCtx_);
        coCtx_.uc_stack.ss_sp   = stack_.data();
        coCtx_.uc_stack.ss_size = STACK_SIZE;
        coCtx_.uc_link          = nullptr;
        uintptr_t ptr = reinterpret_cast<uintptr_t>(this);
        uint32_t  hi  = static_cast<uint32_t>(ptr >> 32);
        uint32_t  lo  = static_cast<uint32_t>(ptr & 0xFFFFFFFF);
        makecontext(&coCtx_, reinterpret_cast<void(*)()>(contextEntry), 2, hi, lo);
    }

    savedCallerEnv_ = interp.getEnvironment();
    if (savedCoroEnv_) {
        interp.setEnvironment(savedCoroEnv_);
        savedCoroEnv_ = nullptr;
    }

    status_ = CoroutineStatus::Running;
    Coroutine* prev = current_;
    current_ = this;
    swapcontext(&mainCtx_, &coCtx_);
    current_ = prev;

    interp.setEnvironment(savedCallerEnv_);
    savedCallerEnv_ = nullptr;

    if (hasError_) throw std::runtime_error(error_);
    return yieldedValue_;
}

Value Coroutine::doYield(Value value) {
    if (!current_) throw std::runtime_error("yield called outside of a coroutine");
    Coroutine* co = current_;
    co->yieldedValue_ = value;
    co->status_       = CoroutineStatus::Suspended;
    co->savedCoroEnv_ = co->interp_->getEnvironment();
    swapcontext(&co->coCtx_, &co->mainCtx_);
    return co->resumeValue_;
}

#endif // _WIN32

} // namespace claw
