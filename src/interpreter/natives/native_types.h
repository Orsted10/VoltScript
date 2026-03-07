#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register type system natives
void registerNativeTypes(const std::shared_ptr<Environment>& globals);

} // namespace claw
