#include "native_vr.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

namespace claw {

// ============================================================
// VR/AR Implementation
// ============================================================
struct VREngine {
    bool isInitialized = false;
    bool isConnected = false;
    std::string deviceName = "Unknown VR Device";
    
    // VR Headset tracking
    struct VRHeadset {
        double position[3] = {0.0, 0.0, 0.0};
        double rotation[4] = {0.0, 0.0, 0.0, 1.0}; // Quaternion
        double fov = 90.0;
        double ipd = 0.064; // Interpupillary distance
    } headset;
    
    // VR Controllers
    struct VRController {
        bool isConnected = false;
        double position[3] = {0.0, 0.0, 0.0};
        double rotation[4] = {0.0, 0.0, 0.0, 1.0};
        bool buttons[10] = {false};
        double trigger[2] = {0.0, 0.0};
        double joystick[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    } controllers[2];
    
    bool initialize() {
        isInitialized = true;
        isConnected = true;
        deviceName = "ClawScript VR Headset";
        
        // Initialize controllers
        controllers[0].isConnected = true;
        controllers[1].isConnected = true;
        
        return true;
    }
    
    void updateTracking() {
        if (!isInitialized) return;
        
        // Simulate head movement (sine wave for demo)
        static double time = 0.0;
        time += 0.016; // ~60 FPS
        
        headset.position[0] = sin(time) * 0.1;
        headset.position[1] = cos(time * 0.7) * 0.05;
        headset.position[2] = 1.0;
        
        // Simulate controller movement
        controllers[0].position[0] = sin(time * 1.5) * 0.2;
        controllers[0].position[1] = cos(time * 1.2) * 0.15;
        controllers[0].position[2] = 0.3;
        
        controllers[1].position[0] = -sin(time * 1.5) * 0.2;
        controllers[1].position[1] = cos(time * 1.2) * 0.15;
        controllers[1].position[2] = 0.3;
    }
    
    void renderFrame(const std::string& scene) {
        // Mock VR rendering
        // In real implementation, would render to both eyes with proper distortion
    }
    
    void hapticFeedback(int controller, float intensity, float duration) {
        // Mock haptic feedback
        // In real implementation, would trigger controller vibration
    }
    
    void shutdown() {
        isInitialized = false;
        isConnected = false;
        controllers[0].isConnected = false;
        controllers[1].isConnected = false;
    }
};

// ============================================================
// Register VR natives
// ============================================================
void registerNativeVR(const std::shared_ptr<Environment>& globals) {

    static VREngine vrEngine;
    
    // VR object with methods
    auto vr = std::make_shared<ClawHashMap>();
    
    // Initialize VR
    vr->set("init", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            bool success = vrEngine.initialize();
            return boolValue(success);
        },
        "vr.init"
    )));
    
    // Update tracking
    vr->set("update", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            vrEngine.updateTracking();
            return boolValue(true);
        },
        "vr.update"
    )));
    
    // Get headset position
    vr->set("getHeadsetPosition", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            auto posArray = std::make_shared<ClawArray>();
            posArray->push(numberToValue(vrEngine.headset.position[0]));
            posArray->push(numberToValue(vrEngine.headset.position[1]));
            posArray->push(numberToValue(vrEngine.headset.position[2]));
            return arrayValue(posArray);
        },
        "vr.getHeadsetPosition"
    )));
    
    // Get controller state
    vr->set("getController", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            int controllerIndex = static_cast<int>(asNumber(args[0]));
            if (controllerIndex < 0 || controllerIndex > 1) {
                return nilValue();
            }
            
            const auto& controller = vrEngine.controllers[controllerIndex];
            auto controllerMap = std::make_shared<ClawHashMap>();
            
            // Position
            auto posArray = std::make_shared<ClawArray>();
            posArray->push(numberToValue(controller.position[0]));
            posArray->push(numberToValue(controller.position[1]));
            posArray->push(numberToValue(controller.position[2]));
            controllerMap->set("position", arrayValue(posArray));
            
            // Trigger
            controllerMap->set("trigger", numberToValue(controller.trigger[0]));
            
            // Connected
            controllerMap->set("connected", boolValue(controller.isConnected));
            
            return hashMapValue(controllerMap);
        },
        "vr.getController"
    )));
    
    // Haptic feedback
    vr->set("haptic", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            int controller = static_cast<int>(asNumber(args[0]));
            float intensity = static_cast<float>(asNumber(args[1]));
            float duration = static_cast<float>(asNumber(args[2]));
            
            vrEngine.hapticFeedback(controller, intensity, duration);
            return boolValue(true);
        },
        "vr.haptic"
    )));
    
    // Shutdown VR
    vr->set("shutdown", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            vrEngine.shutdown();
            return boolValue(true);
        },
        "vr.shutdown"
    )));
    
    globals->define("vr", hashMapValue(vr));
    
    // Convenience functions
    globals->define("initVR", callableValue(std::make_shared<NativeFunction>(
        0,
        [vr](const std::vector<Value>& args) -> Value {
            return vr->get("init")->call(*nullptr, args);
        },
        "initVR"
    )));
    
    globals->define("updateVR", callableValue(std::make_shared<NativeFunction>(
        0,
        [vr](const std::vector<Value>& args) -> Value {
            return vr->get("update")->call(*nullptr, args);
        },
        "updateVR"
    )));
}

} // namespace claw
