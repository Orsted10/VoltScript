#include "native_gfx.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace claw {

// ============================================================
// Graphics Window Handle
// ============================================================
struct WindowHandle {
    int width = 800;
    int height = 600;
    std::string title = "ClawScript Window";
    bool isOpen = true;
    
    void close() {
        isOpen = false;
    }
};

// ============================================================
// Color structure
// ============================================================
struct Color {
    uint8_t r = 255, g = 255, b = 255, a = 255;
    
    static Color fromValue(Value val) {
        Color c;
        if (isHashMap(val)) {
            auto map = asHashMap(val);
            if (map->contains("r")) c.r = static_cast<uint8_t>(asNumber(map->get("r")));
            if (map->contains("g")) c.g = static_cast<uint8_t>(asNumber(map->get("g")));
            if (map->contains("b")) c.b = static_cast<uint8_t>(asNumber(map->get("b")));
            if (map->contains("a")) c.a = static_cast<uint8_t>(asNumber(map->get("a")));
        }
        return c;
    }
};

// ============================================================
// Register graphics natives
// ============================================================
void registerNativeGFX(const std::shared_ptr<Environment>& globals) {

    // Graphics object with methods
    auto gfx = std::make_shared<ClawHashMap>();
    
    // Create window
    gfx->set("createWindow", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            std::string title = valueToString(args[0]);
            int width = static_cast<int>(asNumber(args[1]));
            int height = static_cast<int>(asNumber(args[2]));
            
            auto window = std::make_shared<WindowHandle>();
            window->title = title;
            window->width = width;
            window->height = height;
            
            auto windowMap = std::make_shared<ClawHashMap>();
            windowMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(window.get())));
            windowMap->set("title", stringValue(StringPool::intern(title).data()));
            windowMap->set("width", numberToValue(width));
            windowMap->set("height", numberToValue(height));
            
            // Add window methods
            windowMap->set("close", callableValue(std::make_shared<NativeFunction>(
                0,
                [window](const std::vector<Value>& args) -> Value {
                    window->close();
                    return boolValue(true);
                },
                "window.close"
            )));
            
            windowMap->set("isOpen", callableValue(std::make_shared<NativeFunction>(
                0,
                [window](const std::vector<Value>& args) -> Value {
                    return boolValue(window->isOpen);
                },
                "window.isOpen"
            )));
            
            return hashMapValue(windowMap);
        },
        "gfx.createWindow"
    )));
    
    // Color creation
    gfx->set("color", callableValue(std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            Color c;
            c.r = static_cast<uint8_t>(asNumber(args[0]));
            c.g = static_cast<uint8_t>(asNumber(args[1]));
            c.b = static_cast<uint8_t>(asNumber(args[2]));
            c.a = args.size() > 3 ? static_cast<uint8_t>(asNumber(args[3])) : 255;
            
            auto colorMap = std::make_shared<ClawHashMap>();
            colorMap->set("r", numberToValue(c.r));
            colorMap->set("g", numberToValue(c.g));
            colorMap->set("b", numberToValue(c.b));
            colorMap->set("a", numberToValue(c.a));
            
            return hashMapValue(colorMap);
        },
        "gfx.color"
    )));
    
    // Predefined colors
    auto colors = std::make_shared<ClawHashMap>();
    auto white = std::make_shared<ClawHashMap>();
    white->set("r", numberToValue(255));
    white->set("g", numberToValue(255));
    white->set("b", numberToValue(255));
    white->set("a", numberToValue(255));
    colors->set("WHITE", hashMapValue(white));
    
    auto black = std::make_shared<ClawHashMap>();
    black->set("r", numberToValue(0));
    black->set("g", numberToValue(0));
    black->set("b", numberToValue(0));
    black->set("a", numberToValue(255));
    colors->set("BLACK", hashMapValue(black));
    
    auto red = std::make_shared<ClawHashMap>();
    red->set("r", numberToValue(255));
    red->set("g", numberToValue(0));
    red->set("b", numberToValue(0));
    red->set("a", numberToValue(255));
    colors->set("RED", hashMapValue(red));
    
    auto green = std::make_shared<ClawHashMap>();
    green->set("r", numberToValue(0));
    green->set("g", numberToValue(255));
    green->set("b", numberToValue(0));
    green->set("a", numberToValue(255));
    colors->set("GREEN", hashMapValue(green));
    
    auto blue = std::make_shared<ClawHashMap>();
    blue->set("r", numberToValue(0));
    blue->set("g", numberToValue(0));
    blue->set("b", numberToValue(255));
    blue->set("a", numberToValue(255));
    colors->set("BLUE", hashMapValue(blue));
    
    gfx->set("colors", hashMapValue(colors));
    
    globals->define("gfx", hashMapValue(gfx));
    
    // Convenience functions
    globals->define("createWindow", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            std::string title = valueToString(args[0]);
            int width = static_cast<int>(asNumber(args[1]));
            int height = static_cast<int>(asNumber(args[2]));
            
            auto window = std::make_shared<WindowHandle>();
            window->title = title;
            window->width = width;
            window->height = height;
            
            auto windowMap = std::make_shared<ClawHashMap>();
            windowMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(window.get())));
            windowMap->set("title", stringValue(StringPool::intern(title).data()));
            windowMap->set("width", numberToValue(width));
            windowMap->set("height", numberToValue(height));
            
            return hashMapValue(windowMap);
        },
        "createWindow"
    )));
    
    globals->define("color", callableValue(std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            Color c;
            c.r = static_cast<uint8_t>(asNumber(args[0]));
            c.g = static_cast<uint8_t>(asNumber(args[1]));
            c.b = static_cast<uint8_t>(asNumber(args[2]));
            c.a = args.size() > 3 ? static_cast<uint8_t>(asNumber(args[3])) : 255;
            
            auto colorMap = std::make_shared<ClawHashMap>();
            colorMap->set("r", numberToValue(c.r));
            colorMap->set("g", numberToValue(c.g));
            colorMap->set("b", numberToValue(c.b));
            colorMap->set("a", numberToValue(c.a));
            
            return hashMapValue(colorMap);
        },
        "color"
    )));
}

} // namespace claw
