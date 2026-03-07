#include "native_medical.h"
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

namespace claw {

// ============================================================
// Medical System Implementation
// ============================================================
struct MedicalSystem {
    struct Patient {
        std::string name;
        int age;
        std::string gender;
        double height; // cm
        double weight; // kg
        std::vector<std::string> symptoms;
        std::vector<std::string> conditions;
        std::vector<std::string> medications;
        std::unordered_map<std::string, double> vitals;
        
        Patient(const std::string& n, int a, const std::string& g, double h, double w)
            : name(n), age(a), gender(g), height(h), weight(w) {
            
            // Initialize vitals with normal ranges
            vitals["heart_rate"] = 72.0; // bpm
            vitals["blood_pressure_systolic"] = 120.0; // mmHg
            vitals["blood_pressure_diastolic"] = 80.0; // mmHg
            vitals["temperature"] = 36.6; // Celsius
            vitals["oxygen_saturation"] = 98.0; // percentage
            vitals["respiratory_rate"] = 16.0; // breaths per minute
        }
        
        double calculateBMI() const {
            double heightM = height / 100.0; // Convert to meters
            return weight / (heightM * heightM);
        }
        
        std::string getBMICategory() const {
            double bmi = calculateBMI();
            if (bmi < 18.5) return "underweight";
            if (bmi < 25.0) return "normal";
            if (bmi < 30.0) return "overweight";
            return "obese";
        }
        
        void addSymptom(const std::string& symptom) {
            symptoms.push_back(symptom);
        }
        
        void addCondition(const std::string& condition) {
            conditions.push_back(condition);
        }
        
        void addMedication(const std::string& medication) {
            medications.push_back(medication);
        }
        
        void updateVital(const std::string& vital, double value) {
            vitals[vital] = value;
        }
        
        std::vector<std::string> getRiskFactors() const {
            std::vector<std::string> risks;
            
            double bmi = calculateBMI();
            if (bmi >= 30.0) risks.push_back("obesity");
            if (age >= 65) risks.push_back("advanced_age");
            if (vitals.at("blood_pressure_systolic") >= 140) risks.push_back("hypertension");
            if (vitals.at("temperature") >= 38.0) risks.push_back("fever");
            
            return risks;
        }
    };
    
    struct Medication {
        std::string name;
        std::string category; // "antibiotic", "painkiller", "anti-inflammatory", etc.
        std::vector<std::string> uses;
        std::vector<std::string> sideEffects;
        double dosage; // mg
        std::string frequency; // "daily", "twice_daily", "as_needed"
        
        Medication(const std::string& n, const std::string& cat, double dose, const std::string& freq)
            : name(n), category(cat), dosage(dose), frequency(freq) {}
        
        void addUse(const std::string& use) {
            uses.push_back(use);
        }
        
        void addSideEffect(const std::string& sideEffect) {
            sideEffects.push_back(sideEffect);
        }
    };
    
    struct Diagnosis {
        std::string name;
        std::string icd10Code; // International Classification of Diseases
        std::vector<std::string> symptoms;
        std::vector<std::string> treatments;
        double prevalence; // per 100,000 population
        std::string severity; // "mild", "moderate", "severe", "critical"
        
        Diagnosis(const std::string& n, const std::string& code, double prev, const std::string& sev)
            : name(n), icd10Code(code), prevalence(prev), severity(sev) {}
        
        void addSymptom(const std::string& symptom) {
            symptoms.push_back(symptom);
        }
        
        void addTreatment(const std::string& treatment) {
            treatments.push_back(treatment);
        }
    };
    
    struct MedicalDevice {
        std::string name;
        std::string type; // "monitor", "imaging", "therapeutic", "diagnostic"
        bool isAvailable;
        double accuracy; // percentage
        std::vector<std::string> capabilities;
        
        MedicalDevice(const std::string& n, const std::string& t, double acc)
            : name(n), type(t), isAvailable(true), accuracy(acc) {}
        
        void addCapability(const std::string& capability) {
            capabilities.push_back(capability);
        }
        
        bool performTest(const std::string& test) {
            if (!isAvailable) return false;
            
            // Simulate test accuracy
            return (rand() % 100) < accuracy;
        }
    };
    
    std::vector<Patient> patients;
    std::vector<Medication> medications;
    std::vector<Diagnosis> diagnoses;
    std::vector<MedicalDevice> devices;
    
