#include "native_quantum.h"
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
#include <complex>
#include <random>

namespace claw {

// ============================================================
// Quantum Computing Implementation
// ============================================================
class QuantumComputer {
private:
    std::mt19937 rng;
    
public:
    struct Qubit {
        std::complex<double> alpha; // |0⟩ amplitude
        std::complex<double> beta;  // |1⟩ amplitude
        
        Qubit() : alpha(1.0, 0.0), beta(0.0, 0.0) {} // |0⟩ state
        
        Qubit(double alphaProb, double betaProb) {
            double norm = sqrt(alphaProb * alphaProb + betaProb * betaProb);
            alpha = std::complex<double>(alphaProb / norm, 0.0);
            beta = std::complex<double>(betaProb / norm, 0.0);
        }
        
        void applyHadamard() {
            std::complex<double> newAlpha = (alpha + beta) / sqrt(2.0);
            std::complex<double> newBeta = (alpha - beta) / sqrt(2.0);
            alpha = newAlpha;
            beta = newBeta;
        }
        
        void applyX() {
            std::swap(alpha, beta);
        }
        
        void applyZ() {
            beta = -beta;
        }
        
        void applyY() {
            std::complex<double> newBeta = -std::complex<double>(0.0, 1.0) * alpha;
            alpha = std::complex<double>(0.0, 1.0) * beta;
            beta = newBeta;
        }
        
        int measure() {
            double probZero = std::norm(alpha);
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            
            if (dist(rng) < probZero) {
                // Collapse to |0⟩
                alpha = std::complex<double>(1.0, 0.0);
                beta = std::complex<double>(0.0, 0.0);
                return 0;
            } else {
                // Collapse to |1⟩
                alpha = std::complex<double>(0.0, 0.0);
                beta = std::complex<double>(1.0, 0.0);
                return 1;
            }
        }
        
        std::string getState() const {
            return "(" + std::to_string(alpha.real()) + "+i" + std::to_string(alpha.imag()) + ")|0⟩ + (" +
                   std::to_string(beta.real()) + "+i" + std::to_string(beta.imag()) + ")|1⟩";
        }
    };
    
    struct QuantumCircuit {
        std::vector<Qubit> qubits;
        
        QuantumCircuit(int numQubits) {
            qubits.resize(numQubits);
        }
        
        void addQubit(double alphaProb = 1.0, double betaProb = 0.0) {
            qubits.emplace_back(alphaProb, betaProb);
        }
        
        void applyHadamard(int qubitIndex) {
            if (qubitIndex >= 0 && qubitIndex < qubits.size()) {
                qubits[qubitIndex].applyHadamard();
            }
        }
        
        void applyX(int qubitIndex) {
            if (qubitIndex >= 0 && qubitIndex < qubits.size()) {
                qubits[qubitIndex].applyX();
            }
        }
        
        void applyY(int qubitIndex) {
            if (qubitIndex >= 0 && qubitIndex < qubits.size()) {
                qubits[qubitIndex].applyY();
            }
        }
        
        void applyZ(int qubitIndex) {
            if (qubitIndex >= 0 && qubitIndex < qubits.size()) {
                qubits[qubitIndex].applyZ();
            }
        }
        
        std::vector<int> measureAll() {
            std::vector<int> results;
            for (auto& qubit : qubits) {
                results.push_back(qubit.measure());
            }
            return results;
        }
        
        int getNumQubits() const {
            return qubits.size();
        }
    };
    
    QuantumComputer() : rng(std::random_device{}()) {}
    
    QuantumCircuit* createCircuit(int numQubits) {
        return new QuantumCircuit(numQubits);
    }
    
    void destroyCircuit(QuantumCircuit* circuit) {
        delete circuit;
    }
};

// ============================================================
// Register quantum computing natives
// ============================================================
void registerNativeQuantum(const std::shared_ptr<Environment>& globals) {

    static QuantumComputer quantumComputer;
    
    // Quantum object with methods
    auto quantum = std::make_shared<ClawHashMap>();
    
    // Create quantum circuit
    quantum->set("createCircuit", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            int numQubits = static_cast<int>(asNumber(args[0]));
            
            auto circuit = quantumComputer.createCircuit(numQubits);
            auto circuitMap = std::make_shared<ClawHashMap>();
            circuitMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(circuit)));
            circuitMap->set("numQubits", numberToValue(numQubits));
            
            return hashMapValue(circuitMap);
        },
        "quantum.createCircuit"
    )));
    
    // Apply Hadamard gate
    quantum->set("hadamard", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto circuitMap = asHashMap(args[0]);
            int qubitIndex = static_cast<int>(asNumber(args[1]));
            
            auto ptrValue = circuitMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto circuit = static_cast<QuantumComputer::QuantumCircuit*>(ptr);
            
            circuit->applyHadamard(qubitIndex);
            return boolValue(true);
        },
        "quantum.hadamard"
    )));
    
    // Apply X gate
    quantum->set("x", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto circuitMap = asHashMap(args[0]);
            int qubitIndex = static_cast<int>(asNumber(args[1]));
            
            auto ptrValue = circuitMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto circuit = static_cast<QuantumComputer::QuantumCircuit*>(ptr);
            
            circuit->applyX(qubitIndex);
            return boolValue(true);
        },
        "quantum.x"
    )));
    
    // Apply Y gate
    quantum->set("y", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto circuitMap = asHashMap(args[0]);
            int qubitIndex = static_cast<int>(asNumber(args[1]));
            
            auto ptrValue = circuitMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto circuit = static_cast<QuantumComputer::QuantumCircuit*>(ptr);
            
            circuit->applyY(qubitIndex);
            return boolValue(true);
        },
        "quantum.y"
    )));
    
    // Apply Z gate
    quantum->set("z", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto circuitMap = asHashMap(args[0]);
            int qubitIndex = static_cast<int>(asNumber(args[1]));
            
            auto ptrValue = circuitMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto circuit = static_cast<QuantumComputer::QuantumCircuit*>(ptr);
            
            circuit->applyZ(qubitIndex);
            return boolValue(true);
        },
        "quantum.z"
    )));
    
    // Measure all qubits
    quantum->set("measure", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto circuitMap = asHashMap(args[0]);
            
            auto ptrValue = circuitMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto circuit = static_cast<QuantumComputer::QuantumCircuit*>(ptr);
            
            auto results = circuit->measureAll();
            auto resultArray = std::make_shared<ClawArray>();
            
            for (int result : results) {
                resultArray->push(numberToValue(result));
            }
            
            return arrayValue(resultArray);
        },
        "quantum.measure"
    )));
    
    globals->define("quantum", hashMapValue(quantum));
    
    // Convenience functions
    globals->define("createQuantumCircuit", callableValue(std::make_shared<NativeFunction>(
        1,
        [quantum](const std::vector<Value>& args) -> Value {
            return quantum->get("createCircuit")->call(*nullptr, args);
        },
        "createQuantumCircuit"
    )));
}

} // namespace claw
