#include "native_gamemath.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "features/hashmap.h"
#include "features/array.h"
#include "features/string_pool.h"
#include "interpreter/value.h"
#include "interpreter/gc_alloc.h"
#include <cmath>
#include <limits>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace claw {

// ============================================================
// Helpers
// ============================================================

static Value mkStr(const std::string& s) {
    return stringValue(StringPool::intern(s).data());
}

static double numF(const std::shared_ptr<ClawHashMap>& m, const std::string& k) {
    Value v = m->get(k);
    if (!isNumber(v)) throw std::runtime_error("Expected numeric field '" + k + "'");
    return asNumber(v);
}

static std::string fmtN(double v) {
    if (v == std::floor(v) && std::abs(v) < 1e15) {
        std::ostringstream o; o << static_cast<long long>(v); return o.str();
    }
    std::ostringstream o; o << std::setprecision(6) << v; return o.str();
}

// Forward declarations
Value makeVec2(double x, double y);
Value makeVec3(double x, double y, double z);
Value makeVec4(double x, double y, double z, double w);
Value makeQuat(double x, double y, double z, double w);
static Value makeMat4(const double m[16]);

// ============================================================
// vec2
// ============================================================

Value makeVec2(double x, double y) {
    auto map = gcNewHashMap();
    map->set("x", numberToValue(x));
    map->set("y", numberToValue(y));
    map->set("_type", mkStr("vec2"));
    std::weak_ptr<ClawHashMap> wm = map;

    map->set("length", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"); return numberToValue(std::sqrt(x*x+y*y));
    }, "length")));
    map->set("lengthSq", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"); return numberToValue(x*x+y*y);
    }, "lengthSq")));
    map->set("add", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec2.add: expected vec2");
        auto o = asHashMap(a[0]);
        return makeVec2(numF(m,"x")+numF(o,"x"), numF(m,"y")+numF(o,"y"));
    }, "add")));
    map->set("sub", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec2.sub: expected vec2");
        auto o = asHashMap(a[0]);
        return makeVec2(numF(m,"x")-numF(o,"x"), numF(m,"y")-numF(o,"y"));
    }, "sub")));
    map->set("mul", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isNumber(a[0])) throw std::runtime_error("vec2.mul: expected number");
        double s=asNumber(a[0]); return makeVec2(numF(m,"x")*s, numF(m,"y")*s);
    }, "mul")));
    map->set("div", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isNumber(a[0])) throw std::runtime_error("vec2.div: expected number");
        double s=asNumber(a[0]); if (s==0.0) throw std::runtime_error("vec2.div: division by zero");
        return makeVec2(numF(m,"x")/s, numF(m,"y")/s);
    }, "div")));
    map->set("dot", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec2.dot: expected vec2");
        auto o = asHashMap(a[0]);
        return numberToValue(numF(m,"x")*numF(o,"x") + numF(m,"y")*numF(o,"y"));
    }, "dot")));
    map->set("normalize", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), len=std::sqrt(x*x+y*y);
        if (len==0.0) return makeVec2(0,0); return makeVec2(x/len, y/len);
    }, "normalize")));
    map->set("distance", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec2.distance: expected vec2");
        auto o = asHashMap(a[0]);
        double dx=numF(m,"x")-numF(o,"x"), dy=numF(m,"y")-numF(o,"y");
        return numberToValue(std::sqrt(dx*dx+dy*dy));
    }, "distance")));
    map->set("lerp", callableValue(std::make_shared<NativeFunction>(2, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec2.lerp: expected vec2");
        auto o = asHashMap(a[0]); double t=asNumber(a[1]);
        double sx=numF(m,"x"), sy=numF(m,"y");
        return makeVec2(sx+(numF(o,"x")-sx)*t, sy+(numF(o,"y")-sy)*t);
    }, "lerp")));
    map->set("negate", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec2(-numF(m,"x"), -numF(m,"y"));
    }, "negate")));
    map->set("angle", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return numberToValue(std::atan2(numF(m,"y"), numF(m,"x")));
    }, "angle")));
    map->set("angleTo", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec2.angleTo: expected vec2");
        auto o = asHashMap(a[0]);
        return numberToValue(std::atan2(numF(o,"y")-numF(m,"y"), numF(o,"x")-numF(m,"x")));
    }, "angleTo")));
    map->set("rotate", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double angle=asNumber(a[0]), x=numF(m,"x"), y=numF(m,"y");
        double c=std::cos(angle), s=std::sin(angle);
        return makeVec2(x*c-y*s, x*s+y*c);
    }, "rotate")));
    map->set("perpendicular", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec2(-numF(m,"y"), numF(m,"x"));
    }, "perpendicular")));
    map->set("clamp", callableValue(std::make_shared<NativeFunction>(2, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double mn=asNumber(a[0]), mx=asNumber(a[1]);
        return makeVec2(std::clamp(numF(m,"x"),mn,mx), std::clamp(numF(m,"y"),mn,mx));
    }, "clamp")));
    map->set("abs", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec2(std::abs(numF(m,"x")), std::abs(numF(m,"y")));
    }, "abs")));
    map->set("floor", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec2(std::floor(numF(m,"x")), std::floor(numF(m,"y")));
    }, "floor")));
    map->set("ceil", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec2(std::ceil(numF(m,"x")), std::ceil(numF(m,"y")));
    }, "ceil")));
    map->set("round", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec2(std::round(numF(m,"x")), std::round(numF(m,"y")));
    }, "round")));
    map->set("equals", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) return boolValue(false);
        auto o = asHashMap(a[0]);
        return boolValue(numF(m,"x")==numF(o,"x") && numF(m,"y")==numF(o,"y"));
    }, "equals")));
    map->set("toString", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return mkStr("vec2("+fmtN(numF(m,"x"))+", "+fmtN(numF(m,"y"))+")");
    }, "toString")));
    map->set("toArray", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        auto arr = gcNewArray(); arr->push(m->get("x")); arr->push(m->get("y"));
        return arrayValue(arr);
    }, "toArray")));
    map->set("clone", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec2(numF(m,"x"), numF(m,"y"));
    }, "clone")));
    return hashMapValue(map);
}

