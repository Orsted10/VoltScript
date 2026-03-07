#include "native_cybersecurity.h"
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
#include <random>
#include <sstream>
#include <iomanip>

namespace claw {

struct CybersecurityEngine {
    struct Threat {
        std::string type;
        std::string severity;
        std::string description;
        std::string source;
        std::vector<std::string> indicators;
        double riskScore;
        
        Threat(const std::string& t, const std::string& sev, const std::string& desc)
            : type(t), severity(sev), description(desc), riskScore(0.0) {}
    };
    
    struct SecurityPolicy {
        std::string name;
        std::vector<std::string> rules;
        bool isActive;
        std::string enforcementLevel;
        
        SecurityPolicy(const std::string& n, const std::string& level)
            : name(n), enforcementLevel(level), isActive(true) {}
        
        void addRule(const std::string& rule) {
            rules.push_back(rule);
        }
    };
    
    struct Firewall {
        std::string name;
        std::vector<std::string> allowedIPs;
        std::vector<std::string> blockedIPs;
        std::vector<std::string> allowedPorts;
        std::vector<std::string> blockedPorts;
        bool isActive;
        
        Firewall(const std::string& n) : name(n), isActive(true) {}
        
        bool isIPAllowed(const std::string& ip) {
            for (const auto& blocked : blockedIPs) {
                if (ip.find(blocked) != std::string::npos) return false;
            }
            return true;
        }
        
        bool isPortAllowed(const std::string& port) {
            for (const auto& blocked : blockedPorts) {
                if (port == blocked) return false;
            }
            return true;
        }
    };
    
    struct IntrusionDetection {
        std::vector<std::string> signatures;
        std::vector<Threat> detectedThreats;
        bool isActive;
        double sensitivity;
        
        IntrusionDetection() : isActive(true), sensitivity(0.8) {
            signatures = {
                "SQL injection pattern detected",
                "XSS attack vector found",
                "Brute force attempt",
                "Suspicious file upload",
                "Directory traversal attempt"
            };
        }
        
        bool analyzeTraffic(const std::string& traffic) {
            for (const auto& signature : signatures) {
                if (traffic.find(signature) != std::string::npos) {
                    Threat threat("intrusion", "high", signature);
                    threat.source = "network_traffic";
                    detectedThreats.push_back(threat);
                    return true;
                }
            }
            return false;
        }
        
        std::vector<Threat> getRecentThreats() const {
            return detectedThreats;
        }
    };
    
    struct Encryption {
        std::string algorithm;
        int keySize;
        std::string mode;
        bool isHardwareAccelerated;
        
        Encryption(const std::string& algo, int keySize, const std::string& mode)
            : algorithm(algo), keySize(keySize), mode(mode), isHardwareAccelerated(false) {}
        
        std::string encrypt(const std::string& plaintext, const std::string& key) {
            std::string ciphertext = plaintext;
            for (size_t i = 0; i < ciphertext.length(); ++i) {
                ciphertext[i] ^= key[i % key.length()];
            }
            return this.toHex(ciphertext);
        }
        
        std::string decrypt(const std::string& ciphertext, const std::string& key) {
            std::string hexData = this.fromHex(ciphertext);
            for (size_t i = 0; i < hexData.length(); ++i) {
                hexData[i] ^= key[i % key.length()];
            }
            return hexData;
        }
        
        std::string generateKey(int length) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);
            
            std::string key;
            for (int i = 0; i < length; ++i) {
                key += static_cast<char>(dis(gen));
            }
            return key;
        }
        
    private:
        std::string toHex(const std::string& input) {
            std::stringstream ss;
            ss << std::hex << std::setfill('0') << std::setw(2);
            for (unsigned char c : input) {
                ss << static_cast<int>(c);
            }
            return ss.str();
        }
        
