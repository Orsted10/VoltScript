#include "native_optimization.h"
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
#include <random>

namespace claw {

// ============================================================
// Advanced Optimization Engine Implementation
// ============================================================
struct OptimizationEngine {
    // Genetic Algorithm
    struct GeneticAlgorithm {
        std::vector<std::vector<double>> population;
        std::vector<double> fitness;
        int populationSize;
        int geneLength;
        double mutationRate;
        double crossoverRate;
        std::mt19937 rng;
        
        GeneticAlgorithm(int popSize, int geneLen, double mutRate = 0.1, double crossRate = 0.8)
            : populationSize(popSize), geneLength(geneLen), mutationRate(mutRate), crossoverRate(crossRate),
              rng(std::random_device{}()) {
            
            // Initialize random population
            std::uniform_real_distribution<double> dist(-10.0, 10.0);
            population.resize(populationSize, std::vector<double>(geneLen));
            
            for (int i = 0; i < populationSize; ++i) {
                for (int j = 0; j < geneLength; ++j) {
                    population[i][j] = dist(rng);
                }
            }
        }
        
        void evaluateFitness(std::function<double(const std::vector<double>&)> fitnessFunction) {
            fitness.resize(populationSize);
            for (int i = 0; i < populationSize; ++i) {
                fitness[i] = fitnessFunction(population[i]);
            }
        }
        
        std::vector<double> selectParent() {
            // Tournament selection
            std::uniform_int_distribution<int> dist(0, populationSize - 1);
            int i1 = dist(rng);
            int i2 = dist(rng);
            
            return (fitness[i1] > fitness[i2]) ? population[i1] : population[i2];
        }
        
        std::vector<double> crossover(const std::vector<double>& parent1, const std::vector<double>& parent2) {
            std::vector<double> child(geneLength);
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            
            for (int i = 0; i < geneLength; ++i) {
                if (dist(rng) < crossoverRate) {
                    child[i] = parent1[i];
                } else {
                    child[i] = parent2[i];
                }
            }
            
            return child;
        }
        
        void mutate(std::vector<double>& individual) {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            std::uniform_real_distribution<double> mutation(-1.0, 1.0);
            
            for (int i = 0; i < geneLength; ++i) {
                if (dist(rng) < mutationRate) {
                    individual[i] += mutation(rng);
                }
            }
        }
        
        void evolve(std::function<double(const std::vector<double>&)> fitnessFunction) {
            evaluateFitness(fitnessFunction);
            
            std::vector<std::vector<double>> newPopulation;
            
            // Keep best individual (elitism)
            int bestIdx = 0;
            for (int i = 1; i < populationSize; ++i) {
                if (fitness[i] > fitness[bestIdx]) {
                    bestIdx = i;
                }
            }
            newPopulation.push_back(population[bestIdx]);
            
            // Generate rest of new population
            while (newPopulation.size() < populationSize) {
                std::vector<double> parent1 = selectParent();
                std::vector<double> parent2 = selectParent();
                std::vector<double> child = crossover(parent1, parent2);
                mutate(child);
                newPopulation.push_back(child);
            }
            
            population = newPopulation;
        }
        
        std::vector<double> getBestIndividual() {
            evaluateFitness([](const std::vector<double>& ind) { return 0.0; }); // Ensure fitness is calculated
            
            int bestIdx = 0;
            for (int i = 1; i < populationSize; ++i) {
                if (fitness[i] > fitness[bestIdx]) {
                    bestIdx = i;
                }
            }
            return population[bestIdx];
        }
    };
    
    // Simulated Annealing
    struct SimulatedAnnealing {
        std::vector<double> currentSolution;
        std::vector<double> bestSolution;
        double currentEnergy;
        double bestEnergy;
        double temperature;
        double coolingRate;
        std::mt19937 rng;
        
