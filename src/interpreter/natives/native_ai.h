#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register AI/ML natives
void registerNativeAI(const std::shared_ptr<Environment>& globals);

} // namespace claw