// ============================================================
// vec3
// ============================================================

Value makeVec3(double x, double y, double z) {
    auto map = gcNewHashMap();
    map->set("x", numberToValue(x));
    map->set("y", numberToValue(y));
    map->set("z", numberToValue(z));
    map->set("_type", mkStr("vec3"));
    std::weak_ptr<ClawHashMap> wm = map;

    map->set("length", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), z=numF(m,"z"); return numberToValue(std::sqrt(x*x+y*y+z*z));
    }, "length")));
    map->set("lengthSq", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), z=numF(m,"z"); return numberToValue(x*x+y*y+z*z);
    }, "lengthSq")));
    map->set("add", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec3.add: expected vec3");
        auto o = asHashMap(a[0]);
        return makeVec3(numF(m,"x")+numF(o,"x"), numF(m,"y")+numF(o,"y"), numF(m,"z")+numF(o,"z"));
    }, "add")));
    map->set("sub", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec3.sub: expected vec3");
        auto o = asHashMap(a[0]);
        return makeVec3(numF(m,"x")-numF(o,"x"), numF(m,"y")-numF(o,"y"), numF(m,"z")-numF(o,"z"));
    }, "sub")));
    map->set("mul", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isNumber(a[0])) throw std::runtime_error("vec3.mul: expected number");
        double s=asNumber(a[0]); return makeVec3(numF(m,"x")*s, numF(m,"y")*s, numF(m,"z")*s);
    }, "mul")));
    map->set("div", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isNumber(a[0])) throw std::runtime_error("vec3.div: expected number");
        double s=asNumber(a[0]); if (s==0.0) throw std::runtime_error("vec3.div: division by zero");
        return makeVec3(numF(m,"x")/s, numF(m,"y")/s, numF(m,"z")/s);
    }, "div")));
    map->set("dot", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec3.dot: expected vec3");
        auto o = asHashMap(a[0]);
        return numberToValue(numF(m,"x")*numF(o,"x")+numF(m,"y")*numF(o,"y")+numF(m,"z")*numF(o,"z"));
    }, "dot")));
    map->set("cross", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec3.cross: expected vec3");
        auto o = asHashMap(a[0]);
        double ax=numF(m,"x"), ay=numF(m,"y"), az=numF(m,"z");
        double bx=numF(o,"x"), by=numF(o,"y"), bz=numF(o,"z");
        return makeVec3(ay*bz-az*by, az*bx-ax*bz, ax*by-ay*bx);
    }, "cross")));
    map->set("normalize", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), z=numF(m,"z"), len=std::sqrt(x*x+y*y+z*z);
        if (len==0.0) return makeVec3(0,0,0); return makeVec3(x/len, y/len, z/len);
    }, "normalize")));
    map->set("distance", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec3.distance: expected vec3");
        auto o = asHashMap(a[0]);
        double dx=numF(m,"x")-numF(o,"x"), dy=numF(m,"y")-numF(o,"y"), dz=numF(m,"z")-numF(o,"z");
        return numberToValue(std::sqrt(dx*dx+dy*dy+dz*dz));
    }, "distance")));
    map->set("lerp", callableValue(std::make_shared<NativeFunction>(2, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec3.lerp: expected vec3");
        auto o = asHashMap(a[0]); double t=asNumber(a[1]);
        double sx=numF(m,"x"), sy=numF(m,"y"), sz=numF(m,"z");
        return makeVec3(sx+(numF(o,"x")-sx)*t, sy+(numF(o,"y")-sy)*t, sz+(numF(o,"z")-sz)*t);
    }, "lerp")));
    map->set("negate", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec3(-numF(m,"x"), -numF(m,"y"), -numF(m,"z"));
    }, "negate")));
    map->set("reflect", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec3.reflect: expected vec3 normal");
        auto n = asHashMap(a[0]);
        double ix=numF(m,"x"), iy=numF(m,"y"), iz=numF(m,"z");
        double nx=numF(n,"x"), ny=numF(n,"y"), nz=numF(n,"z");
        double d=2.0*(ix*nx+iy*ny+iz*nz);
        return makeVec3(ix-d*nx, iy-d*ny, iz-d*nz);
    }, "reflect")));
    map->set("project", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec3.project: expected vec3");
        auto o = asHashMap(a[0]);
        double ax=numF(m,"x"), ay=numF(m,"y"), az=numF(m,"z");
        double bx=numF(o,"x"), by=numF(o,"y"), bz=numF(o,"z");
        double bLenSq=bx*bx+by*by+bz*bz;
        if (bLenSq==0.0) return makeVec3(0,0,0);
        double t=(ax*bx+ay*by+az*bz)/bLenSq;
        return makeVec3(bx*t, by*t, bz*t);
    }, "project")));
    map->set("abs", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec3(std::abs(numF(m,"x")), std::abs(numF(m,"y")), std::abs(numF(m,"z")));
    }, "abs")));
    map->set("floor", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec3(std::floor(numF(m,"x")), std::floor(numF(m,"y")), std::floor(numF(m,"z")));
    }, "floor")));
    map->set("ceil", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec3(std::ceil(numF(m,"x")), std::ceil(numF(m,"y")), std::ceil(numF(m,"z")));
    }, "ceil")));
    map->set("equals", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) return boolValue(false);
        auto o = asHashMap(a[0]);
        return boolValue(numF(m,"x")==numF(o,"x") && numF(m,"y")==numF(o,"y") && numF(m,"z")==numF(o,"z"));
    }, "equals")));
    map->set("toString", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return mkStr("vec3("+fmtN(numF(m,"x"))+", "+fmtN(numF(m,"y"))+", "+fmtN(numF(m,"z"))+")");
    }, "toString")));
    map->set("toArray", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        auto arr = gcNewArray(); arr->push(m->get("x")); arr->push(m->get("y")); arr->push(m->get("z"));
        return arrayValue(arr);
    }, "toArray")));
    map->set("clone", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec3(numF(m,"x"), numF(m,"y"), numF(m,"z"));
    }, "clone")));
    map->set("toVec2", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec2(numF(m,"x"), numF(m,"y"));
    }, "toVec2")));
    return hashMapValue(map);
}

