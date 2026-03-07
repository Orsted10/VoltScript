#pragma once
#include <memory>

namespace claw {
class Environment;
void registerNativeEvents(const std::shared_ptr<Environment>& globals);
} // namespace claw
