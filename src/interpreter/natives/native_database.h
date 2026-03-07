#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register database natives
void registerNativeDatabase(const std::shared_ptr<Environment>& globals);

} // namespace claw
