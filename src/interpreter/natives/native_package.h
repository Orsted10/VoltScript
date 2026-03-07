#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register package manager natives
void registerNativePackage(const std::shared_ptr<Environment>& globals);

} // namespace claw
