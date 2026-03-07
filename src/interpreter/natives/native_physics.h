#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register advanced physics engine natives
void registerNativePhysics(const std::shared_ptr<Environment>& globals);

} // namespace claw
