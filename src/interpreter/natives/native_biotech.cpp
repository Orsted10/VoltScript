#include "native_biotech.h"
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

namespace claw {

// ============================================================
// Biotechnology Implementation
// ============================================================
struct BiotechLab {
    struct DNASequence {
        std::string sequence;
        std::string name;
        std::string species;
        
        DNASequence(const std::string& seq, const std::string& n, const std::string& sp) 
            : sequence(seq), name(n), species(sp) {}
        
        int countBases() const {
            return sequence.length();
        }
        
        std::unordered_map<char, int> getBaseComposition() const {
            std::unordered_map<char, int> composition;
            for (char base : sequence) {
                composition[base]++;
            }
            return composition;
        }
        
        std::string getComplement() const {
            std::string complement;
            for (char base : sequence) {
                switch (base) {
                    case 'A': complement += 'T'; break;
                    case 'T': complement += 'A'; break;
                    case 'C': complement += 'G'; break;
                    case 'G': complement += 'C'; break;
                    default: complement += 'N'; break;
                }
            }
            return complement;
        }
        
        std::string getReverseComplement() const {
            std::string complement = getComplement();
            std::reverse(complement.begin(), complement.end());
            return complement;
        }
    };
    
    struct Protein {
        std::string name;
        std::string sequence;
        std::string structure; // "primary", "secondary", "tertiary", "quaternary"
        double molecularWeight;
        int length;
        
        Protein(const std::string& n, const std::string& seq) 
            : name(n), sequence(seq), structure("primary"), length(seq.length()) {
            molecularWeight = calculateMolecularWeight();
        }
        
        double calculateMolecularWeight() const {
            // Simplified molecular weight calculation
            std::unordered_map<char, double> aaWeights = {
                {'A', 89.09}, {'R', 174.20}, {'N', 132.12}, {'D', 133.10},
                {'C', 121.16}, {'E', 147.13}, {'Q', 146.15}, {'G', 75.07},
                {'H', 155.16}, {'I', 131.17}, {'L', 131.17}, {'K', 146.19},
                {'M', 149.21}, {'F', 165.19}, {'P', 115.13}, {'S', 105.09},
                {'T', 119.12}, {'W', 204.23}, {'Y', 181.19}, {'V', 117.15}
            };
            
            double weight = 0.0;
            for (char aa : sequence) {
                if (aaWeights.find(aa) != aaWeights.end()) {
                    weight += aaWeights[aa];
                }
            }
            return weight;
        }
        
        std::string getHydrophobicity() const {
            // Simplified hydrophobicity analysis
            std::unordered_map<char, std::string> hydrophobicity = {
                {'A', "hydrophobic"}, {'V', "hydrophobic"}, {'L', "hydrophobic"},
                {'I', "hydrophobic"}, {'M', "hydrophobic"}, {'F', "hydrophobic"},
                {'W', "hydrophobic"}, {'P', "hydrophobic"}, {'G', "neutral"},
                {'S', "hydrophilic"}, {'T', "hydrophilic"}, {'C', "hydrophilic"},
                {'Y', "hydrophilic"}, {'N', "hydrophilic"}, {'Q', "hydrophilic"}
            };
            
            int hydrophobic = 0, hydrophilic = 0;
            for (char aa : sequence) {
                if (hydrophobicity[aa] == "hydrophobic") hydrophobic++;
                else if (hydrophobicity[aa] == "hydrophilic") hydrophilic++;
            }
            
            if (hydrophobic > hydrophilic) return "hydrophobic";
            else if (hydrophilic > hydrophobic) return "hydrophilic";
            else return "neutral";
        }
    };
    
    struct Cell {
        std::string type;
        std::string name;
        double size; // micrometers
        bool isEukaryotic;
        bool isProkaryotic;
        std::vector<std::string> organelles;
        std::vector<DNASequence> chromosomes;
        
        Cell(const std::string& t, const std::string& n, double s) 
            : type(t), name(n), size(s), isEukaryotic(false), isProkaryotic(false) {
            
            if (t == "animal" || t == "plant" || t == "fungi") {
                isEukaryotic = true;
                organelles = {"nucleus", "mitochondria", "ribosomes", "endoplasmic_reticulum"};
            } else if (t == "bacteria") {
                isProkaryotic = true;
                organelles = {"ribosomes", "cell_wall", "plasma_membrane"};
            }
        }
        
