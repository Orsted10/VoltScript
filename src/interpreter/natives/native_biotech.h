#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register biotechnology natives
void registerNativeBiotech(const std::shared_ptr<Environment>& globals);

} // namespace claw
