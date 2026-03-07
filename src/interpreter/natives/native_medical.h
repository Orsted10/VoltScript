#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register medical system natives
void registerNativeMedical(const std::shared_ptr<Environment>& globals);

} // namespace claw
