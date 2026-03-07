#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register advanced cybersecurity natives
void registerNativeCybersecurity(const std::shared_ptr<Environment>& globals);

} // namespace claw