    MedicalSystem() {
        // Initialize sample medications
        Medication aspirin("Aspirin", "painkiller", 325.0, "as_needed");
        aspirin.addUse("pain_relief");
        aspirin.addUse("fever_reduction");
        aspirin.addUse("blood_thinning");
        aspirin.addSideEffect("stomach_irritation");
        aspirin.addSideEffect("bleeding_risk");
        medications.push_back(aspirin);
        
        Medication amoxicillin("Amoxicillin", "antibiotic", 500.0, "twice_daily");
        amoxicillin.addUse("bacterial_infections");
        amoxicillin.addSideEffect("allergic_reaction");
        amoxicillin.addSideEffect("diarrhea");
        medications.push_back(amoxicillin);
        
        // Initialize sample diagnoses
        Diagnosis flu("Influenza", "J11.1", 10000.0, "moderate");
        flu.addSymptom("fever");
        flu.addSymptom("cough");
        flu.addSymptom("body_aches");
        flu.addSymptom("fatigue");
        flu.addTreatment("rest");
        flu.addTreatment("fluids");
        flu.addTreatment("antipyretics");
        diagnoses.push_back(flu);
        
        Diagnosis hypertension("Hypertension", "I10", 30000.0, "moderate");
        hypertension.addSymptom("high_blood_pressure");
        hypertension.addSymptom("headaches");
        hypertension.addTreatment("lifestyle_changes");
        hypertension.addTreatment("antihypertensives");
        diagnoses.push_back(hypertension);
        
        // Initialize medical devices
        MedicalDevice ecg("ECG Machine", "monitor", 95.0);
        ecg.addCapability("heart_rhythm_monitoring");
        ecg.addCapability("electrocardiogram");
        devices.push_back(ecg);
        
        MedicalDevice mri("MRI Scanner", "imaging", 98.0);
        mri.addCapability("soft_tissue_imaging");
        mri.addCapability("brain_scanning");
        mri.addCapability("joint_imaging");
        devices.push_back(mri);
    }
    
    Patient* addPatient(const std::string& name, int age, const std::string& gender, 
                       double height, double weight) {
        patients.emplace_back(name, age, gender, height, weight);
        return &patients.back();
    }
    
    Medication* addMedication(const std::string& name, const std::string& category, 
                             double dosage, const std::string& frequency) {
        medications.emplace_back(name, category, dosage, frequency);
        return &medications.back();
    }
    
    Diagnosis* addDiagnosis(const std::string& name, const std::string& icd10Code, 
                           double prevalence, const std::string& severity) {
        diagnoses.emplace_back(name, icd10Code, prevalence, severity);
        return &diagnoses.back();
    }
    
    MedicalDevice* addMedicalDevice(const std::string& name, const std::string& type, 
                                   double accuracy) {
        devices.emplace_back(name, type, accuracy);
        return &devices.back();
    }
    
    std::vector<Diagnosis> diagnosePatient(const Patient& patient) const {
        std::vector<Diagnosis> possibleDiagnoses;
        
        for (const auto& diagnosis : diagnoses) {
            int matchingSymptoms = 0;
            
            for (const auto& symptom : patient.symptoms) {
                for (const auto& diagSymptom : diagnosis.symptoms) {
                    if (symptom == diagSymptom) {
                        matchingSymptoms++;
                        break;
                    }
                }
            }
            
            // If at least 50% of symptoms match, consider it a possible diagnosis
            if (matchingSymptoms >= diagnosis.symptoms.size() / 2) {
                possibleDiagnoses.push_back(diagnosis);
            }
        }
        
        return possibleDiagnoses;
    }
    
    std::vector<Medication> prescribeMedications(const Diagnosis& diagnosis) const {
        std::vector<Medication> prescribed;
        
        for (const auto& medication : medications) {
            for (const auto& use : medication.uses) {
                // Simplified prescription logic
                if (diagnosis.name == "Influenza" && use == "fever_reduction") {
                    prescribed.push_back(medication);
                    break;
                }
                if (diagnosis.name == "Hypertension" && medication.category == "antihypertensives") {
                    prescribed.push_back(medication);
                    break;
                }
            }
        }
        
        return prescribed;
    }
    
    bool performMedicalTest(const std::string& deviceName, const std::string& test) {
        for (auto& device : devices) {
            if (device.name == deviceName) {
                return device.performTest(test);
            }
        }
        return false;
    }
    
    std::vector<std::string> getEmergencyProtocols(const std::string& condition) const {
        if (condition == "cardiac_arrest") {
            return {"call_911", "start_CPR", "use_AED", "administer_epinephrine"};
        } else if (condition == "stroke") {
            return {"call_911", "note_time", "FAST_assessment", "prepare_tPA"};
        } else if (condition == "severe_bleeding") {
            return {"apply_pressure", "elevate_limb", "use_tourniquet", "call_911"};
        }
        return {"assess_patient", "provide_basic_care", "call_911"};
    }
    
