#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register advanced optimization engine natives
void registerNativeOptimization(const std::shared_ptr<Environment>& globals);

} // namespace claw
