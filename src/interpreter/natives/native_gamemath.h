#pragma once
#include <memory>
#include "interpreter/value.h"

namespace claw {
class Environment;

// Register math.vec2, math.vec3, math.mat4, math.quat, math.color
// and enhanced math functions (clamp, lerp, smoothstep, etc.)
// into a 'math' namespace hashmap registered as global 'math'.
void registerNativeGameMath(const std::shared_ptr<Environment>& globals);

// Internal constructors (used by other natives)
Value makeVec2(double x, double y);
Value makeVec3(double x, double y, double z);
Value makeVec4(double x, double y, double z, double w);
Value makeQuat(double x, double y, double z, double w);

} // namespace claw