        SimulatedAnnealing(const std::vector<double>& initialSolution, double initialTemp = 1000.0, double coolRate = 0.95)
            : currentSolution(initialSolution), bestSolution(initialSolution),
              temperature(initialTemp), coolingRate(coolRate),
              rng(std::random_device{}()) {
            
            currentEnergy = 0.0;
            bestEnergy = 0.0;
        }
        
        double calculateEnergy(std::function<double(const std::vector<double>&)> energyFunction) {
            return energyFunction(currentSolution);
        }
        
        std::vector<double> generateNeighbor() {
            std::vector<double> neighbor = currentSolution;
            std::uniform_real_distribution<double> dist(-1.0, 1.0);
            std::uniform_int_distribution<int> indexDist(0, currentSolution.size() - 1);
            
            // Randomly modify one element
            int idx = indexDist(rng);
            neighbor[idx] += dist(rng);
            
            return neighbor;
        }
        
        bool acceptSolution(double newEnergy) {
            if (newEnergy < currentEnergy) {
                return true;
            }
            
            // Accept with probability based on temperature
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            double probability = exp(-(newEnergy - currentEnergy) / temperature);
            return dist(rng) < probability;
        }
        
        void optimize(std::function<double(const std::vector<double>&)> energyFunction, int maxIterations = 1000) {
            currentEnergy = calculateEnergy(energyFunction);
            bestEnergy = currentEnergy;
            bestSolution = currentSolution;
            
            for (int i = 0; i < maxIterations; ++i) {
                std::vector<double> neighbor = generateNeighbor();
                std::vector<double> tempSolution = currentSolution;
                currentSolution = neighbor;
                double newEnergy = calculateEnergy(energyFunction);
                
                if (acceptSolution(newEnergy)) {
                    currentEnergy = newEnergy;
                    
                    if (newEnergy < bestEnergy) {
                        bestEnergy = newEnergy;
                        bestSolution = currentSolution;
                    }
                } else {
                    currentSolution = tempSolution; // Revert
                }
                
                temperature *= coolingRate;
            }
        }
        
        std::vector<double> getBestSolution() const {
            return bestSolution;
        }
        
        double getBestEnergy() const {
            return bestEnergy;
        }
    };
    
    // Gradient Descent
    struct GradientDescent {
        std::vector<double> parameters;
        double learningRate;
        int maxIterations;
        double tolerance;
        
        GradientDescent(const std::vector<double>& initialParams, double lr = 0.01, int maxIter = 1000, double tol = 1e-6)
            : parameters(initialParams), learningRate(lr), maxIterations(maxIter), tolerance(tol) {}
        
        std::vector<double> calculateGradient(std::function<double(const std::vector<double>&)> costFunction) {
            std::vector<double> gradient(parameters.size());
            const double h = 1e-6;
            
            for (size_t i = 0; i < parameters.size(); ++i) {
                std::vector<double> paramsPlus = parameters;
                std::vector<double> paramsMinus = parameters;
                
                paramsPlus[i] += h;
                paramsMinus[i] -= h;
                
                double costPlus = costFunction(paramsPlus);
                double costMinus = costFunction(paramsMinus);
                
                gradient[i] = (costPlus - costMinus) / (2 * h);
            }
            
            return gradient;
        }
        
        void optimize(std::function<double(const std::vector<double>&)> costFunction) {
            for (int iter = 0; iter < maxIterations; ++iter) {
                std::vector<double> gradient = calculateGradient(costFunction);
                
                double gradientNorm = 0.0;
                for (double g : gradient) {
                    gradientNorm += g * g;
                }
                gradientNorm = sqrt(gradientNorm);
                
                if (gradientNorm < tolerance) break;
                
                // Update parameters
                for (size_t i = 0; i < parameters.size(); ++i) {
                    parameters[i] -= learningRate * gradient[i];
                }
            }
        }
        
        std::vector<double> getParameters() const {
            return parameters;
        }
    };
    
    OptimizationEngine() {}
    
