#include "native_space.h"
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
// Space/Astronomy Implementation
// ============================================================
struct SpaceSystem {
    struct CelestialBody {
        std::string name;
        std::string type; // "planet", "star", "moon", "asteroid"
        double mass; // kg
        double radius; // km
        double position[3]; // AU from sun
        double velocity[3]; // km/s
        double temperature; // Kelvin
        std::vector<std::string> satellites;
        
        CelestialBody(const std::string& n, const std::string& t, double m, double r) 
            : name(n), type(t), mass(m), radius(r) {
            position[0] = 0.0; position[1] = 0.0; position[2] = 0.0;
            velocity[0] = 0.0; velocity[1] = 0.0; velocity[2] = 0.0;
            temperature = 273.15; // Default to 0°C
        }
    };
    
    struct Spacecraft {
        std::string name;
        double position[3]; // km
        double velocity[3]; // km/s
        double fuel; // kg
        bool isActive = false;
        std::string mission;
        
        Spacecraft(const std::string& n) : name(n), fuel(1000.0) {
            position[0] = 0.0; position[1] = 0.0; position[2] = 0.0;
            velocity[0] = 0.0; velocity[1] = 0.0; velocity[2] = 0.0;
        }
    };
    
    std::vector<CelestialBody> celestialBodies;
    std::vector<Spacecraft> spacecraft;
    
    SpaceSystem() {
        // Initialize solar system
        celestialBodies.emplace_back("Sun", "star", 1.989e30, 696340);
        celestialBodies.back().temperature = 5778;
        celestialBodies.back().position[0] = 0;
        
        celestialBodies.emplace_back("Mercury", "planet", 3.301e23, 2439.7);
        celestialBodies.back().position[0] = 0.39; // AU from sun
        celestialBodies.back().temperature = 440;
        
        celestialBodies.emplace_back("Venus", "planet", 4.867e24, 6051.8);
        celestialBodies.back().position[0] = 0.72;
        celestialBodies.back().temperature = 737;
        
        celestialBodies.emplace_back("Earth", "planet", 5.972e24, 6371);
        celestialBodies.back().position[0] = 1.0;
        celestialBodies.back().temperature = 288;
        celestialBodies.back().satellites.push_back("Moon");
        
        celestialBodies.emplace_back("Moon", "moon", 7.342e22, 1737.4);
        celestialBodies.back().position[0] = 1.00257; // Earth + Moon distance
        
        celestialBodies.emplace_back("Mars", "planet", 6.417e23, 3389.5);
        celestialBodies.back().position[0] = 1.52;
        celestialBodies.back().temperature = 210;
        
        celestialBodies.emplace_back("Jupiter", "planet", 1.898e27, 69911);
        celestialBodies.back().position[0] = 5.20;
        celestialBodies.back().temperature = 165;
        
        celestialBodies.emplace_back("Saturn", "planet", 5.683e26, 58232);
        celestialBodies.back().position[0] = 9.58;
        celestialBodies.back().temperature = 134;
    }
    
    Spacecraft* createSpacecraft(const std::string& name, const std::string& mission) {
        spacecraft.emplace_back(name);
        spacecraft.back().mission = mission;
        return &spacecraft.back();
    }
    
    bool launchSpacecraft(const std::string& name) {
        for (auto& craft : spacecraft) {
            if (craft.name == name) {
                craft.isActive = true;
                // Launch to low Earth orbit (400 km altitude)
                craft.position[0] = 0;
                craft.position[1] = 0;
                craft.position[2] = 6371 + 400; // Earth radius + altitude
                craft.velocity[0] = 7.8; // orbital velocity km/s
                craft.fuel -= 100; // launch fuel cost
                return true;
            }
        }
        return false;
    }
    
    bool calculateTrajectory(const std::string& spacecraftName, const std::string& targetBody) {
        // Simplified trajectory calculation
        for (auto& craft : spacecraft) {
            if (craft.name == spacecraftName) {
                for (const auto& body : celestialBodies) {
                    if (body.name == targetBody) {
                        // Calculate Hohmann transfer orbit
                        double r1 = 1.0; // Earth orbit (AU)
                        double r2 = body.position[0]; // Target orbit (AU)
                        
                        // Semi-major axis of transfer orbit
                        double a = (r1 + r2) / 2.0;
                        
                        // Transfer time (simplified)
                        double transferTime = 3.14159 * sqrt(a * a * a / 1.327e20); // GM of sun
                        
                        // Set spacecraft on trajectory (simplified)
                        craft.position[0] = body.position[0];
                        craft.fuel -= 200; // transfer burn
                        return true;
                    }
                }
            }
        }
        return false;
    }
    
