#pragma once
#include <memory>

namespace claw {
class Environment;
void registerNativeOS(const std::shared_ptr<Environment>& globals);
} // namespace claw
