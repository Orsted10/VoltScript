#include "native_climate.h"
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
// Climate Modeling Implementation
// ============================================================
struct ClimateModel {
    struct WeatherStation {
        std::string name;
        double latitude;
        double longitude;
        double temperature; // Celsius
        double humidity; // percentage
        double pressure; // hPa
        double windSpeed; // km/h
        double windDirection; // degrees
        double precipitation; // mm/hour
        std::string condition; // "sunny", "cloudy", "rainy", "stormy"
        
        WeatherStation(const std::string& n, double lat, double lon)
            : name(n), latitude(lat), longitude(lon), temperature(20.0), humidity(50.0),
              pressure(1013.25), windSpeed(10.0), windDirection(0.0), precipitation(0.0),
              condition("sunny") {}
        
        void updateWeather(double temp, double humid, double press, double wind, double dir, double precip, const std::string& cond) {
            temperature = temp;
            humidity = humid;
            pressure = press;
            windSpeed = wind;
            windDirection = dir;
            precipitation = precip;
            condition = cond;
        }
        
        double getHeatIndex() const {
            // Simplified heat index calculation
            double t = temperature;
            double h = humidity;
            if (t < 27) return t;
            
            double hi = -42.379 + 2.04901523 * t + 10.14333127 * h
                      - 0.22475541 * t * h - 6.83783e-3 * t * t
                      - 5.481717e-2 * h * h + 1.22874e-3 * t * t * h
                      + 8.5282e-4 * t * h * h - 1.99e-6 * t * t * h * h;
            return hi;
        }
        
        double getWindChill() const {
            // Simplified wind chill calculation
            double t = temperature;
            double v = windSpeed;
            if (t > 10 || v < 4.8) return t;
            
            double wc = 13.12 + 0.6215 * t - 11.37 * pow(v, 0.16) + 0.3965 * t * pow(v, 0.16);
            return wc;
        }
    };
    
    struct ClimateZone {
        std::string name;
        std::string type; // "tropical", "temperate", "arctic", "desert", "mediterranean"
        double avgTemperature;
        double avgPrecipitation;
        double co2Level; // ppm
        double seaLevel; // meters above sea level
        
        ClimateZone(const std::string& n, const std::string& t, double temp, double precip)
            : name(n), type(t), avgTemperature(temp), avgPrecipitation(precip), co2Level(415.0), seaLevel(0.0) {}
        
        void updateCO2(double level) {
            co2Level = level;
        }
        
        double getTemperatureAnomaly(double baseline) const {
            return avgTemperature - baseline;
        }
        
        std::string getClimateStatus() const {
            if (co2Level > 450) return "critical";
            if (co2Level > 400) return "concerning";
            if (co2Level > 350) return "moderate";
            return "stable";
        }
    };
    
    struct CarbonFootprint {
        std::string source;
        double emissions; // kg CO2 per year
        std::string category; // "transport", "energy", "industry", "agriculture"
        
        CarbonFootprint(const std::string& src, double emis, const std::string& cat)
            : source(src), emissions(emis), category(cat) {}
        
        double getEquivalentTrees() const {
            // One tree absorbs ~22 kg CO2 per year
            return emissions / 22.0;
        }
        
        double getCarKilometers() const {
            // Average car emits ~0.12 kg CO2 per km
            return emissions / 0.12;
        }
    };
    
    std::vector<WeatherStation> weatherStations;
    std::vector<ClimateZone> climateZones;
    std::vector<CarbonFootprint> carbonFootprints;
    double globalTemperature = 15.0; // Global average temperature
    double seaLevelRise = 0.0; // mm per year
    
    ClimateModel() {
        // Initialize weather stations
        weatherStations.emplace_back("New York", 40.7128, -74.0060);
        weatherStations.emplace_back("London", 51.5074, -0.1278);
        weatherStations.emplace_back("Tokyo", 35.6762, 139.6503);
        weatherStations.emplace_back("Sydney", -33.8688, 151.2093);
        weatherStations.emplace_back("Mumbai", 19.0760, 72.8777);
        
        // Initialize climate zones
        climateZones.emplace_back("Amazon Rainforest", "tropical", 26.0, 2300.0);
        climateZones.emplace_back("Sahara Desert", "desert", 30.0, 76.0);
        climateZones.emplace_back("Siberia", "arctic", -5.0, 200.0);
        climateZones.emplace_back("Mediterranean", "mediterranean", 18.0, 600.0);
        climateZones.emplace_back("Temperate Forest", "temperate", 12.0, 1000.0);
        
        // Initialize carbon footprints
        carbonFootprints.emplace_back("Global Transport", 8.2e12, "transport");
        carbonFootprints.emplace_back("Energy Production", 1.5e13, "energy");
        carbonFootprints.emplace_back("Industrial Processes", 6.5e12, "industry");
        carbonFootprints.emplace_back("Agriculture", 5.8e12, "agriculture");
    }
    
    WeatherStation* addWeatherStation(const std::string& name, double latitude, double longitude) {
        weatherStations.emplace_back(name, latitude, longitude);
        return &weatherStations.back();
    }
    
    ClimateZone* addClimateZone(const std::string& name, const std::string& type, 
                               double avgTemp, double avgPrecip) {
        climateZones.emplace_back(name, type, avgTemp, avgPrecip);
        return &climateZones.back();
    }
    
