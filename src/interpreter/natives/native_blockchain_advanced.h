#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register advanced blockchain natives
void registerNativeBlockchainAdvanced(const std::shared_ptr<Environment>& globals);

} // namespace claw