// ============================================================
// vec4
// ============================================================

Value makeVec4(double x, double y, double z, double w) {
    auto map = gcNewHashMap();
    map->set("x", numberToValue(x)); map->set("y", numberToValue(y));
    map->set("z", numberToValue(z)); map->set("w", numberToValue(w));
    map->set("_type", mkStr("vec4"));
    std::weak_ptr<ClawHashMap> wm = map;

    map->set("length", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), z=numF(m,"z"), w=numF(m,"w");
        return numberToValue(std::sqrt(x*x+y*y+z*z+w*w));
    }, "length")));
    map->set("add", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec4.add: expected vec4");
        auto o = asHashMap(a[0]);
        return makeVec4(numF(m,"x")+numF(o,"x"), numF(m,"y")+numF(o,"y"), numF(m,"z")+numF(o,"z"), numF(m,"w")+numF(o,"w"));
    }, "add")));
    map->set("dot", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec4.dot: expected vec4");
        auto o = asHashMap(a[0]);
        return numberToValue(numF(m,"x")*numF(o,"x")+numF(m,"y")*numF(o,"y")+numF(m,"z")*numF(o,"z")+numF(m,"w")*numF(o,"w"));
    }, "dot")));
    map->set("normalize", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), z=numF(m,"z"), w=numF(m,"w");
        double len=std::sqrt(x*x+y*y+z*z+w*w);
        if (len==0.0) return makeVec4(0,0,0,0); return makeVec4(x/len, y/len, z/len, w/len);
    }, "normalize")));
    map->set("lerp", callableValue(std::make_shared<NativeFunction>(2, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("vec4.lerp: expected vec4");
        auto o = asHashMap(a[0]); double t=asNumber(a[1]);
        double sx=numF(m,"x"), sy=numF(m,"y"), sz=numF(m,"z"), sw=numF(m,"w");
        return makeVec4(sx+(numF(o,"x")-sx)*t, sy+(numF(o,"y")-sy)*t, sz+(numF(o,"z")-sz)*t, sw+(numF(o,"w")-sw)*t);
    }, "lerp")));
    map->set("toString", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return mkStr("vec4("+fmtN(numF(m,"x"))+", "+fmtN(numF(m,"y"))+", "+fmtN(numF(m,"z"))+", "+fmtN(numF(m,"w"))+")");
    }, "toString")));
    map->set("clone", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeVec4(numF(m,"x"), numF(m,"y"), numF(m,"z"), numF(m,"w"));
    }, "clone")));
    return hashMapValue(map);
}

// ============================================================
// quat  (x, y, z, w)
// ============================================================

