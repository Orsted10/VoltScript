#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register nanotechnology natives
void registerNativeNanotech(const std::shared_ptr<Environment>& globals);

} // namespace claw