    double calculateHealthScore(const Patient& patient) const {
        double score = 100.0; // Start with perfect health
        
        // Age factor
        if (patient.age > 65) score -= 10;
        if (patient.age > 80) score -= 10;
        
        // BMI factor
        double bmi = patient.calculateBMI();
        if (bmi < 18.5 || bmi >= 30) score -= 15;
        
        // Vitals factor
        if (patient.vitals.at("heart_rate") < 60 || patient.vitals.at("heart_rate") > 100) score -= 10;
        if (patient.vitals.at("blood_pressure_systolic") >= 140) score -= 10;
        if (patient.vitals.at("temperature") >= 38.0) score -= 5;
        
        // Conditions factor
        score -= patient.conditions.size() * 5;
        
        return std::max(0.0, score);
    }
};

// ============================================================
// Register medical natives
// ============================================================
void registerNativeMedical(const std::shared_ptr<Environment>& globals) {

    static MedicalSystem medicalSystem;
    
    // Medical object with methods
    auto medical = std::make_shared<ClawHashMap>();
    
    // Add patient
    medical->set("addPatient", callableValue(std::make_shared<NativeFunction>(
        5,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            int age = static_cast<int>(asNumber(args[1]));
            std::string gender = valueToString(args[2]);
            double height = asNumber(args[3]);
            double weight = asNumber(args[4]);
            
            auto patient = medicalSystem.addPatient(name, age, gender, height, weight);
            auto patientMap = std::make_shared<ClawHashMap>();
            patientMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(patient)));
            patientMap->set("name", stringValue(StringPool::intern(name).data()));
            patientMap->set("age", numberToValue(age));
            patientMap->set("gender", stringValue(StringPool::intern(gender).data()));
            patientMap->set("bmi", numberToValue(patient->calculateBMI()));
            patientMap->set("bmiCategory", stringValue(StringPool::intern(patient->getBMICategory()).data()));
            
            return hashMapValue(patientMap);
        },
        "medical.addPatient"
    )));
    
    // Add medication
    medical->set("addMedication", callableValue(std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            std::string category = valueToString(args[1]);
            double dosage = asNumber(args[2]);
            std::string frequency = valueToString(args[3]);
            
            auto medication = medicalSystem.addMedication(name, category, dosage, frequency);
            auto medMap = std::make_shared<ClawHashMap>();
            medMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(medication)));
            medMap->set("name", stringValue(StringPool::intern(name).data()));
            medMap->set("category", stringValue(StringPool::intern(category).data()));
            medMap->set("dosage", numberToValue(dosage));
            medMap->set("frequency", stringValue(StringPool::intern(frequency).data()));
            
            return hashMapValue(medMap);
        },
        "medical.addMedication"
    )));
    
    // Diagnose patient
    medical->set("diagnose", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto patientMap = asHashMap(args[0]);
            
            auto ptrValue = patientMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto patient = static_cast<MedicalSystem::Patient*>(ptr);
            
            auto diagnoses = medicalSystem.diagnosePatient(*patient);
            auto diagnosisArray = std::make_shared<ClawArray>();
            
            for (const auto& diagnosis : diagnoses) {
                auto diagMap = std::make_shared<ClawHashMap>();
                diagMap->set("name", stringValue(StringPool::intern(diagnosis.name).data()));
                diagMap->set("severity", stringValue(StringPool::intern(diagnosis.severity).data()));
                diagMap->set("prevalence", numberToValue(diagnosis.prevalence));
                diagnosisArray->push(hashMapValue(diagMap));
            }
            
            return arrayValue(diagnosisArray);
        },
        "medical.diagnose"
    )));
    
    // Calculate health score
    medical->set("healthScore", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto patientMap = asHashMap(args[0]);
            
            auto ptrValue = patientMap->get("_ptr");
            void* ptr = reinterpret_cast<void*>(static_cast<uintptr_t>(asNumber(ptrValue)));
            auto patient = static_cast<MedicalSystem::Patient*>(ptr);
            
            double score = medicalSystem.calculateHealthScore(*patient);
            return numberToValue(score);
        },
        "medical.healthScore"
    )));
    
    globals->define("medical", hashMapValue(medical));
    
    // Convenience functions
    globals->define("addPatient", callableValue(std::make_shared<NativeFunction>(
        5,
        [medical](const std::vector<Value>& args) -> Value {
            return medical->get("addPatient")->call(*nullptr, args);
        },
        "addPatient"
    )));
    
    globals->define("diagnosePatient", callableValue(std::make_shared<NativeFunction>(
        1,
        [medical](const std::vector<Value>& args) -> Value {
            return medical->get("diagnose")->call(*nullptr, args);
        },
        "diagnosePatient"
    )));
}

} // namespace claw
