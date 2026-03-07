#pragma once
#include "interpreter/value.h"
#include "interpreter/environment.h"

namespace claw {

// Register WebSocket natives
void registerNativeWebSocket(const std::shared_ptr<Environment>& globals);

} // namespace claw
