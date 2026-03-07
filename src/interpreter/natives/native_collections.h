#pragma once
#include <memory>

namespace claw {
class Environment;
void registerNativeCollections(const std::shared_ptr<Environment>& globals);
} // namespace claw