        bool divide() {
            // Simulate cell division
            return true;
        }
        
        bool mutate(double mutationRate) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);
            
            if (dis(gen) < mutationRate) {
                return true; // Mutation occurred
            }
            return false;
        }
    };
    
    std::vector<DNASequence> dnaDatabase;
    std::vector<Protein> proteinDatabase;
    std::vector<Cell> cellDatabase;
    
    BiotechLab() {
        // Initialize with sample data
        dnaDatabase.emplace_back("ATCGATCGATCGATCG", "Sample DNA", "Homo sapiens");
        dnaDatabase.emplace_back("GCTAGCTAGCTAGCTA", "Sample RNA", "Mus musculus");
        
        proteinDatabase.emplace_back("Hemoglobin", "MVHLTPEEKSAVTALWGKVNVDEVGGEALGRLLVVYPWTQRFFESFGDLSTPDAVMGNPKVKAHGKKVLGAFSDGLAHLDNLKGTFATLSELHCDKLHVDPENFRLLGNVLVCVLAHHFGKEFTPPVQAAYQKVVAGVANALAHKYH");
        proteinDatabase.emplace_back("Insulin", "GIVEQCCTSICSLYQLENYCN");
        
        cellDatabase.emplace_back("animal", "Neuron", 0.02);
        cellDatabase.emplace_back("animal", "Muscle Cell", 0.05);
        cellDatabase.emplace_back("bacteria", "E. coli", 0.002);
    }
    
    DNASequence* createDNASequence(const std::string& sequence, const std::string& name, const std::string& species) {
        dnaDatabase.emplace_back(sequence, name, species);
        return &dnaDatabase.back();
    }
    
    Protein* createProtein(const std::string& name, const std::string& sequence) {
        proteinDatabase.emplace_back(name, sequence);
        return &proteinDatabase.back();
    }
    
    Cell* createCell(const std::string& type, const std::string& name, double size) {
        cellDatabase.emplace_back(type, name, size);
        return &cellDatabase.back();
    }
    
    std::string translateDNA(const std::string& dna) {
        // Simplified DNA to protein translation
        std::unordered_map<std::string, char> codonTable = {
            {"ATG", 'M'}, {"TTT", 'F'}, {"TTC", 'F'}, {"TTA", 'L'}, {"TTG", 'L'},
            {"CTT", 'L'}, {"CTC", 'L'}, {"CTA", 'L'}, {"CTG", 'L'}, {"ATT", 'I'},
            {"ATC", 'I'}, {"ATA", 'I'}, {"GTT", 'V'}, {"GTC", 'V'}, {"GTA", 'V'},
            {"GTG", 'V'}, {"TCT", 'S'}, {"TCC", 'S'}, {"TCA", 'S'}, {"TCG", 'S'},
            {"CCT", 'P'}, {"CCC", 'P'}, {"CCA", 'P'}, {"CCG", 'P'}, {"ACT", 'T'},
            {"ACC", 'T'}, {"ACA", 'T'}, {"ACG", 'T'}, {"GCT", 'A'}, {"GCC", 'A'},
            {"GCA", 'A'}, {"GCG", 'A'}, {"TAT", 'Y'}, {"TAC", 'Y'}, {"TAA", '*'},
            {"TAG", '*'}, {"CAT", 'H'}, {"CAC", 'H'}, {"CAA", 'Q'}, {"CAG", 'Q'},
            {"AAT", 'N'}, {"AAC", 'N'}, {"AAA", 'K'}, {"AAG", 'K'}, {"GAT", 'D'},
            {"GAC", 'D'}, {"GAA", 'E'}, {"GAG", 'E'}, {"TGT", 'C'}, {"TGC", 'C'},
            {"TGA", '*'}, {"TGG", 'W'}, {"CGT", 'R'}, {"CGC", 'R'}, {"CGA", 'R'},
            {"CGG", 'R'}, {"AGT", 'S'}, {"AGC", 'S'}, {"AGA", 'R'}, {"AGG", 'R'},
            {"GGT", 'G'}, {"GGC", 'G'}, {"GGA", 'G'}, {"GGG", 'G'}
        };
        
        std::string protein;
        for (size_t i = 0; i < dna.length() - 2; i += 3) {
            std::string codon = dna.substr(i, 3);
            if (codonTable.find(codon) != codonTable.end()) {
                protein += codonTable[codon];
            }
        }
        return protein;
    }
    
    bool performPCR(const std::string& dna, int cycles) {
        // Simulate PCR amplification
        return cycles > 0;
    }
    
    bool performSequencing(const std::string& sample) {
        // Simulate DNA sequencing
        return true;
    }
    
    std::vector<std::string> getGenomeAnalysis(const std::string& species) {
        return {"chromosome_1", "chromosome_2", "chromosome_3", "mitochondria"};
    }
};

