#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register threading natives
void registerNativeThread(const std::shared_ptr<Environment>& globals);

} // namespace claw
