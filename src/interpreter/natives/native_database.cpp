#include "native_database.h"
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
// Database Connection Structure
// ============================================================
struct DatabaseConnection {
    std::string connectionString;
    bool isConnected = false;
    std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>> tables;
    
    bool connect(const std::string& connStr) {
        connectionString = connStr;
        isConnected = true;
        
        // Mock data for demonstration
        tables["users"] = {
            {{"id", "1"}, {"name", "Alice"}, {"email", "alice@example.com"}},
            {{"id", "2"}, {"name", "Bob"}, {"email", "bob@example.com"}}
        };
        
        return true;
    }
    
    std::vector<std::unordered_map<std::string, std::string>> query(const std::string& sql) {
        if (!isConnected) return {};
        
        // Simple mock query parser
        if (sql.find("SELECT") != std::string::npos && sql.find("users") != std::string::npos) {
            return tables["users"];
        }
        
        return {};
    }
    
    bool execute(const std::string& sql) {
        if (!isConnected) return false;
        
        // Mock execution
        return true;
    }
    
    void close() {
        isConnected = false;
        tables.clear();
    }
};

// ============================================================
// Register database natives
// ============================================================
void registerNativeDatabase(const std::shared_ptr<Environment>& globals) {

    // Database object with methods
    auto db = std::make_shared<ClawHashMap>();
    
    // Connect to database
    db->set("connect", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string connStr = valueToString(args[0]);
            
            auto connection = std::make_shared<DatabaseConnection>();
            bool success = connection->connect(connStr);
            
            auto dbMap = std::make_shared<ClawHashMap>();
            dbMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(connection.get())));
            dbMap->set("connected", boolValue(success));
            
            return hashMapValue(dbMap);
        },
        "db.connect"
    )));
    
    // Query database
    db->set("query", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto dbMap = asHashMap(args[0]);
            std::string sql = valueToString(args[1]);
            
            auto ptrValue = dbMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto connection = static_cast<DatabaseConnection*>(ptr);
            
            auto results = connection->query(sql);
            auto resultArray = std::make_shared<ClawArray>();
            
            for (const auto& row : results) {
                auto rowMap = std::make_shared<ClawHashMap>();
                for (const auto& [column, value] : row) {
                    rowMap->set(column, stringValue(StringPool::intern(value).data()));
                }
                resultArray->push(hashMapValue(rowMap));
            }
            
            return arrayValue(resultArray);
        },
        "db.query"
    )));
    
    // Execute SQL
    db->set("execute", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto dbMap = asHashMap(args[0]);
            std::string sql = valueToString(args[1]);
            
            auto ptrValue = dbMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto connection = static_cast<DatabaseConnection*>(ptr);
            
            bool success = connection->execute(sql);
            return boolValue(success);
        },
        "db.execute"
    )));
    
    globals->define("db", hashMapValue(db));
    
    // Convenience functions
    globals->define("connectDB", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string connStr = valueToString(args[0]);
            
            auto connection = std::make_shared<DatabaseConnection>();
            bool success = connection->connect(connStr);
            
            auto dbMap = std::make_shared<ClawHashMap>();
            dbMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(connection.get())));
            dbMap->set("connected", boolValue(success));
            
            return hashMapValue(dbMap);
        },
        "connectDB"
    )));
}

} // namespace claw