Value makeQuat(double x, double y, double z, double w) {
    auto map = gcNewHashMap();
    map->set("x", numberToValue(x)); map->set("y", numberToValue(y));
    map->set("z", numberToValue(z)); map->set("w", numberToValue(w));
    map->set("_type", mkStr("quat"));
    std::weak_ptr<ClawHashMap> wm = map;

    map->set("length", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), z=numF(m,"z"), w=numF(m,"w");
        return numberToValue(std::sqrt(x*x+y*y+z*z+w*w));
    }, "length")));
    map->set("normalize", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), z=numF(m,"z"), w=numF(m,"w");
        double len=std::sqrt(x*x+y*y+z*z+w*w);
        if (len==0.0) return makeQuat(0,0,0,1);
        return makeQuat(x/len, y/len, z/len, w/len);
    }, "normalize")));
    map->set("conjugate", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeQuat(-numF(m,"x"), -numF(m,"y"), -numF(m,"z"), numF(m,"w"));
    }, "conjugate")));
    map->set("multiply", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("quat.multiply: expected quat");
        auto o = asHashMap(a[0]);
        double ax=numF(m,"x"), ay=numF(m,"y"), az=numF(m,"z"), aw=numF(m,"w");
        double bx=numF(o,"x"), by=numF(o,"y"), bz=numF(o,"z"), bw=numF(o,"w");
        return makeQuat(
            aw*bx + ax*bw + ay*bz - az*by,
            aw*by - ax*bz + ay*bw + az*bx,
            aw*bz + ax*by - ay*bx + az*bw,
            aw*bw - ax*bx - ay*by - az*bz
        );
    }, "multiply")));
    map->set("rotate", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("quat.rotate: expected vec3");
        auto v = asHashMap(a[0]);
        double qx=numF(m,"x"), qy=numF(m,"y"), qz=numF(m,"z"), qw=numF(m,"w");
        double vx=numF(v,"x"), vy=numF(v,"y"), vz=numF(v,"z");
        // v' = q * v * q^-1
        double tx = 2.0*(qy*vz - qz*vy);
        double ty = 2.0*(qz*vx - qx*vz);
        double tz = 2.0*(qx*vy - qy*vx);
        return makeVec3(vx + qw*tx + qy*tz - qz*ty,
                       vy + qw*ty + qz*tx - qx*tz,
                       vz + qw*tz + qx*ty - qy*tx);
    }, "rotate")));
    map->set("slerp", callableValue(std::make_shared<NativeFunction>(2, [wm](const std::vector<Value>& a) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        if (!isHashMap(a[0])) throw std::runtime_error("quat.slerp: expected quat");
        auto o = asHashMap(a[0]); double t=asNumber(a[1]);
        double ax=numF(m,"x"), ay=numF(m,"y"), az=numF(m,"z"), aw=numF(m,"w");
        double bx=numF(o,"x"), by=numF(o,"y"), bz=numF(o,"z"), bw=numF(o,"w");
        double dot = ax*bx+ay*by+az*bz+aw*bw;
        if (dot < 0.0) { bx=-bx; by=-by; bz=-bz; bw=-bw; dot=-dot; }
        if (dot > 0.9995) {
            return makeQuat(ax+(bx-ax)*t, ay+(by-ay)*t, az+(bz-az)*t, aw+(bw-aw)*t);
        }
        double theta0=std::acos(dot), theta=theta0*t;
        double sinTheta=std::sin(theta), sinTheta0=std::sin(theta0);
        double s0=std::cos(theta)-dot*sinTheta/sinTheta0, s1=sinTheta/sinTheta0;
        return makeQuat(ax*s0+bx*s1, ay*s0+by*s1, az*s0+bz*s1, aw*s0+bw*s1);
    }, "slerp")));
    map->set("toEuler", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        double x=numF(m,"x"), y=numF(m,"y"), z=numF(m,"z"), w=numF(m,"w");
        double sinr_cosp = 2.0*(w*x+y*z);
        double cosr_cosp = 1.0-2.0*(x*x+y*y);
        double roll = std::atan2(sinr_cosp, cosr_cosp);
        double sinp = 2.0*(w*y-z*x);
        double pitch = std::abs(sinp)>=1.0 ? std::copysign(1.5707963267948966, sinp) : std::asin(sinp);
        double siny_cosp = 2.0*(w*z+x*y);
        double cosy_cosp = 1.0-2.0*(y*y+z*z);
        double yaw = std::atan2(siny_cosp, cosy_cosp);
        return makeVec3(roll, pitch, yaw);
    }, "toEuler")));
    map->set("toString", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return mkStr("quat("+fmtN(numF(m,"x"))+", "+fmtN(numF(m,"y"))+", "+fmtN(numF(m,"z"))+", "+fmtN(numF(m,"w"))+")");
    }, "toString")));
    map->set("clone", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
        auto m = wm.lock(); if (!m) return nilValue();
        return makeQuat(numF(m,"x"), numF(m,"y"), numF(m,"z"), numF(m,"w"));
    }, "clone")));
    return hashMapValue(map);
}

// ============================================================
// mat4 — column-major 4x4 matrix stored as ClawArray of 16 doubles
// ============================================================

static Value makeMat4(const double d[16]) {
    auto arr = gcNewArray();
    for (int i = 0; i < 16; i++) arr->push(numberToValue(d[i]));
    return arrayValue(arr);
}

static double mat4Get(const std::shared_ptr<ClawArray>& a, int i) {
    return asNumber(a->get(i));
}

