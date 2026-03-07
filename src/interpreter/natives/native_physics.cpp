#include "native_physics.h"
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
// Advanced Physics Engine Implementation
// ============================================================
struct PhysicsEngine {
    struct Particle {
        double position[3] = {0.0, 0.0, 0.0};
        double velocity[3] = {0.0, 0.0, 0.0};
        double acceleration[3] = {0.0, 0.0, 0.0};
        double mass = 1.0;
        double charge = 0.0;
        double radius = 1.0;
        bool isActive = true;
        
        void applyForce(double fx, double fy, double fz) {
            acceleration[0] += fx / mass;
            acceleration[1] += fy / mass;
            acceleration[2] += fz / mass;
        }
        
        void update(double dt) {
            velocity[0] += acceleration[0] * dt;
            velocity[1] += acceleration[1] * dt;
            velocity[2] += acceleration[2] * dt;
            
            position[0] += velocity[0] * dt;
            position[1] += velocity[1] * dt;
            position[2] += velocity[2] * dt;
            
            // Reset acceleration for next frame
            acceleration[0] = 0.0;
            acceleration[1] = 0.0;
            acceleration[2] = 0.0;
        }
        
        double kineticEnergy() const {
            double v2 = velocity[0]*velocity[0] + velocity[1]*velocity[1] + velocity[2]*velocity[2];
            return 0.5 * mass * v2;
        }
        
        double momentum() const {
            double v = sqrt(velocity[0]*velocity[0] + velocity[1]*velocity[1] + velocity[2]*velocity[2]);
            return mass * v;
        }
    };
    
    struct RigidBody {
        double position[3] = {0.0, 0.0, 0.0};
        double rotation[4] = {0.0, 0.0, 0.0, 1.0}; // Quaternion
        double linearVelocity[3] = {0.0, 0.0, 0.0};
        double angularVelocity[3] = {0.0, 0.0, 0.0};
        double mass = 1.0;
        double inertia[3][3] = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};
        std::string shape; // "box", "sphere", "cylinder"
        double dimensions[3] = {1.0, 1.0, 1.0};
        
        void applyTorque(double tx, double ty, double tz) {
            // Simplified torque application
            angularVelocity[0] += tx / inertia[0][0];
            angularVelocity[1] += ty / inertia[1][1];
            angularVelocity[2] += tz / inertia[2][2];
        }
        