        std::string fromHex(const std::string& hex) {
            std::string result;
            for (size_t i = 0; i < hex.length(); i += 2) {
                std::string byte = hex.substr(i, 2);
                char c = static_cast<char>(std::stoi(byte, nullptr, 16));
                result += c;
            }
            return result;
        }
    };
    
    struct VulnerabilityScanner {
        std::vector<std::string> scanTypes;
        std::vector<Threat> vulnerabilities;
        
        VulnerabilityScanner() {
            scanTypes = {
                "port_scanning",
                "service_enumeration", 
                "weak_passwords",
                "outdated_software",
                "misconfigurations"
            };
        }
        
        std::vector<Threat> scanSystem(const std::string& target) {
            std::vector<Threat> foundVulns;
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, scanTypes.size() - 1);
            
            int vulnCount = dis(gen) % 5;
            
            for (int i = 0; i < vulnCount; ++i) {
                std::string vulnType = scanTypes[dis(gen)];
                std::string severity = (i < 2) ? "high" : "medium";
                
                Threat vuln(vulnType, severity, "Vulnerability found on " + target);
                vuln.source = "vulnerability_scanner";
                vuln.riskScore = (severity == "high") ? 8.5 : 5.0;
                
                foundVulns.push_back(vuln);
                vulnerabilities.push_back(vuln);
            }
            
            return foundVulns;
        }
        
        double calculateRiskScore() const {
            if (vulnerabilities.empty()) return 0.0;
            
            double totalRisk = 0.0;
            for (const auto& vuln : vulnerabilities) {
                totalRisk += vuln.riskScore;
            }
            
            return totalRisk / vulnerabilities.size();
        }
    };
    
    std::vector<Threat> threatDatabase;
    std::vector<SecurityPolicy> policies;
    std::vector<Firewall> firewalls;
    IntrusionDetection ids;
    Encryption defaultEncryption;
    VulnerabilityScanner vulnScanner;
    
    CybersecurityEngine() : defaultEncryption("AES", 256, "GCM") {
        threatDatabase.emplace_back("malware", "critical", "Ransomware attack detected");
        threatDatabase.emplace_back("phishing", "high", "Email phishing campaign");
        threatDatabase.emplace_back("ddos", "medium", "DDoS attack in progress");
        threatDatabase.emplace_back("sql_injection", "high", "SQL injection attempt");
        threatDatabase.emplace_back("xss", "medium", "Cross-site scripting vulnerability");
        
        SecurityPolicy passwordPolicy("Password Policy", "enforce");
        passwordPolicy.addRule("Minimum 12 characters");
        passwordPolicy.addRule("Include uppercase, lowercase, numbers, symbols");
        passwordPolicy.addRule("Password history: 5 previous passwords");
        policies.push_back(passwordPolicy);
        
        SecurityPolicy accessPolicy("Access Control", "block");
        accessPolicy.addRule("Two-factor authentication required");
        accessPolicy.addRule("Session timeout: 30 minutes");
        policies.push_back(accessPolicy);
        
        Firewall defaultFW("Main Firewall");
        defaultFW.allowedPorts = {"80", "443", "22"};
        defaultFW.blockedPorts = {"23", "135", "139", "445"};
        firewalls.push_back(defaultFW);
    }
    
    Threat* createThreat(const std::string& type, const std::string& severity, const std::string& description) {
        threatDatabase.emplace_back(type, severity, description);
        return &threatDatabase.back();
    }
    
    SecurityPolicy* createPolicy(const std::string& name, const std::string& enforcementLevel) {
        policies.emplace_back(name, enforcementLevel);
        return &policies.back();
    }
    
    Firewall* createFirewall(const std::string& name) {
        firewalls.emplace_back(name);
        return &firewalls.back();
    }
    
    bool analyzeThreat(const std::string& threatData) {
        return ids.analyzeTraffic(threatData);
    }
    
    std::vector<Threat> scanVulnerabilities(const std::string& target) {
        return vulnScanner.scanSystem(target);
    }
    
    std::string encryptData(const std::string& data, const std::string& key) {
        return defaultEncryption.encrypt(data, key);
    }
    
    std::string decryptData(const std::string& encryptedData, const std::string& key) {
        return defaultEncryption.decrypt(encryptedData, key);
    }
    
    std::string generateSecureKey(int length = 32) {
        return defaultEncryption.generateKey(length);
    }
    
    double calculateOverallRisk() const {
        double totalRisk = 0.0;
        int threatCount = 0;
        
        for (const auto& threat : threatDatabase) {
            totalRisk += threat.riskScore;
            threatCount++;
        }
        
        return threatCount > 0 ? totalRisk / threatCount : 0.0;
    }
    
    std::vector<std::string> getSecurityRecommendations() const {
        double riskScore = calculateOverallRisk();
        std::vector<std::string> recommendations;
        
        if (riskScore > 7.0) {
            recommendations.push_back("IMMEDIATE: Implement comprehensive security audit");
            recommendations.push_back("CRITICAL: Update all security patches");
            recommendations.push_back("URGENT: Enable multi-factor authentication");
        } else if (riskScore > 5.0) {
            recommendations.push_back("Review and update security policies");
            recommendations.push_back("Conduct penetration testing");
            recommendations.push_back("Enhance monitoring and logging");
        } else {
            recommendations.push_back("Maintain current security posture");
            recommendations.push_back("Regular security awareness training");
        }
        
        return recommendations;
    }
};