    GeneticAlgorithm* createGeneticAlgorithm(int populationSize, int geneLength, double mutationRate, double crossoverRate) {
        return new GeneticAlgorithm(populationSize, geneLength, mutationRate, crossoverRate);
    }
    
    SimulatedAnnealing* createSimulatedAnnealing(const std::vector<double>& initialSolution, double temperature, double coolingRate) {
        return new SimulatedAnnealing(initialSolution, temperature, coolingRate);
    }
    
    GradientDescent* createGradientDescent(const std::vector<double>& initialParams, double learningRate, int maxIterations, double tolerance) {
        return new GradientDescent(initialParams, learningRate, maxIterations, tolerance);
    }
};

// ============================================================
// Register optimization natives
// ============================================================
void registerNativeOptimization(const std::shared_ptr<Environment>& globals) {

    static OptimizationEngine optEngine;
    
    // Optimization object with methods
    auto opt = std::make_shared<ClawHashMap>();
    
    // Create genetic algorithm
    opt->set("createGA", callableValue(std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            int populationSize = static_cast<int>(asNumber(args[0]));
            int geneLength = static_cast<int>(asNumber(args[1]));
            double mutationRate = asNumber(args[2]);
            double crossoverRate = asNumber(args[3]);
            
            auto ga = optEngine.createGeneticAlgorithm(populationSize, geneLength, mutationRate, crossoverRate);
            auto gaMap = std::make_shared<ClawHashMap>();
            gaMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(ga)));
            gaMap->set("populationSize", numberToValue(populationSize));
            gaMap->set("geneLength", numberToValue(geneLength));
            
            return hashMapValue(gaMap);
        },
        "opt.createGA"
    )));
    
    // Create simulated annealing
    opt->set("createSA", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            auto solutionArray = asArray(args[0]);
            std::vector<double> solution;
            for (size_t i = 0; i < solutionArray->size(); ++i) {
                solution.push_back(asNumber(solutionArray->get(i)));
            }
            
            double temperature = asNumber(args[1]);
            double coolingRate = asNumber(args[2]);
            
            auto sa = optEngine.createSimulatedAnnealing(solution, temperature, coolingRate);
            auto saMap = std::make_shared<ClawHashMap>();
            saMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(sa)));
            saMap->set("temperature", numberToValue(temperature));
            saMap->set("coolingRate", numberToValue(coolingRate));
            
            return hashMapValue(saMap);
        },
        "opt.createSA"
    )));
    
    // Create gradient descent
    opt->set("createGD", callableValue(std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            auto paramsArray = asArray(args[0]);
            std::vector<double> params;
            for (size_t i = 0; i < paramsArray->size(); ++i) {
                params.push_back(asNumber(paramsArray->get(i)));
            }
            
            double learningRate = asNumber(args[1]);
            int maxIterations = static_cast<int>(asNumber(args[2]));
            double tolerance = asNumber(args[3]);
            
            auto gd = optEngine.createGradientDescent(params, learningRate, maxIterations, tolerance);
            auto gdMap = std::make_shared<ClawHashMap>();
            gdMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(gd)));
            gdMap->set("learningRate", numberToValue(learningRate));
            gdMap->set("maxIterations", numberToValue(maxIterations));
            
            return hashMapValue(gdMap);
        },
        "opt.createGD"
    )));
    
    globals->define("opt", hashMapValue(opt));
    
    // Convenience functions
    globals->define("createGeneticAlgorithm", callableValue(std::make_shared<NativeFunction>(
        4,
        [opt](const std::vector<Value>& args) -> Value {
            return opt->get("createGA")->call(*nullptr, args);
        },
        "createGeneticAlgorithm"
    )));
    
    globals->define("createSimulatedAnnealing", callableValue(std::make_shared<NativeFunction>(
        3,
        [opt](const std::vector<Value>& args) -> Value {
            return opt->get("createSA")->call(*nullptr, args);
        },
        "createSimulatedAnnealing"
    )));
}

} // namespace claw