        void update(double dt) {
            position[0] += linearVelocity[0] * dt;
            position[1] += linearVelocity[1] * dt;
            position[2] += linearVelocity[2] * dt;
            
            // Update rotation (simplified)
            rotation[0] += angularVelocity[0] * dt * 0.5;
            rotation[1] += angularVelocity[1] * dt * 0.5;
            rotation[2] += angularVelocity[2] * dt * 0.5;
        }
    };
    
    struct ForceField {
        std::string type; // "gravity", "electromagnetic", "nuclear"
        double strength = 1.0;
        double center[3] = {0.0, 0.0, 0.0};
        double range = 1000.0;
        
        std::vector<double> calculateForce(double pos[3], double mass, double charge) const {
            std::vector<double> force(3, 0.0);
            
            if (type == "gravity") {
                double dx = center[0] - pos[0];
                double dy = center[1] - pos[1];
                double dz = center[2] - pos[2];
                double r2 = dx*dx + dy*dy + dz*dz;
                
                if (r2 > 0.01) { // Avoid division by zero
                    double r = sqrt(r2);
                    double f = strength * mass / r2;
                    force[0] = f * dx / r;
                    force[1] = f * dy / r;
                    force[2] = f * dz / r;
                }
            } else if (type == "electromagnetic") {
                double dx = center[0] - pos[0];
                double dy = center[1] - pos[1];
                double dz = center[2] - pos[2];
                double r2 = dx*dx + dy*dy + dz*dz;
                
                if (r2 > 0.01 && charge != 0) {
                    double r = sqrt(r2);
                    double f = strength * charge / r2;
                    force[0] = f * dx / r;
                    force[1] = f * dy / r;
                    force[2] = f * dz / r;
                }
            }
            
            return force;
        }
    };
    
    std::vector<Particle> particles;
    std::vector<RigidBody> rigidBodies;
    std::vector<ForceField> forceFields;
    double timeStep = 0.016; // 60 FPS
    double gravity = 9.81;
    bool isPaused = false;
    
    PhysicsEngine() {
        // Add default gravity field
        ForceField gravityField;
        gravityField.type = "gravity";
        gravityField.strength = -gravity;
        gravityField.center[1] = -1000.0; // Gravity pointing down
        gravityField.range = 10000.0;
        forceFields.push_back(gravityField);
    }
    
    Particle* createParticle(double x, double y, double z, double mass, double charge) {
        particles.emplace_back();
        Particle& p = particles.back();
        p.position[0] = x;
        p.position[1] = y;
        p.position[2] = z;
        p.mass = mass;
        p.charge = charge;
        return &p;
    }
    
    RigidBody* createRigidBody(const std::string& shape, double x, double y, double z, double mass) {
        rigidBodies.emplace_back();
        RigidBody& rb = rigidBodies.back();
        rb.shape = shape;
        rb.position[0] = x;
        rb.position[1] = y;
        rb.position[2] = z;
        rb.mass = mass;
        return &rb;
    }
    
    ForceField* createForceField(const std::string& type, double strength, double x, double y, double z, double range) {
        forceFields.emplace_back();
        ForceField& ff = forceFields.back();
        ff.type = type;
        ff.strength = strength;
        ff.center[0] = x;
        ff.center[1] = y;
        ff.center[2] = z;
        ff.range = range;
        return &ff;
    }
    
    void simulate(double dt) {
        if (isPaused) return;
        
        // Apply forces to particles
        for (auto& particle : particles) {
            if (!particle.isActive) continue;
            
            for (const auto& field : forceFields) {
                auto force = field.calculateForce(particle.position, particle.mass, particle.charge);
                particle.applyForce(force[0], force[1], force[2]);
            }
        }
        
        // Update particles
        for (auto& particle : particles) {
            if (particle.isActive) {
                particle.update(dt);
            }
        }
        
        // Update rigid bodies
        for (auto& rb : rigidBodies) {
            rb.update(dt);
        }
    }
    
    bool checkCollision(const Particle& p1, const Particle& p2) {
        double dx = p1.position[0] - p2.position[0];
        double dy = p1.position[1] - p2.position[1];
        double dz = p1.position[2] - p2.position[2];
        double distance = sqrt(dx*dx + dy*dy + dz*dz);
        return distance < (p1.radius + p2.radius);
    }
    
    void resolveCollision(Particle& p1, Particle& p2) {
        // Simple elastic collision
        double dx = p2.position[0] - p1.position[0];
        double dy = p2.position[1] - p1.position[1];
        double dz = p2.position[2] - p1.position[2];
        double distance = sqrt(dx*dx + dy*dy + dz*dz);
        
        if (distance == 0) return; // Avoid division by zero
        
        // Normalize collision vector
        dx /= distance;
        dy /= distance;
        dz /= distance;
        
        // Relative velocity
        double dvx = p2.velocity[0] - p1.velocity[0];
        double dvy = p2.velocity[1] - p1.velocity[1];
        double dvz = p2.velocity[2] - p1.velocity[2];
        
        // Relative velocity in collision normal direction
        double speed = dvx * dx + dvy * dy + dvz * dz;
        
        if (speed < 0) return; // Objects moving apart
        
        // Collision response
        double impulse = 2 * speed / (p1.mass + p2.mass);
        
        p1.velocity[0] += impulse * p2.mass * dx;
        p1.velocity[1] += impulse * p2.mass * dy;
        p1.velocity[2] += impulse * p2.mass * dz;
        
        p2.velocity[0] -= impulse * p1.mass * dx;
        p2.velocity[1] -= impulse * p1.mass * dy;
        p2.velocity[2] -= impulse * p1.mass * dz;
    }
    
    double getTotalEnergy() const {
        double totalEnergy = 0.0;
        
        for (const auto& particle : particles) {
            totalEnergy += particle.kineticEnergy();
            // Add potential energy (simplified)
            totalEnergy += particle.mass * gravity * particle.position[1];
        }
        
        return totalEnergy;
    }
    
    void pause() { isPaused = true; }
    void resume() { isPaused = false; }
    void setTimeStep(double dt) { timeStep = dt; }
    void setGravity(double g) { gravity = g; }
};