static Value buildMat4Namespace() {
    auto ns = gcNewHashMap();

    ns->set("identity", callableValue(std::make_shared<NativeFunction>(0, [](const std::vector<Value>&) -> Value {
        double m[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        return makeMat4(m);
    }, "identity")));

    ns->set("multiply", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        if (!isArray(a[0]) || !isArray(a[1])) throw std::runtime_error("mat4.multiply: expected mat4 arrays");
        auto A = asArray(a[0]), B = asArray(a[1]);
        double r[16] = {};
        for (int row=0; row<4; row++)
            for (int col=0; col<4; col++)
                for (int k=0; k<4; k++)
                    r[col*4+row] += mat4Get(A, k*4+row) * mat4Get(B, col*4+k);
        return makeMat4(r);
    }, "multiply")));

    ns->set("translate", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        if (!isArray(a[0])) throw std::runtime_error("mat4.translate: expected mat4");
        if (!isHashMap(a[1])) throw std::runtime_error("mat4.translate: expected vec3");
        auto M = asArray(a[0]); auto v = asHashMap(a[1]);
        double m[16]; for (int i=0;i<16;i++) m[i]=mat4Get(M,i);
        double tx=numF(v,"x"), ty=numF(v,"y"), tz=numF(v,"z");
        m[12] += m[0]*tx + m[4]*ty + m[8]*tz;
        m[13] += m[1]*tx + m[5]*ty + m[9]*tz;
        m[14] += m[2]*tx + m[6]*ty + m[10]*tz;
        m[15] += m[3]*tx + m[7]*ty + m[11]*tz;
        return makeMat4(m);
    }, "translate")));

    ns->set("scale", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        if (!isArray(a[0])) throw std::runtime_error("mat4.scale: expected mat4");
        auto M = asArray(a[0]);
        double m[16]; for (int i=0;i<16;i++) m[i]=mat4Get(M,i);
        double sx, sy, sz;
        if (isHashMap(a[1])) {
            auto v = asHashMap(a[1]); sx=numF(v,"x"); sy=numF(v,"y"); sz=numF(v,"z");
        } else { sx=sy=sz=asNumber(a[1]); }
        for (int i=0;i<4;i++) { m[i]*=sx; m[4+i]*=sy; m[8+i]*=sz; }
        return makeMat4(m);
    }, "scale")));

    ns->set("rotateX", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        if (!isArray(a[0])) throw std::runtime_error("mat4.rotateX: expected mat4");
        auto M = asArray(a[0]); double angle=asNumber(a[1]);
        double c=std::cos(angle), s=std::sin(angle);
        double r[16] = {1,0,0,0, 0,c,s,0, 0,-s,c,0, 0,0,0,1};
        // multiply M * R
        double res[16]={};
        for (int row=0;row<4;row++) for (int col=0;col<4;col++) for (int k=0;k<4;k++)
            res[col*4+row] += mat4Get(M,k*4+row)*r[col*4+k];
        return makeMat4(res);
    }, "rotateX")));

    ns->set("rotateY", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        if (!isArray(a[0])) throw std::runtime_error("mat4.rotateY: expected mat4");
        auto M = asArray(a[0]); double angle=asNumber(a[1]);
        double c=std::cos(angle), s=std::sin(angle);
        double r[16] = {c,0,-s,0, 0,1,0,0, s,0,c,0, 0,0,0,1};
        double res[16]={};
        for (int row=0;row<4;row++) for (int col=0;col<4;col++) for (int k=0;k<4;k++)
            res[col*4+row] += mat4Get(M,k*4+row)*r[col*4+k];
        return makeMat4(res);
    }, "rotateY")));

    ns->set("rotateZ", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        if (!isArray(a[0])) throw std::runtime_error("mat4.rotateZ: expected mat4");
        auto M = asArray(a[0]); double angle=asNumber(a[1]);
        double c=std::cos(angle), s=std::sin(angle);
        double r[16] = {c,s,0,0, -s,c,0,0, 0,0,1,0, 0,0,0,1};
        double res[16]={};
        for (int row=0;row<4;row++) for (int col=0;col<4;col++) for (int k=0;k<4;k++)
            res[col*4+row] += mat4Get(M,k*4+row)*r[col*4+k];
        return makeMat4(res);
    }, "rotateZ")));

    ns->set("perspective", callableValue(std::make_shared<NativeFunction>(4, [](const std::vector<Value>& a) -> Value {
        double fov=asNumber(a[0]), aspect=asNumber(a[1]), near=asNumber(a[2]), far=asNumber(a[3]);
        double f = 1.0/std::tan(fov*0.5);
        double nf = 1.0/(near-far);
        double m[16] = {
            f/aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (far+near)*nf, -1,
            0, 0, 2*far*near*nf, 0
        };
        return makeMat4(m);
    }, "perspective")));

    ns->set("ortho", callableValue(std::make_shared<NativeFunction>(6, [](const std::vector<Value>& a) -> Value {
        double l=asNumber(a[0]), r=asNumber(a[1]), b=asNumber(a[2]);
        double t=asNumber(a[3]), n=asNumber(a[4]), f=asNumber(a[5]);
        double m[16] = {
            2/(r-l), 0, 0, 0,
            0, 2/(t-b), 0, 0,
            0, 0, -2/(f-n), 0,
            -(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1
        };
        return makeMat4(m);
    }, "ortho")));

    ns->set("lookAt", callableValue(std::make_shared<NativeFunction>(3, [](const std::vector<Value>& a) -> Value {
        if (!isHashMap(a[0])||!isHashMap(a[1])||!isHashMap(a[2]))
            throw std::runtime_error("mat4.lookAt: expected vec3 eye, center, up");
        auto eye=asHashMap(a[0]), center=asHashMap(a[1]), up=asHashMap(a[2]);
        double ex=numF(eye,"x"), ey=numF(eye,"y"), ez=numF(eye,"z");
        double cx=numF(center,"x"), cy=numF(center,"y"), cz=numF(center,"z");
        double ux=numF(up,"x"), uy=numF(up,"y"), uz=numF(up,"z");
        double fx=ex-cx, fy=ey-cy, fz=ez-cz;
        double fl=std::sqrt(fx*fx+fy*fy+fz*fz); fx/=fl; fy/=fl; fz/=fl;
        double rx=uy*fz-uz*fy, ry=uz*fx-ux*fz, rz=ux*fy-uy*fx;
        double rl=std::sqrt(rx*rx+ry*ry+rz*rz); rx/=rl; ry/=rl; rz/=rl;
        double ux2=fy*rz-fz*ry, uy2=fz*rx-fx*rz, uz2=fx*ry-fy*rx;
        double m[16] = {
            rx, ux2, fx, 0,
            ry, uy2, fy, 0,
            rz, uz2, fz, 0,
            -(rx*ex+ry*ey+rz*ez), -(ux2*ex+uy2*ey+uz2*ez), -(fx*ex+fy*ey+fz*ez), 1
        };
        return makeMat4(m);
    }, "lookAt")));

    ns->set("transformPoint", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        if (!isArray(a[0])||!isHashMap(a[1])) throw std::runtime_error("mat4.transformPoint: expected mat4, vec3");
        auto M=asArray(a[0]); auto v=asHashMap(a[1]);
        double x=numF(v,"x"), y=numF(v,"y"), z=numF(v,"z");
        double rx=mat4Get(M,0)*x+mat4Get(M,4)*y+mat4Get(M,8)*z+mat4Get(M,12);
        double ry=mat4Get(M,1)*x+mat4Get(M,5)*y+mat4Get(M,9)*z+mat4Get(M,13);
        double rz=mat4Get(M,2)*x+mat4Get(M,6)*y+mat4Get(M,10)*z+mat4Get(M,14);
        double rw=mat4Get(M,3)*x+mat4Get(M,7)*y+mat4Get(M,11)*z+mat4Get(M,15);
        if (rw!=0.0&&rw!=1.0) { rx/=rw; ry/=rw; rz/=rw; }
        return makeVec3(rx, ry, rz);
    }, "transformPoint")));

    ns->set("transpose", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        if (!isArray(a[0])) throw std::runtime_error("mat4.transpose: expected mat4");
        auto M=asArray(a[0]);
        double m[16];
        for (int r=0;r<4;r++) for (int c=0;c<4;c++) m[r*4+c]=mat4Get(M,c*4+r);
        return makeMat4(m);
    }, "transpose")));

    return hashMapValue(ns);
}

// ============================================================
// registerNativeGameMath — builds the 'math' global namespace
// ============================================================

