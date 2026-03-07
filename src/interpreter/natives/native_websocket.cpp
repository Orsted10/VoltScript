#include "native_websocket.h"
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
// WebSocket Implementation
// ============================================================
struct WebSocketConnection {
    std::string url;
    bool isConnected = false;
    std::vector<std::string> messageQueue;
    
    bool connect(const std::string& wsUrl) {
        url = wsUrl;
        isConnected = true;
        return true; // Mock implementation
    }
    
    bool send(const std::string& message) {
        if (!isConnected) return false;
        messageQueue.push_back("SENT: " + message);
        return true;
    }
    
    std::string receive() {
        if (!isConnected || messageQueue.empty()) {
            return "";
        }
        
        std::string message = messageQueue.front();
        messageQueue.erase(messageQueue.begin());
        return message;
    }
    
    void close() {
        isConnected = false;
        messageQueue.clear();
    }
};

// ============================================================
// Register WebSocket natives
// ============================================================
void registerNativeWebSocket(const std::shared_ptr<Environment>& globals) {

    // WebSocket object with methods
    auto ws = std::make_shared<ClawHashMap>();
    
    // Connect to WebSocket
    ws->set("connect", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string url = valueToString(args[0]);
            
            auto connection = std::make_shared<WebSocketConnection>();
            bool success = connection->connect(url);
            
            auto wsMap = std::make_shared<ClawHashMap>();
            wsMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(connection.get())));
            wsMap->set("connected", boolValue(success));
            wsMap->set("url", stringValue(StringPool::intern(url).data()));
            
            return hashMapValue(wsMap);
        },
        "ws.connect"
    )));
    
    // Send message
    ws->set("send", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto wsMap = asHashMap(args[0]);
            std::string message = valueToString(args[1]);
            
            auto ptrValue = wsMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto connection = static_cast<WebSocketConnection*>(ptr);
            
            bool success = connection->send(message);
            return boolValue(success);
        },
        "ws.send"
    )));
    
    // Receive message
    ws->set("receive", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto wsMap = asHashMap(args[0]);
            
            auto ptrValue = wsMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto connection = static_cast<WebSocketConnection*>(ptr);
            
            std::string message = connection->receive();
            return stringValue(StringPool::intern(message).data());
        },
        "ws.receive"
    )));
    
    // Close connection
    ws->set("close", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto wsMap = asHashMap(args[0]);
            
            auto ptrValue = wsMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto connection = static_cast<WebSocketConnection*>(ptr);
            
            connection->close();
            return boolValue(true);
        },
        "ws.close"
    )));
    
    globals->define("ws", hashMapValue(ws));
    
    // Convenience functions
    globals->define("connectWebSocket", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string url = valueToString(args[0]);
            
            auto connection = std::make_shared<WebSocketConnection>();
            bool success = connection->connect(url);
            
            auto wsMap = std::make_shared<ClawHashMap>();
            wsMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(connection.get())));
            wsMap->set("connected", boolValue(success));
            wsMap->set("url", stringValue(StringPool::intern(url).data()));
            
            return hashMapValue(wsMap);
        },
        "connectWebSocket"
    )));
}

} // namespace claw
