#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register advanced mathematics natives
void registerNativeMathematics(const std::shared_ptr<Environment>& globals);

} // namespace claw