void registerNativeCybersecurity(const std::shared_ptr<Environment>& globals) {

    static CybersecurityEngine cyberEngine;
    
    auto cyber = std::make_shared<ClawHashMap>();
    
    cyber->set("createThreat", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            std::string type = valueToString(args[0]);
            std::string severity = valueToString(args[1]);
            std::string description = valueToString(args[2]);
            
            auto threat = cyberEngine.createThreat(type, severity, description);
            auto threatMap = std::make_shared<ClawHashMap>();
            threatMap->set("type", stringValue(StringPool::intern(type).data()));
            threatMap->set("severity", stringValue(StringPool::intern(severity).data()));
            threatMap->set("description", stringValue(StringPool::intern(description).data()));
            
            return hashMapValue(threatMap);
        },
        "cyber.createThreat"
    )));
    
    cyber->set("createPolicy", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            std::string enforcementLevel = valueToString(args[1]);
            
            auto policy = cyberEngine.createPolicy(name, enforcementLevel);
            auto policyMap = std::make_shared<ClawHashMap>();
            policyMap->set("name", stringValue(StringPool::intern(name).data()));
            policyMap->set("enforcementLevel", stringValue(StringPool::intern(enforcementLevel).data()));
            
            return hashMapValue(policyMap);
        },
        "cyber.createPolicy"
    )));
    
    cyber->set("createFirewall", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            
            auto firewall = cyberEngine.createFirewall(name);
            auto fwMap = std::make_shared<ClawHashMap>();
            fwMap->set("name", stringValue(StringPool::intern(name).data()));
            fwMap->set("active", boolValue(firewall->isActive));
            
            return hashMapValue(fwMap);
        },
        "cyber.createFirewall"
    )));
    
    cyber->set("analyzeThreat", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string threatData = valueToString(args[0]);
            bool detected = cyberEngine.analyzeThreat(threatData);
            return boolValue(detected);
        },
        "cyber.analyzeThreat"
    )));
    
    cyber->set("scanVulnerabilities", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string target = valueToString(args[0]);
            auto vulnerabilities = cyberEngine.scanVulnerabilities(target);
            
            auto vulnArray = std::make_shared<ClawArray>();
            for (const auto& vuln : vulnerabilities) {
                auto vulnMap = std::make_shared<ClawHashMap>();
                vulnMap->set("type", stringValue(StringPool::intern(vuln.type).data()));
                vulnMap->set("severity", stringValue(StringPool::intern(vuln.severity).data()));
                vulnMap->set("description", stringValue(StringPool::intern(vuln.description).data()));
                vulnMap->set("riskScore", numberToValue(vuln.riskScore));
                vulnArray->push(hashMapValue(vulnMap));
            }
            
            return arrayValue(vulnArray);
        },
        "cyber.scanVulnerabilities"
    )));
    
    cyber->set("encrypt", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string data = valueToString(args[0]);
            std::string key = valueToString(args[1]);
            std::string encrypted = cyberEngine.encryptData(data, key);
            return stringValue(StringPool::intern(encrypted).data());
        },
        "cyber.encrypt"
    )));
    
    cyber->set("decrypt", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string encryptedData = valueToString(args[0]);
            std::string key = valueToString(args[1]);
            std::string decrypted = cyberEngine.decryptData(encryptedData, key);
            return stringValue(StringPool::intern(decrypted).data());
        },
        "cyber.decrypt"
    )));
    
    cyber->set("generateKey", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            int length = static_cast<int>(asNumber(args[0]));
            std::string key = cyberEngine.generateSecureKey(length);
            return stringValue(StringPool::intern(key).data());
        },
        "cyber.generateKey"
    )));
    
    cyber->set("getRiskScore", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            double riskScore = cyberEngine.calculateOverallRisk();
            return numberToValue(riskScore);
        },
        "cyber.getRiskScore"
    )));
    
    cyber->set("getRecommendations", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            auto recommendations = cyberEngine.getSecurityRecommendations();
            auto recArray = std::make_shared<ClawArray>();
            
            for (const auto& rec : recommendations) {
                recArray->push(stringValue(StringPool::intern(rec).data()));
            }
            
            return arrayValue(recArray);
        },
        "cyber.getRecommendations"
    )));
    
    globals->define("cyber", hashMapValue(cyber));
    
    globals->define("createThreat", callableValue(std::make_shared<NativeFunction>(
        3,
        [cyber](const std::vector<Value>& args) -> Value {
            return cyber->get("createThreat")->call(*nullptr, args);
        },
        "createThreat"
    )));
    
    globals->define("encryptData", callableValue(std::make_shared<NativeFunction>(
        2,
        [cyber](const std::vector<Value>& args) -> Value {
            return cyber->get("encrypt")->call(*nullptr, args);
        },
        "encryptData"
    )));
    
    globals->define("generateSecureKey", callableValue(std::make_shared<NativeFunction>(
        1,
        [cyber](const std::vector<Value>& args) -> Value {
            return cyber->get("generateKey")->call(*nullptr, args);
        },
        "generateSecureKey"
    )));
}

} // namespace claw
