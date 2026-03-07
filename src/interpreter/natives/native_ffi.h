#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register FFI (Foreign Function Interface) natives
void registerNativeFFI(const std::shared_ptr<Environment>& globals);

} // namespace claw
