#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register graphics natives
void registerNativeGFX(const std::shared_ptr<Environment>& globals);

} // namespace claw