void registerNativeGameMath(const std::shared_ptr<Environment>& globals) {
    auto mathNs = gcNewHashMap();

    // Constants (literal values to avoid _USE_MATH_DEFINES ordering issues on MSVC)
    static constexpr double CLAW_PI    = 3.14159265358979323846;
    static constexpr double CLAW_E     = 2.71828182845904523536;
    static constexpr double CLAW_SQRT2 = 1.41421356237309504880;
    mathNs->set("PI",       numberToValue(CLAW_PI));
    mathNs->set("TAU",      numberToValue(2.0*CLAW_PI));
    mathNs->set("E",        numberToValue(CLAW_E));
    mathNs->set("SQRT2",    numberToValue(CLAW_SQRT2));
    mathNs->set("INFINITY", numberToValue(std::numeric_limits<double>::infinity()));
    mathNs->set("NAN",      numberToValue(std::numeric_limits<double>::quiet_NaN()));
    mathNs->set("DEG2RAD",  numberToValue(CLAW_PI/180.0));
    mathNs->set("RAD2DEG",  numberToValue(180.0/CLAW_PI));

    // Enhanced scalar math
    mathNs->set("clamp", callableValue(std::make_shared<NativeFunction>(3, [](const std::vector<Value>& a) -> Value {
        double v=asNumber(a[0]), mn=asNumber(a[1]), mx=asNumber(a[2]);
        return numberToValue(std::clamp(v, mn, mx));
    }, "clamp")));
    mathNs->set("lerp", callableValue(std::make_shared<NativeFunction>(3, [](const std::vector<Value>& a) -> Value {
        double x=asNumber(a[0]), y=asNumber(a[1]), t=asNumber(a[2]);
        return numberToValue(x + (y-x)*t);
    }, "lerp")));
    mathNs->set("smoothstep", callableValue(std::make_shared<NativeFunction>(3, [](const std::vector<Value>& a) -> Value {
        double e0=asNumber(a[0]), e1=asNumber(a[1]), x=asNumber(a[2]);
        double t = std::clamp((x-e0)/(e1-e0), 0.0, 1.0);
        return numberToValue(t*t*(3.0-2.0*t));
    }, "smoothstep")));
    mathNs->set("smootherstep", callableValue(std::make_shared<NativeFunction>(3, [](const std::vector<Value>& a) -> Value {
        double e0=asNumber(a[0]), e1=asNumber(a[1]), x=asNumber(a[2]);
        double t = std::clamp((x-e0)/(e1-e0), 0.0, 1.0);
        return numberToValue(t*t*t*(t*(t*6.0-15.0)+10.0));
    }, "smootherstep")));
    mathNs->set("sign", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        double v=asNumber(a[0]); return numberToValue(v>0?1.0:v<0?-1.0:0.0);
    }, "sign")));
    mathNs->set("fract", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        double v=asNumber(a[0]); return numberToValue(v - std::floor(v));
    }, "fract")));
    mathNs->set("map", callableValue(std::make_shared<NativeFunction>(5, [](const std::vector<Value>& a) -> Value {
        double v=asNumber(a[0]), inMin=asNumber(a[1]), inMax=asNumber(a[2]);
        double outMin=asNumber(a[3]), outMax=asNumber(a[4]);
        return numberToValue(outMin + (v-inMin)/(inMax-inMin)*(outMax-outMin));
    }, "map")));
    mathNs->set("isPrime", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        long long n = static_cast<long long>(asNumber(a[0]));
        if (n < 2) return boolValue(false);
        if (n == 2) return boolValue(true);
        if (n%2==0) return boolValue(false);
        for (long long i=3; i*i<=n; i+=2) if (n%i==0) return boolValue(false);
        return boolValue(true);
    }, "isPrime")));
    mathNs->set("gcd", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        long long x=std::abs(static_cast<long long>(asNumber(a[0])));
        long long y=std::abs(static_cast<long long>(asNumber(a[1])));
        while (y) { x%=y; std::swap(x,y); }
        return numberToValue(static_cast<double>(x));
    }, "gcd")));
    mathNs->set("lcm", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        long long x=std::abs(static_cast<long long>(asNumber(a[0])));
        long long y=std::abs(static_cast<long long>(asNumber(a[1])));
        if (x==0||y==0) return numberToValue(0.0);
        long long g=x; long long tmp=y; while (tmp) { g%=tmp; std::swap(g,tmp); }
        return numberToValue(static_cast<double>(x/g*y));
    }, "lcm")));
    mathNs->set("factorial", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        int n=static_cast<int>(asNumber(a[0]));
        if (n<0) throw std::runtime_error("math.factorial: negative input");
        double r=1.0; for (int i=2;i<=n;i++) r*=i;
        return numberToValue(r);
    }, "factorial")));
    mathNs->set("atan2", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::atan2(asNumber(a[0]), asNumber(a[1])));
    }, "atan2")));
    mathNs->set("asin", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::asin(asNumber(a[0])));
    }, "asin")));
    mathNs->set("acos", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::acos(asNumber(a[0])));
    }, "acos")));
    mathNs->set("atan", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::atan(asNumber(a[0])));
    }, "atan")));
    mathNs->set("log2", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::log2(asNumber(a[0])));
    }, "log2")));
    mathNs->set("log10", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::log10(asNumber(a[0])));
    }, "log10")));
    mathNs->set("hypot", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::hypot(asNumber(a[0]), asNumber(a[1])));
    }, "hypot")));
    mathNs->set("trunc", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::trunc(asNumber(a[0])));
    }, "trunc")));
    mathNs->set("cbrt", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(std::cbrt(asNumber(a[0])));
    }, "cbrt")));
    mathNs->set("isNaN", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        if (!isNumber(a[0])) return boolValue(false);
        return boolValue(std::isnan(asNumber(a[0])));
    }, "isNaN")));
    mathNs->set("isFinite", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        if (!isNumber(a[0])) return boolValue(false);
        return boolValue(std::isfinite(asNumber(a[0])));
    }, "isFinite")));
    mathNs->set("toRadians", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(asNumber(a[0]) * 3.14159265358979323846 / 180.0);
    }, "toRadians")));
    mathNs->set("toDegrees", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        return numberToValue(asNumber(a[0]) * 180.0 / 3.14159265358979323846);
    }, "toDegrees")));
    mathNs->set("pingpong", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        double t=asNumber(a[0]), len=asNumber(a[1]);
        if (len==0.0) return numberToValue(0.0);
        double L=2.0*len, t2=t-L*std::floor(t/L);
        return numberToValue(len - std::abs(t2 - len));
    }, "pingpong")));
    mathNs->set("repeat", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        double t=asNumber(a[0]), len=asNumber(a[1]);
        if (len==0.0) return numberToValue(0.0);
        return numberToValue(t - std::floor(t/len)*len);
    }, "repeat")));

    // Vector constructors
    mathNs->set("vec2", callableValue(std::make_shared<NativeFunction>(-1, [](const std::vector<Value>& a) -> Value {
        double x=0, y=0;
        if (a.size()>=1 && isNumber(a[0])) x=asNumber(a[0]);
        if (a.size()>=2 && isNumber(a[1])) y=asNumber(a[1]);
        if (a.size()==1 && isNumber(a[0])) y=x; // vec2(s) → vec2(s,s)
        return makeVec2(x, y);
    }, "vec2")));
    mathNs->set("vec3", callableValue(std::make_shared<NativeFunction>(-1, [](const std::vector<Value>& a) -> Value {
        double x=0, y=0, z=0;
        if (a.size()>=1 && isNumber(a[0])) x=asNumber(a[0]);
        if (a.size()>=2 && isNumber(a[1])) y=asNumber(a[1]);
        if (a.size()>=3 && isNumber(a[2])) z=asNumber(a[2]);
        if (a.size()==1 && isNumber(a[0])) { y=x; z=x; } // vec3(s) → vec3(s,s,s)
        return makeVec3(x, y, z);
    }, "vec3")));
    mathNs->set("vec4", callableValue(std::make_shared<NativeFunction>(-1, [](const std::vector<Value>& a) -> Value {
        double x=0, y=0, z=0, w=1;
        if (a.size()>=1 && isNumber(a[0])) x=asNumber(a[0]);
        if (a.size()>=2 && isNumber(a[1])) y=asNumber(a[1]);
        if (a.size()>=3 && isNumber(a[2])) z=asNumber(a[2]);
        if (a.size()>=4 && isNumber(a[3])) w=asNumber(a[3]);
        return makeVec4(x, y, z, w);
    }, "vec4")));

    // Quaternion constructors
    auto quatNs = gcNewHashMap();
    quatNs->set("create", callableValue(std::make_shared<NativeFunction>(-1, [](const std::vector<Value>& a) -> Value {
        double x=0, y=0, z=0, w=1;
        if (a.size()>=4) { x=asNumber(a[0]); y=asNumber(a[1]); z=asNumber(a[2]); w=asNumber(a[3]); }
        return makeQuat(x, y, z, w);
    }, "new")));
    quatNs->set("identity", callableValue(std::make_shared<NativeFunction>(0, [](const std::vector<Value>&) -> Value {
        return makeQuat(0, 0, 0, 1);
    }, "identity")));
    quatNs->set("fromEuler", callableValue(std::make_shared<NativeFunction>(3, [](const std::vector<Value>& a) -> Value {
        double rx=asNumber(a[0])*0.5, ry=asNumber(a[1])*0.5, rz=asNumber(a[2])*0.5;
        double cx=std::cos(rx), sx=std::sin(rx);
        double cy=std::cos(ry), sy=std::sin(ry);
        double cz=std::cos(rz), sz=std::sin(rz);
        return makeQuat(
            sx*cy*cz - cx*sy*sz,
            cx*sy*cz + sx*cy*sz,
            cx*cy*sz - sx*sy*cz,
            cx*cy*cz + sx*sy*sz
        );
    }, "fromEuler")));
    quatNs->set("fromAxisAngle", callableValue(std::make_shared<NativeFunction>(2, [](const std::vector<Value>& a) -> Value {
        if (!isHashMap(a[0])) throw std::runtime_error("quat.fromAxisAngle: expected vec3 axis");
        auto axis=asHashMap(a[0]); double angle=asNumber(a[1]);
        double s=std::sin(angle*0.5);
        return makeQuat(numF(axis,"x")*s, numF(axis,"y")*s, numF(axis,"z")*s, std::cos(angle*0.5));
    }, "fromAxisAngle")));
    mathNs->set("quat", hashMapValue(quatNs));

    // mat4 namespace
    mathNs->set("mat4", buildMat4Namespace());

    // AABB helper
    auto aabbNs = gcNewHashMap();
    aabbNs->set("create", callableValue(std::make_shared<NativeFunction>(4, [](const std::vector<Value>& a) -> Value {
        auto m = gcNewHashMap();
        m->set("x", a[0]); m->set("y", a[1]); m->set("w", a[2]); m->set("h", a[3]);
        m->set("_type", mkStr("aabb"));
        std::weak_ptr<ClawHashMap> wm = m;
        m->set("intersects", callableValue(std::make_shared<NativeFunction>(1, [wm](const std::vector<Value>& b) -> Value {
            auto m2 = wm.lock(); if (!m2) return nilValue();
            if (!isHashMap(b[0])) return boolValue(false);
            auto o = asHashMap(b[0]);
            double ax=numF(m2,"x"), ay=numF(m2,"y"), aw=numF(m2,"w"), ah=numF(m2,"h");
            double bx=numF(o,"x"), by=numF(o,"y"), bw=numF(o,"w"), bh=numF(o,"h");
            return boolValue(ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by);
        }, "intersects")));
        m->set("contains", callableValue(std::make_shared<NativeFunction>(2, [wm](const std::vector<Value>& b) -> Value {
            auto m2 = wm.lock(); if (!m2) return nilValue();
            double px=asNumber(b[0]), py=asNumber(b[1]);
            double ax=numF(m2,"x"), ay=numF(m2,"y"), aw=numF(m2,"w"), ah=numF(m2,"h");
            return boolValue(px>=ax && px<=ax+aw && py>=ay && py<=ay+ah);
        }, "contains")));
        m->set("toString", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
            auto m2 = wm.lock(); if (!m2) return nilValue();
            return mkStr("AABB("+fmtN(numF(m2,"x"))+","+fmtN(numF(m2,"y"))+","+fmtN(numF(m2,"w"))+"x"+fmtN(numF(m2,"h"))+")");
        }, "toString")));
        return hashMapValue(m);
    }, "new")));
    mathNs->set("AABB", hashMapValue(aabbNs));

    // Color helper
    auto colorNs = gcNewHashMap();
    colorNs->set("create", callableValue(std::make_shared<NativeFunction>(-1, [](const std::vector<Value>& a) -> Value {
        double r=0, g=0, b=0, al=1;
        if (a.size()>=3) { r=asNumber(a[0]); g=asNumber(a[1]); b=asNumber(a[2]); }
        if (a.size()>=4) al=asNumber(a[3]);
        auto m = gcNewHashMap();
        m->set("r", numberToValue(r)); m->set("g", numberToValue(g));
        m->set("b", numberToValue(b)); m->set("a", numberToValue(al));
        m->set("_type", mkStr("color"));
        std::weak_ptr<ClawHashMap> wm = m;
        m->set("lerp", callableValue(std::make_shared<NativeFunction>(2, [wm](const std::vector<Value>& args) -> Value {
            auto m2 = wm.lock(); if (!m2) return nilValue();
            if (!isHashMap(args[0])) throw std::runtime_error("color.lerp: expected color");
            auto o = asHashMap(args[0]); double t=asNumber(args[1]);
            double nr=numF(m2,"r")+(numF(o,"r")-numF(m2,"r"))*t;
            double ng=numF(m2,"g")+(numF(o,"g")-numF(m2,"g"))*t;
            double nb=numF(m2,"b")+(numF(o,"b")-numF(m2,"b"))*t;
            double na=numF(m2,"a")+(numF(o,"a")-numF(m2,"a"))*t;
            auto cm = gcNewHashMap();
            cm->set("r",numberToValue(nr)); cm->set("g",numberToValue(ng));
            cm->set("b",numberToValue(nb)); cm->set("a",numberToValue(na));
            cm->set("_type", mkStr("color"));
            return hashMapValue(cm);
        }, "lerp")));
        m->set("toHex", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
            auto m2 = wm.lock(); if (!m2) return nilValue();
            char buf[10];
            int ri=static_cast<int>(std::clamp(numF(m2,"r"),0.0,1.0)*255);
            int gi=static_cast<int>(std::clamp(numF(m2,"g"),0.0,1.0)*255);
            int bi=static_cast<int>(std::clamp(numF(m2,"b"),0.0,1.0)*255);
            std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", ri, gi, bi);
            return mkStr(std::string(buf));
        }, "toHex")));
        m->set("toString", callableValue(std::make_shared<NativeFunction>(0, [wm](const std::vector<Value>&) -> Value {
            auto m2 = wm.lock(); if (!m2) return nilValue();
            return mkStr("color("+fmtN(numF(m2,"r"))+","+fmtN(numF(m2,"g"))+","+fmtN(numF(m2,"b"))+","+fmtN(numF(m2,"a"))+")");
        }, "toString")));
        return hashMapValue(m);
    }, "new")));
    colorNs->set("fromHex", callableValue(std::make_shared<NativeFunction>(1, [](const std::vector<Value>& a) -> Value {
        std::string hex = asString(a[0]);
        if (!hex.empty() && hex[0]=='#') hex=hex.substr(1);
        if (hex.size()==6) hex+="FF";
        if (hex.size()!=8) throw std::runtime_error("color.fromHex: invalid hex string");
        auto parse2=[&](int pos){ return std::stoi(hex.substr(pos,2),nullptr,16)/255.0; };
        auto m = gcNewHashMap();
        m->set("r",numberToValue(parse2(0))); m->set("g",numberToValue(parse2(2)));
        m->set("b",numberToValue(parse2(4))); m->set("a",numberToValue(parse2(6)));
        m->set("_type", mkStr("color"));
        return hashMapValue(m);
    }, "fromHex")));
    // Predefined colors
    auto makeColor=[](double r,double g,double b,double a=1.0){
        auto m=gcNewHashMap();
        m->set("r",numberToValue(r)); m->set("g",numberToValue(g));
        m->set("b",numberToValue(b)); m->set("a",numberToValue(a));
        m->set("_type",mkStr("color")); return hashMapValue(m);
    };
    colorNs->set("RED",     makeColor(1,0,0));
    colorNs->set("GREEN",   makeColor(0,1,0));
    colorNs->set("BLUE",    makeColor(0,0,1));
    colorNs->set("WHITE",   makeColor(1,1,1));
    colorNs->set("BLACK",   makeColor(0,0,0));
    colorNs->set("YELLOW",  makeColor(1,1,0));
    colorNs->set("CYAN",    makeColor(0,1,1));
    colorNs->set("MAGENTA", makeColor(1,0,1));
    colorNs->set("TRANSPARENT", makeColor(0,0,0,0));
    mathNs->set("Color", hashMapValue(colorNs));

    // Register the math namespace as global 'math'
    globals->define("math", hashMapValue(mathNs));
}

} // namespace claw