    double calculateDistance(const std::string& body1, const std::string& body2) {
        const CelestialBody* b1 = nullptr;
        const CelestialBody* b2 = nullptr;
        
        for (const auto& body : celestialBodies) {
            if (body.name == body1) b1 = &body;
            if (body.name == body2) b2 = &body;
        }
        
        if (b1 && b2) {
            double dx = b1->position[0] - b2->position[0];
            double dy = b1->position[1] - b2->position[1];
            double dz = b1->position[2] - b2->position[2];
            return sqrt(dx*dx + dy*dy + dz*dz);
        }
        
        return -1.0;
    }
    
    std::vector<std::string> getVisibleBodies(double observerX, double observerY, double observerZ) {
        std::vector<std::string> visible;
        
        for (const auto& body : celestialBodies) {
            double distance = sqrt(
                pow(body.position[0] - observerX, 2) +
                pow(body.position[1] - observerY, 2) +
                pow(body.position[2] - observerZ, 2)
            );
            
            // Simple visibility check (within 10 AU)
            if (distance < 10.0) {
                visible.push_back(body.name);
            }
        }
        
        return visible;
    }
    
    void simulateTime(double days) {
        // Simulate orbital motion (simplified)
        for (auto& body : celestialBodies) {
            if (body.type == "planet") {
                // Simple circular orbit
                double radius = sqrt(body.position[0]*body.position[0] + body.position[1]*body.position[1]);
                double angularVelocity = sqrt(1.327e20 / (radius * radius * radius * 1.496e11)); // rad/s
                
                double angle = angularVelocity * days * 86400; // Convert days to seconds
                double newAngle = atan2(body.position[1], body.position[0]) + angle;
                
                body.position[0] = radius * cos(newAngle);
                body.position[1] = radius * sin(newAngle);
            }
        }
    }
};

// ============================================================
// Register space natives
// ============================================================
void registerNativeSpace(const std::shared_ptr<Environment>& globals) {

    static SpaceSystem spaceSystem;
    
    // Space object with methods
    auto space = std::make_shared<ClawHashMap>();
    
    // Get solar system info
    space->set("getSolarSystem", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            auto systemArray = std::make_shared<ClawArray>();
            
            for (const auto& body : spaceSystem.celestialBodies) {
                auto bodyMap = std::make_shared<ClawHashMap>();
                bodyMap->set("name", stringValue(StringPool::intern(body.name).data()));
                bodyMap->set("type", stringValue(StringPool::intern(body.type).data()));
                bodyMap->set("mass", numberToValue(body.mass));
                bodyMap->set("radius", numberToValue(body.radius));
                bodyMap->set("temperature", numberToValue(body.temperature));
                
                auto posArray = std::make_shared<ClawArray>();
                posArray->push(numberToValue(body.position[0]));
                posArray->push(numberToValue(body.position[1]));
                posArray->push(numberToValue(body.position[2]));
                bodyMap->set("position", arrayValue(posArray));
                
                systemArray->push(hashMapValue(bodyMap));
            }
            
            return arrayValue(systemArray);
        },
        "space.getSolarSystem"
    )));
    
    // Create spacecraft
    space->set("createSpacecraft", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            std::string mission = valueToString(args[1]);
            
            auto craft = spaceSystem.createSpacecraft(name, mission);
            auto craftMap = std::make_shared<ClawHashMap>();
            craftMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(craft)));
            craftMap->set("name", stringValue(StringPool::intern(name).data()));
            craftMap->set("mission", stringValue(StringPool::intern(mission).data()));
            craftMap->set("fuel", numberToValue(craft->fuel));
            craftMap->set("active", boolValue(craft->isActive));
            
            return hashMapValue(craftMap);
        },
        "space.createSpacecraft"
    )));
    
    // Launch spacecraft
    space->set("launch", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            bool success = spaceSystem.launchSpacecraft(name);
            return boolValue(success);
        },
        "space.launch"
    )));
    
    // Calculate trajectory
    space->set("calculateTrajectory", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string spacecraft = valueToString(args[0]);
            std::string target = valueToString(args[1]);
            bool success = spaceSystem.calculateTrajectory(spacecraft, target);
            return boolValue(success);
        },
        "space.calculateTrajectory"
    )));
    
    // Calculate distance
    space->set("getDistance", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string body1 = valueToString(args[0]);
            std::string body2 = valueToString(args[1]);
            double distance = spaceSystem.calculateDistance(body1, body2);
            return numberToValue(distance);
        },
        "space.getDistance"
    )));
    
    // Simulate time
    space->set("simulate", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            double days = asNumber(args[0]);
            spaceSystem.simulateTime(days);
            return boolValue(true);
        },
        "space.simulate"
    )));
    
    globals->define("space", hashMapValue(space));
    
    // Convenience functions
    globals->define("createSpacecraft", callableValue(std::make_shared<NativeFunction>(
        2,
        [space](const std::vector<Value>& args) -> Value {
            return space->get("createSpacecraft")->call(*nullptr, args);
        },
        "createSpacecraft"
    )));
}

} // namespace claw
