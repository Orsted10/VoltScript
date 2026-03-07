#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register quantum computing natives
void registerNativeQuantum(const std::shared_ptr<Environment>& globals);

} // namespace claw
