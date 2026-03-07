#include "native_robotics.h"
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
// Robotics Implementation
// ============================================================
struct Robot {
    std::string name;
    double position[3] = {0.0, 0.0, 0.0};
    double rotation[3] = {0.0, 0.0, 0.0}; // Roll, Pitch, Yaw
    double velocity[3] = {0.0, 0.0, 0.0};
    bool isActive = false;
    
    struct Joint {
        std::string name;
        double angle = 0.0;
        double minAngle = -180.0;
        double maxAngle = 180.0;
        double maxVelocity = 90.0; // degrees per second
    };
    
    std::vector<Joint> joints;
    
    struct Sensor {
        std::string type;
        double value = 0.0;
        bool isActive = false;
        std::unordered_map<std::string, double> data;
    };
    
    std::vector<Sensor> sensors;
    
    Robot(const std::string& robotName) : name(robotName) {
        // Initialize with default joints for a humanoid robot
        joints = {
            {"head_yaw", 0.0, -90.0, 90.0, 45.0},
            {"head_pitch", 0.0, -45.0, 45.0, 45.0},
            {"left_shoulder_pitch", 0.0, -180.0, 180.0, 90.0},
            {"left_shoulder_roll", 0.0, -90.0, 90.0, 90.0},
            {"left_elbow", 0.0, -150.0, 150.0, 120.0},
            {"right_shoulder_pitch", 0.0, -180.0, 180.0, 90.0},
            {"right_shoulder_roll", 0.0, -90.0, 90.0, 90.0},
            {"right_elbow", 0.0, -150.0, 150.0, 120.0},
            {"left_hip_pitch", 0.0, -90.0, 90.0, 60.0},
            {"left_hip_roll", 0.0, -45.0, 45.0, 30.0},
            {"left_knee", 0.0, -150.0, 0.0, 90.0},
            {"right_hip_pitch", 0.0, -90.0, 90.0, 60.0},
            {"right_hip_roll", 0.0, -45.0, 45.0, 30.0},
            {"right_knee", 0.0, -150.0, 0.0, 90.0}
        };
        
        // Initialize sensors
        sensors = {
            {"camera", 0.0, true, {{"resolution_x", 1920}, {"resolution_y", 1080}, {"fps", 30}}},
            {"lidar", 0.0, true, {{"range", 10.0}, {"resolution", 0.1}}},
            {"accelerometer", 0.0, true, {{"range", 16.0}, {"resolution", 0.001}}},
            {"gyroscope", 0.0, true, {{"range", 2000.0}, {"resolution", 0.1}}},
            {"microphone", 0.0, true, {{"sample_rate", 44100}, {"channels", 2}}}
        };
    }
    
    bool activate() {
        isActive = true;
        return true;
    }
    
    bool deactivate() {
        isActive = false;
        return true;
    }
    
    bool moveTo(double x, double y, double z) {
        if (!isActive) return false;
        
        position[0] = x;
        position[1] = y;
        position[2] = z;
        
        return true;
    }
    
    bool rotate(double roll, double pitch, double yaw) {
        if (!isActive) return false;
        
        rotation[0] = roll;
        rotation[1] = pitch;
        rotation[2] = yaw;
        
        return true;
    }
    
    bool setJointAngle(const std::string& jointName, double angle) {
        if (!isActive) return false;
        
        for (auto& joint : joints) {
            if (joint.name == jointName) {
                if (angle >= joint.minAngle && angle <= joint.maxAngle) {
                    joint.angle = angle;
                    return true;
                }
            }
        }
        
        return false;
    }
    
    double getJointAngle(const std::string& jointName) {
        for (const auto& joint : joints) {
            if (joint.name == jointName) {
                return joint.angle;
            }
        }
        return 0.0;
    }
    
    bool setVelocity(double vx, double vy, double vz) {
        if (!isActive) return false;
        
        velocity[0] = vx;
        velocity[1] = vy;
        velocity[2] = vz;
        
        return true;
    }
    
