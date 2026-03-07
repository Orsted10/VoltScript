#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register audio natives
void registerNativeAudio(const std::shared_ptr<Environment>& globals);

} // namespace claw
