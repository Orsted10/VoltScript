#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register advanced quantum computing natives
void registerNativeQuantumComputingAdvanced(const std::shared_ptr<Environment>& globals);

} // namespace claw
