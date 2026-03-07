#include "native_crypto.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <random>
#include <sstream>
#include <iomanip>

namespace claw {

// ============================================================
// Crypto Implementation
// ============================================================
class CryptoUtils {
public:
    static std::string hash(const std::string& input) {
        // Simple hash implementation (in production, use proper crypto)
        std::hash<std::string> hasher;
        size_t hashValue = hasher(input);
        
        std::stringstream ss;
        ss << std::hex << hashValue;
        return ss.str();
    }
    
    static std::string encrypt(const std::string& plaintext, const std::string& key) {
        // Simple XOR encryption (in production, use proper encryption)
        std::string encrypted;
        for (size_t i = 0; i < plaintext.length(); ++i) {
            encrypted += plaintext[i] ^ key[i % key.length()];
        }
        return encrypted;
    }
    
    static std::string decrypt(const std::string& ciphertext, const std::string& key) {
        // Simple XOR decryption (in production, use proper decryption)
        return encrypt(ciphertext, key); // XOR is symmetric
    }
    
    static std::string generateUUID() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        std::stringstream ss;
        ss << std::hex;
        
        for (int i = 0; i < 32; ++i) {
            if (i == 8 || i == 12 || i == 16 || i == 20) ss << "-";
            ss << dis(gen);
        }
        
        return ss.str();
    }
    
    static std::string randomString(int length) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(33, 126); // Printable ASCII
        
        std::string result;
        for (int i = 0; i < length; ++i) {
            result += static_cast<char>(dis(gen));
        }
        return result;
    }
};

// ============================================================
// Register crypto natives
// ============================================================
void registerNativeCrypto(const std::shared_ptr<Environment>& globals) {

    // Crypto object with methods
    auto crypto = std::make_shared<ClawHashMap>();
    
    // Hash function
    crypto->set("hash", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string input = valueToString(args[0]);
            std::string hashResult = CryptoUtils::hash(input);
            return stringValue(StringPool::intern(hashResult).data());
        },
        "crypto.hash"
    )));
    
    // Encrypt function
    crypto->set("encrypt", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string plaintext = valueToString(args[0]);
            std::string key = valueToString(args[1]);
            std::string encrypted = CryptoUtils::encrypt(plaintext, key);
            return stringValue(StringPool::intern(encrypted).data());
        },
        "crypto.encrypt"
    )));
    
    // Decrypt function
    crypto->set("decrypt", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string ciphertext = valueToString(args[0]);
            std::string key = valueToString(args[1]);
            std::string decrypted = CryptoUtils::decrypt(ciphertext, key);
            return stringValue(StringPool::intern(decrypted).data());
        },
        "crypto.decrypt"
    )));
    
    // UUID generator
    crypto->set("uuid", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            std::string uuid = CryptoUtils::generateUUID();
            return stringValue(StringPool::intern(uuid).data());
        },
        "crypto.uuid"
    )));
    
    // Random string generator
    crypto->set("random", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            int length = static_cast<int>(asNumber(args[0]));
            std::string random = CryptoUtils::randomString(length);
            return stringValue(StringPool::intern(random).data());
        },
        "crypto.random"
    )));
    
    globals->define("crypto", hashMapValue(crypto));
    
    // Convenience functions
    globals->define("hash", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string input = valueToString(args[0]);
            std::string hashResult = CryptoUtils::hash(input);
            return stringValue(StringPool::intern(hashResult).data());
        },
        "hash"
    )));
    
    globals->define("encrypt", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string plaintext = valueToString(args[0]);
            std::string key = valueToString(args[1]);
            std::string encrypted = CryptoUtils::encrypt(plaintext, key);
            return stringValue(StringPool::intern(encrypted).data());
        },
        "encrypt"
    )));
    
    globals->define("decrypt", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string ciphertext = valueToString(args[0]);
            std::string key = valueToString(args[1]);
            std::string decrypted = CryptoUtils::decrypt(ciphertext, key);
            return stringValue(StringPool::intern(decrypted).data());
        },
        "decrypt"
    )));
    
    globals->define("uuid", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            std::string uuid = CryptoUtils::generateUUID();
            return stringValue(StringPool::intern(uuid).data());
        },
        "uuid"
    )));
}

} // namespace claw
