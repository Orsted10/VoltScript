#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register augmented reality natives
void registerNativeAugmentedReality(const std::shared_ptr<Environment>& globals);

} // namespace claw
