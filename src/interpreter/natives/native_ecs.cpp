#include "native_ecs.h"
#include "interpreter/interpreter.h"
#include "features/callable.h"
#include "features/class.h"

namespace claw {

// Simple ECS implementation using hash maps
struct SimpleECSWorld {
    std::unordered_map<std::string, std::unordered_map<std::string, Value>> entities;
    int nextId = 1;
};

static std::unordered_map<int, std::unique_ptr<SimpleECSWorld>> ecsWorlds;
static int nextWorldId = 1;

void registerNativeECS(Environment& env) {
    // ECS World creation
    env.define("ecsCreateWorld", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            int id = nextWorldId++;
            ecsWorlds[id] = std::make_unique<SimpleECSWorld>();
            return numberToValue(id);
        },
        "ecsCreateWorld"
    ));
    
    // Create entity
    env.define("ecsCreateEntity", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (args.size() < 1 || !isNumber(args[0])) return nilValue();
            int worldId = static_cast<int>(asNumber(args[0]));
            auto it = ecsWorlds.find(worldId);
            if (it == ecsWorlds.end()) return nilValue();
            
            std::string entityId = "entity_" + std::to_string(it->second->nextId++);
            it->second->entities[entityId] = {};
            auto sv = StringPool::intern(entityId);
            return stringValue(sv.data());
        },
        "ecsCreateEntity"
    ));
    
    // Add component
    env.define("ecsSetComponent", std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            if (args.size() < 4 || !isNumber(args[0])) return nilValue();
            int worldId = static_cast<int>(asNumber(args[0]));
            auto it = ecsWorlds.find(worldId);
            if (it == ecsWorlds.end()) return nilValue();
            
            std::string entityId = asString(args[1]);
            std::string compName = asString(args[2]);
            Value compValue = args[3];
            
            it->second->entities[entityId][compName] = compValue;
            return compValue;
        },
        "ecsSetComponent"
    ));
    
    // Get component
    env.define("ecsGetComponent", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (args.size() < 3 || !isNumber(args[0])) return nilValue();
            int worldId = static_cast<int>(asNumber(args[0]));
            auto it = ecsWorlds.find(worldId);
            if (it == ecsWorlds.end()) return nilValue();
            
            std::string entityId = asString(args[1]);
            std::string compName = asString(args[2]);
            
            auto& entity = it->second->entities[entityId];
            auto compIt = entity.find(compName);
            if (compIt != entity.end()) {
                return compIt->second;
            }
            return nilValue();
        },
        "ecsGetComponent"
    ));
    
    // Check if entity has component
    env.define("ecsHasComponent", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (args.size() < 3 || !isNumber(args[0])) return boolValue(false);
            int worldId = static_cast<int>(asNumber(args[0]));
            auto it = ecsWorlds.find(worldId);
            if (it == ecsWorlds.end()) return boolValue(false);
            
            std::string entityId = asString(args[1]);
            std::string compName = asString(args[2]);
            
            auto& entity = it->second->entities[entityId];
            return boolValue(entity.count(compName) > 0);
        },
        "ecsHasComponent"
    ));
    
    // Remove component
    env.define("ecsRemoveComponent", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            if (args.size() < 3 || !isNumber(args[0])) return nilValue();
            int worldId = static_cast<int>(asNumber(args[0]));
            auto it = ecsWorlds.find(worldId);
            if (it == ecsWorlds.end()) return nilValue();
            
            std::string entityId = asString(args[1]);
            std::string compName = asString(args[2]);
            
            it->second->entities[entityId].erase(compName);
            return nilValue();
        },
        "ecsRemoveComponent"
    ));
    
    // Destroy entity
    env.define("ecsDestroyEntity", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (args.size() < 2 || !isNumber(args[0])) return nilValue();
            int worldId = static_cast<int>(asNumber(args[0]));
            auto it = ecsWorlds.find(worldId);
            if (it == ecsWorlds.end()) return nilValue();
            
            std::string entityId = asString(args[1]);
            it->second->entities.erase(entityId);
            return nilValue();
        },
        "ecsDestroyEntity"
    ));
    
    // Query entities with component
    env.define("ecsQuery", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {  // NOLINT
            if (args.size() < 2 || !isNumber(args[0])) {
                auto sv = StringPool::intern("[]");
                return stringValue(sv.data());
            }
            int worldId = static_cast<int>(asNumber(args[0]));
            auto it = ecsWorlds.find(worldId);
            if (it == ecsWorlds.end()) {
                auto sv = StringPool::intern("[]");
                return stringValue(sv.data());
            }
            
            std::string compName = asString(args[1]);
            std::vector<std::string> matching;
            
            for (auto& entityPair : it->second->entities) {
                if (entityPair.second.count(compName)) {
                    matching.push_back(entityPair.first);
                }
            }

            
            // Build JSON-like array string
            std::string result = "[";
            for (size_t i = 0; i < matching.size(); i++) {
                if (i > 0) result += ",";
                result += "\"" + matching[i] + "\"";
            }
            result += "]";
            
            auto sv = StringPool::intern(result);
            return stringValue(sv.data());
        },
        "ecsQuery"
    ));
    
    // Get all entities
    env.define("ecsGetAllEntities", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (args.size() < 1 || !isNumber(args[0])) {
                auto sv = StringPool::intern("[]");
                return stringValue(sv.data());
            }
            int worldId = static_cast<int>(asNumber(args[0]));
            auto it = ecsWorlds.find(worldId);
            if (it == ecsWorlds.end()) {
                auto sv = StringPool::intern("[]");
                return stringValue(sv.data());
            }
            
            std::string result = "[";
            bool first = true;
            for (auto& entityPair : it->second->entities) {
                if (!first) result += ",";
                first = false;
                result += "\"" + entityPair.first + "\"";
            }

            result += "]";
            
            auto sv = StringPool::intern(result);
            return stringValue(sv.data());
        },
        "ecsGetAllEntities"
    ));
    
    // Destroy world
    env.define("ecsDestroyWorld", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (args.size() < 1 || !isNumber(args[0])) return nilValue();
            int worldId = static_cast<int>(asNumber(args[0]));
            ecsWorlds.erase(worldId);
            return nilValue();
        },
        "ecsDestroyWorld"
    ));
}

} // namespace claw