    CarbonFootprint* addCarbonFootprint(const std::string& source, double emissions, 
                                       const std::string& category) {
        carbonFootprints.emplace_back(source, emissions, category);
        return &carbonFootprints.back();
    }
    
    void simulateClimateChange(double years, double co2IncreaseRate) {
        // Simplified climate change simulation
        for (auto& zone : climateZones) {
            zone.updateCO2(zone.co2Level + co2IncreaseRate * years);
            // Temperature increases with CO2 (simplified climate sensitivity)
            zone.avgTemperature += 0.01 * co2IncreaseRate * years;
        }
        
        globalTemperature += 0.01 * co2IncreaseRate * years;
        seaLevelRise += 3.2 * years; // mm per year
    }
    
    std::vector<double> predictWeather(int daysAhead) {
        std::vector<double> forecast;
        
        // Simplified weather prediction
        for (int i = 0; i < daysAhead; ++i) {
            double temp = globalTemperature + sin(i * 0.1) * 5.0;
            forecast.push_back(temp);
        }
        
        return forecast;
    }
    
    double calculateCarbonOffset(double emissions) const {
        // Calculate required carbon offset in trees
        return emissions / 22.0; // trees needed
    }
    
    std::string getClimateAlert() const {
        if (globalTemperature > 16.0) return "Global warming critical";
        if (seaLevelRise > 100.0) return "Sea level rise alarming";
        if (carbonFootprints[0].emissions > 1e13) return "Carbon emissions too high";
        return "Climate stable";
    }
    
    std::vector<std::string> getMitigationStrategies() const {
        return {
            "Reduce fossil fuel consumption",
            "Increase renewable energy adoption",
            "Implement carbon capture technologies",
            "Promote reforestation and afforestation",
            "Improve energy efficiency",
            "Develop sustainable transportation",
            "Support climate-resilient agriculture"
        };
    }
};

// ============================================================
// Register climate natives
// ============================================================
void registerNativeClimate(const std::shared_ptr<Environment>& globals) {

    static ClimateModel climateModel;
    
    // Climate object with methods
    auto climate = std::make_shared<ClawHashMap>();
    
    // Add weather station
    climate->set("addWeatherStation", callableValue(std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            double latitude = asNumber(args[1]);
            double longitude = asNumber(args[2]);
            
            auto station = climateModel.addWeatherStation(name, latitude, longitude);
            auto stationMap = std::make_shared<ClawHashMap>();
            stationMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(station)));
            stationMap->set("name", stringValue(StringPool::intern(name).data()));
            stationMap->set("latitude", numberToValue(latitude));
            stationMap->set("longitude", numberToValue(longitude));
            stationMap->set("temperature", numberToValue(station->temperature));
            stationMap->set("humidity", numberToValue(station->humidity));
            
            return hashMapValue(stationMap);
        },
        "climate.addWeatherStation"
    )));
    
    // Add climate zone
    climate->set("addClimateZone", callableValue(std::make_shared<NativeFunction>(
        4,
        [](const std::vector<Value>& args) -> Value {
            std::string name = valueToString(args[0]);
            std::string type = valueToString(args[1]);
            double avgTemp = asNumber(args[2]);
            double avgPrecip = asNumber(args[3]);
            
            auto zone = climateModel.addClimateZone(name, type, avgTemp, avgPrecip);
            auto zoneMap = std::make_shared<ClawHashMap>();
            zoneMap->set("_ptr", numberToValue(reinterpret_cast<uintptr_t>(zone)));
            zoneMap->set("name", stringValue(StringPool::intern(name).data()));
            zoneMap->set("type", stringValue(StringPool::intern(type).data()));
            zoneMap->set("avgTemperature", numberToValue(avgTemp));
            zoneMap->set("avgPrecipitation", numberToValue(avgPrecip));
            zoneMap->set("co2Level", numberToValue(zone->co2Level));
            
            return hashMapValue(zoneMap);
        },
        "climate.addClimateZone"
    )));
    
    // Simulate climate change
    climate->set("simulateClimateChange", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            double years = asNumber(args[0]);
            double co2IncreaseRate = asNumber(args[1]);
            
            climateModel.simulateClimateChange(years, co2IncreaseRate);
            return boolValue(true);
        },
        "climate.simulateClimateChange"
    )));
    
    // Predict weather
    climate->set("predictWeather", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            int daysAhead = static_cast<int>(asNumber(args[0]));
            
            auto forecast = climateModel.predictWeather(daysAhead);
            auto forecastArray = std::make_shared<ClawArray>();
            
            for (double temp : forecast) {
                forecastArray->push(numberToValue(temp));
            }
            
            return arrayValue(forecastArray);
        },
        "climate.predictWeather"
    )));
    
    // Get climate alert
    climate->set("getAlert", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            std::string alert = climateModel.getClimateAlert();
            return stringValue(StringPool::intern(alert).data());
        },
        "climate.getAlert"
    )));
    
    globals->define("climate", hashMapValue(climate));
    
    // Convenience functions
    globals->define("addWeatherStation", callableValue(std::make_shared<NativeFunction>(
        3,
        [climate](const std::vector<Value>& args) -> Value {
            return climate->get("addWeatherStation")->call(*nullptr, args);
        },
        "addWeatherStation"
    )));
    
    globals->define("predictWeather", callableValue(std::make_shared<NativeFunction>(
        1,
        [climate](const std::vector<Value>& args) -> Value {
            return climate->get("predictWeather")->call(*nullptr, args);
        },
        "predictWeather"
    )));
}

} // namespace claw
