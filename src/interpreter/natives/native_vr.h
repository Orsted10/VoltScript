#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register VR natives
void registerNativeVR(const std::shared_ptr<Environment>& globals);

} // namespace claw
