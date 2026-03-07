#include "native_blockchain.h"
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
#include <sstream>
#include <iomanip>

namespace claw {

// ============================================================
// Blockchain Implementation
// ============================================================
struct Block {
    int index;
    std::string timestamp;
    std::string data;
    std::string previousHash;
    std::string hash;
    
    Block(int idx, const std::string& ts, const std::string& d, const std::string& prevHash) 
        : index(idx), timestamp(ts), data(d), previousHash(prevHash) {
        hash = calculateHash();
    }
    
    std::string calculateHash() const {
        std::stringstream ss;
        ss << index << timestamp << data << previousHash;
        
        // Simple hash (in production, use SHA-256)
        std::hash<std::string> hasher;
        size_t hashValue = hasher(ss.str());
        
        std::stringstream hex;
        hex << std::hex << hashValue;
        return hex.str();
    }
};

struct Blockchain {
    std::vector<Block> chain;
    int difficulty = 4;
    
    Blockchain() {
        // Create genesis block
        chain.emplace_back(0, getCurrentTime(), "Genesis Block", "0");
    }
    
    std::string getCurrentTime() const {
        auto now = std::time(nullptr);
        std::stringstream ss;
        ss << now;
        return ss.str();
    }
    
    Block getLatestBlock() const {
        return chain.back();
    }
    
    void addBlock(const std::string& data) {
        Block latestBlock = getLatestBlock();
        int newIndex = latestBlock.index + 1;
        std::string newTimestamp = getCurrentTime();
        
        Block newBlock(newIndex, newTimestamp, data, latestBlock.hash);
        chain.push_back(newBlock);
    }
    
    bool isValid() const {
        for (size_t i = 1; i < chain.size(); ++i) {
            const Block& currentBlock = chain[i];
            const Block& previousBlock = chain[i - 1];
            
            if (currentBlock.previousHash != previousBlock.hash) {
                return false;
            }
            
            if (currentBlock.hash != currentBlock.calculateHash()) {
                return false;
            }
        }
        return true;
    }
    
    int getLength() const {
        return chain.size();
    }
};

// ============================================================
// Register blockchain natives
// ============================================================
void registerNativeBlockchain(const std::shared_ptr<Environment>& globals) {

    static Blockchain blockchain;
    
    // Blockchain object with methods
    auto bc = std::make_shared<ClawHashMap>();
    
    // Create blockchain
    bc->set("create", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            auto newChain = std::make_shared<Blockchain>();
            auto chainMap = std::make_shared<ClawHashMap>();
            chainMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(newChain.get())));
            return hashMapValue(chainMap);
        },
        "bc.create"
    )));
    
    // Add block
    bc->set("addBlock", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto chainMap = asHashMap(args[0]);
            std::string data = valueToString(args[1]);
            
            auto ptrValue = chainMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto chain = static_cast<Blockchain*>(ptr);
            
            chain->addBlock(data);
            return boolValue(true);
        },
        "bc.addBlock"
    )));
    
    // Get blockchain info
    bc->set("getInfo", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto chainMap = asHashMap(args[0]);
            
            auto ptrValue = chainMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto chain = static_cast<Blockchain*>(ptr);
            
            auto info = std::make_shared<ClawHashMap>();
            info->set("length", numberToValue(chain->getLength()));
            info->set("isValid", boolValue(chain->isValid()));
            
            return hashMapValue(info);
        },
        "bc.getInfo"
    )));
    
    // Mine block
    bc->set("mine", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto chainMap = asHashMap(args[0]);
            std::string data = valueToString(args[1]);
            
            auto ptrValue = chainMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto chain = static_cast<Blockchain*>(ptr);
            
            // Simple mining simulation
            for (int i = 0; i < 1000000; ++i) {
                // Simulate mining work
            }
            
            chain->addBlock("MINED: " + data);
            return boolValue(true);
        },
        "bc.mine"
    )));
    
    globals->define("blockchain", hashMapValue(bc));
    
    // Convenience functions
    globals->define("createBlockchain", callableValue(std::make_shared<NativeFunction>(
        0,
        [bc](const std::vector<Value>& args) -> Value {
            return bc->get("create")->call(*nullptr, args);
        },
        "createBlockchain"
    )));
}

} // namespace claw