    std::unordered_map<std::string, double> getSensorData(const std::string& sensorType) {
        for (const auto& sensor : sensors) {
            if (sensor.type == sensorType && sensor.isActive) {
                // Simulate sensor data
                auto data = sensor.data;
                
                if (sensorType == "camera") {
                    data["objects_detected"] = 3.0;
                    data["faces_detected"] = 1.0;
                } else if (sensorType == "lidar") {
                    data["obstacles"] = 2.0;
                    data["nearest_distance"] = 2.5;
                } else if (sensorType == "accelerometer") {
                    data["x"] = sin(0.1) * 9.8;
                    data["y"] = cos(0.1) * 9.8;
                    data["z"] = 9.8;
                }
                
                return data;
            }
        }
        
        return {};
    }
    
    void update(double deltaTime) {
        if (!isActive) return;
        
        // Update position based on velocity
        position[0] += velocity[0] * deltaTime;
        position[1] += velocity[1] * deltaTime;
        position[2] += velocity[2] * deltaTime;
    }
};

// ============================================================
// Register robotics natives
// ============================================================
void registerNativeRobotics(const std::shared_ptr<Environment>& globals) {

    // Robotics object with methods
    auto robotics = std::make_shared<ClawHashMap>();
    
    // Create robot
    robotics->set("createRobot", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            
            auto robot = std::make_shared<Robot>(name);
            auto robotMap = std::make_shared<ClawHashMap>();
            robotMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(robot.get())));
            robotMap->set("name", stringValue(StringPool::intern(name).data()));
            robotMap->set("active", boolValue(false));
            
            return hashMapValue(robotMap);
        },
        "robotics.createRobot"
    )));
    
    // Activate robot
    robotics->set("activate", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto robotMap = asHashMap(args[0]);
            
            auto ptrValue = robotMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto robot = static_cast<Robot*>(ptr);
            
            bool success = robot->activate();
            robotMap->set("active", boolValue(success));
            
            return boolValue(success);
        },
        "robotics.activate"
    )));
    
    // Move robot
    robotics->set("moveTo", callableValue(std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            auto robotMap = asHashMap(args[0]);
            double x = asNumber(args[1]);
            double y = asNumber(args[2]);
            double z = asNumber(args[3]);
            
            auto ptrValue = robotMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto robot = static_cast<Robot*>(ptr);
            
            bool success = robot->moveTo(x, y, z);
            return boolValue(success);
        },
        "robotics.moveTo"
    )));
    
    // Set joint angle
    robotics->set("setJoint", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            auto robotMap = asHashMap(args[0]);
            std::string jointName = valueToString(args[1]);
            double angle = asNumber(args[2]);
            
            auto ptrValue = robotMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto robot = static_cast<Robot*>(ptr);
            
            bool success = robot->setJointAngle(jointName, angle);
            return boolValue(success);
        },
        "robotics.setJoint"
    )));
    
    // Get joint angle
    robotics->set("getJoint", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto robotMap = asHashMap(args[0]);
            std::string jointName = valueToString(args[1]);
            
            auto ptrValue = robotMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto robot = static_cast<Robot*>(ptr);
            
            double angle = robot->getJointAngle(jointName);
            return numberToValue(angle);
        },
        "robotics.getJoint"
    )));
    
    // Get sensor data
    robotics->set("getSensorData", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto robotMap = asHashMap(args[0]);
            std::string sensorType = valueToString(args[1]);
            
            auto ptrValue = robotMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto robot = static_cast<Robot*>(ptr);
            
            auto sensorData = robot->getSensorData(sensorType);
            auto dataMap = std::make_shared<ClawHashMap>();
            
            for (const auto& [key, value] : sensorData) {
                dataMap->set(key, numberToValue(value));
            }
            
            return hashMapValue(dataMap);
        },
        "robotics.getSensorData"
    )));
    
    globals->define("robotics", hashMapValue(robotics));
    
    // Convenience functions
    globals->define("createRobot", callableValue(std::make_shared<NativeFunction>(
        1,
        [robotics](const std::vector<Value>& args) -> Value {
            return robotics->get("createRobot")->call(*nullptr, args);
        },
        "createRobot"
    )));
}

} // namespace claw
