#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register HTTP networking natives
void registerNativeHTTP(const std::shared_ptr<Environment>& globals);

} // namespace claw