// ============================================================
// Register biotech natives
// ============================================================
void registerNativeBiotech(const std::shared_ptr<Environment>& globals) {

    static BiotechLab biotechLab;
    
    // Biotech object with methods
    auto biotech = std::make_shared<ClawHashMap>();
    
    // Create DNA sequence
    biotech->set("createDNA", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            std::string sequence = valueToString(args[0]);
            std::string name = valueToString(args[1]);
            std::string species = valueToString(args[2]);
            
            auto dna = biotechLab.createDNASequence(sequence, name, species);
            auto dnaMap = std::make_shared<ClawHashMap>();
            dnaMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(dna)));
            dnaMap->set("sequence", stringValue(StringPool::intern(sequence).data()));
            dnaMap->set("name", stringValue(StringPool::intern(name).data()));
            dnaMap->set("species", stringValue(StringPool::intern(species).data()));
            dnaMap->set("length", numberToValue(dna->countBases()));
            
            return hashMapValue(dnaMap);
        },
        "biotech.createDNA"
    )));
    
    // Create protein
    biotech->set("createProtein", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            std::string sequence = valueToString(args[1]);
            
            auto protein = biotechLab.createProtein(name, sequence);
            auto proteinMap = std::make_shared<ClawHashMap>();
            proteinMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(protein)));
            proteinMap->set("name", stringValue(StringPool::intern(name).data()));
            proteinMap->set("sequence", stringValue(StringPool::intern(sequence).data()));
            proteinMap->set("molecularWeight", numberToValue(protein->molecularWeight));
            proteinMap->set("length", numberToValue(protein->length));
            
            return hashMapValue(proteinMap);
        },
        "biotech.createProtein"
    )));
    
    // Create cell
    biotech->set("createCell", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            std::string type = valueToString(args[0]);
            std::string name = valueToString(args[1]);
            double size = asNumber(args[2]);
            
            auto cell = biotechLab.createCell(type, name, size);
            auto cellMap = std::make_shared<ClawHashMap>();
            cellMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(cell)));
            cellMap->set("type", stringValue(StringPool::intern(type).data()));
            cellMap->set("name", stringValue(StringPool::intern(name).data()));
            cellMap->set("size", numberToValue(size));
            cellMap->set("isEukaryotic", boolValue(cell->isEukaryotic));
            cellMap->set("isProkaryotic", boolValue(cell->isProkaryotic));
            
            return hashMapValue(cellMap);
        },
        "biotech.createCell"
    )));
    
    // Translate DNA to protein
    biotech->set("translateDNA", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string dna = valueToString(args[0]);
            std::string protein = biotechLab.translateDNA(dna);
            return stringValue(StringPool::intern(protein).data());
        },
        "biotech.translateDNA"
    )));
    
    // Perform PCR
    biotech->set("performPCR", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string dna = valueToString(args[0]);
            int cycles = static_cast<int>(asNumber(args[1]));
            bool success = biotechLab.performPCR(dna, cycles);
            return boolValue(success);
        },
        "biotech.performPCR"
    )));
    
    globals->define("biotech", hashMapValue(biotech));
    
    // Convenience functions
    globals->define("createDNA", callableValue(std::make_shared<NativeFunction>(
        3,
        [biotech](const std::vector<Value>& args) -> Value {
            return biotech->get("createDNA")->call(*nullptr, args);
        },
        "createDNA"
    )));
    
    globals->define("createProtein", callableValue(std::make_shared<NativeFunction>(
        2,
        [biotech](const std::vector<Value>& args) -> Value {
            return biotech->get("createProtein")->call(*nullptr, args);
        },
        "createProtein"
    )));
}

} // namespace claw
