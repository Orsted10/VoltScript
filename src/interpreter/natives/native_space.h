#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register space/astronomy natives
void registerNativeSpace(const std::shared_ptr<Environment>& globals);

} // namespace claw
