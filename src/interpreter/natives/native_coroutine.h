#pragma once
#include <memory>

namespace claw {
class Environment;
void registerNativeCoroutine(const std::shared_ptr<Environment>& globals);
} // namespace claw