// ============================================================
// Register physics natives
// ============================================================
void registerNativePhysics(const std::shared_ptr<Environment>& globals) {

    static PhysicsEngine physicsEngine;
    
    // Physics object with methods
    auto physics = std::make_shared<ClawHashMap>();
    
    // Create particle
    physics->set("createParticle", callableValue(std::make_shared<NativeFunction>(
        5,
        [](const std::vector<Value>& args) -> Value {
            double x = asNumber(args[0]);
            double y = asNumber(args[1]);
            double z = asNumber(args[2]);
            double mass = asNumber(args[3]);
            double charge = asNumber(args[4]);
            
            auto particle = physicsEngine.createParticle(x, y, z, mass, charge);
            auto particleMap = std::make_shared<ClawHashMap>();
            particleMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(particle)));
            particleMap->set("mass", numberToValue(mass));
            particleMap->set("charge", numberToValue(charge));
            
            return hashMapValue(particleMap);
        },
        "physics.createParticle"
    )));
    
    // Create rigid body
    physics->set("createRigidBody", callableValue(std::make_shared<NativeFunction>(
        5,
        [](const std::vector<Value>& args) -> Value {
            std::string shape = valueToString(args[0]);
            double x = asNumber(args[1]);
            double y = asNumber(args[2]);
            double z = asNumber(args[3]);
            double mass = asNumber(args[4]);
            
            auto rigidBody = physicsEngine.createRigidBody(shape, x, y, z, mass);
            auto rbMap = std::make_shared<ClawHashMap>();
            rbMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(rigidBody)));
            rbMap->set("shape", stringValue(StringPool::intern(shape).data()));
            rbMap->set("mass", numberToValue(mass));
            
            return hashMapValue(rbMap);
        },
        "physics.createRigidBody"
    )));
    
    // Create force field
    physics->set("createForceField", callableValue(std::make_shared<NativeFunction>(
        6,
        [](const std::vector<Value>& args) -> Value {
            std::string type = valueToString(args[0]);
            double strength = asNumber(args[1]);
            double x = asNumber(args[2]);
            double y = asNumber(args[3]);
            double z = asNumber(args[4]);
            double range = asNumber(args[5]);
            
            auto forceField = physicsEngine.createForceField(type, strength, x, y, z, range);
            auto ffMap = std::make_shared<ClawHashMap>();
            ffMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(forceField)));
            ffMap->set("type", stringValue(StringPool::intern(type).data()));
            ffMap->set("strength", numberToValue(strength));
            ffMap->set("range", numberToValue(range));
            
            return hashMapValue(ffMap);
        },
        "physics.createForceField"
    )));
    
    // Simulate physics
    physics->set("simulate", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            double dt = asNumber(args[0]);
            physicsEngine.simulate(dt);
            return boolValue(true);
        },
        "physics.simulate"
    )));
    
    // Get total energy
    physics->set("getTotalEnergy", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            double energy = physicsEngine.getTotalEnergy();
            return numberToValue(energy);
        },
        "physics.getTotalEnergy"
    )));
    
    globals->define("physics", hashMapValue(physics));
    
    // Convenience functions
    globals->define("createParticle", callableValue(std::make_shared<NativeFunction>(
        5,
        [physics](const std::vector<Value>& args) -> Value {
            return physics->get("createParticle")->call(*nullptr, args);
        },
        "createParticle"
    )));
    
    globals->define("simulatePhysics", callableValue(std::make_shared<NativeFunction>(
        1,
        [physics](const std::vector<Value>& args) -> Value {
            return physics->get("simulate")->call(*nullptr, args);
        },
        "simulatePhysics"
    )));
}

} // namespace claw
