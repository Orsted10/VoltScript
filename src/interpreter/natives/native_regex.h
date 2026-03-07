#pragma once
#include <memory>

namespace claw {
class Environment;
void registerNativeRegex(const std::shared_ptr<Environment>& globals);
} // namespace claw
