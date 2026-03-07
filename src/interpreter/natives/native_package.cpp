#include "native_package.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <fstream>

namespace claw {

// ============================================================
// Package Manager Implementation
// ============================================================
struct PackageManager {
    std::unordered_map<std::string, std::string> packages;
    std::string packageDir = "./packages";
    
    bool installPackage(const std::string& packageName) {
        // Mock implementation - in real scenario would download from repository
        packages[packageName] = "installed";
        return true;
    }
    
    bool uninstallPackage(const std::string& packageName) {
        if (packages.find(packageName) != packages.end()) {
            packages.erase(packageName);
            return true;
        }
        return false;
    }
    
    bool isInstalled(const std::string& packageName) {
        return packages.find(packageName) != packages.end();
    }
    
    std::vector<std::string> listPackages() {
        std::vector<std::string> result;
        for (const auto& [name, version] : packages) {
            result.push_back(name);
        }
        return result;
    }
};

// ============================================================
// Register package manager natives
// ============================================================
void registerNativePackage(const std::shared_ptr<Environment>& globals) {

    static PackageManager packageManager;
    
    // Package object with methods
    auto pkg = std::make_shared<ClawHashMap>();
    
    // Install package
    pkg->set("install", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string packageName = valueToString(args[0]);
            bool success = packageManager.installPackage(packageName);
            return boolValue(success);
        },
        "package.install"
    )));
    
    // Uninstall package
    pkg->set("uninstall", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string packageName = valueToString(args[0]);
            bool success = packageManager.uninstallPackage(packageName);
            return boolValue(success);
        },
        "package.uninstall"
    )));
    
    // Check if package is installed
    pkg->set("isInstalled", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string packageName = valueToString(args[0]);
            bool installed = packageManager.isInstalled(packageName);
            return boolValue(installed);
        },
        "package.isInstalled"
    )));
    
    // List packages
    pkg->set("list", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            auto packageList = packageManager.listPackages();
            auto result = std::make_shared<ClawArray>();
            
            for (const auto& packageName : packageList) {
                result->push(stringValue(StringPool::intern(packageName).data()));
            }
            
            return arrayValue(result);
        },
        "package.list"
    )));
    
    globals->define("package", hashMapValue(pkg));
    
    // Convenience functions
    globals->define("install", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string packageName = valueToString(args[0]);
            bool success = packageManager.installPackage(packageName);
            return boolValue(success);
        },
        "install"
    )));
    
    globals->define("uninstall", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string packageName = valueToString(args[0]);
            bool success = packageManager.uninstallPackage(packageName);
            return boolValue(success);
        },
        "uninstall"
    )));
    
    globals->define("packages", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            auto packageList = packageManager.listPackages();
            auto result = std::make_shared<ClawArray>();
            
            for (const auto& packageName : packageList) {
                result->push(stringValue(StringPool::intern(packageName).data()));
            }
            
            return arrayValue(result);
        },
        "packages"
    )));
}

} // namespace claw
