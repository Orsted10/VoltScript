#include "native_audio.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

namespace claw {

// ============================================================
// Audio Implementation
// ============================================================
struct AudioEngine {
    bool isInitialized = false;
    double volume = 1.0;
    
    bool initialize() {
        isInitialized = true;
        return true;
    }
    
    void playSound(const std::string& soundId) {
        // Mock audio playback
        // In real implementation, would play actual audio
    }
    
    void playMusic(const std::string& musicId, bool loop = false) {
        // Mock music playback
        // In real implementation, would play actual music
    }
    
    void setVolume(double vol) {
        volume = std::max(0.0, std::min(1.0, vol));
    }
    
    void stop() {
        // Mock stop all audio
    }
    
    void pause() {
        // Mock pause audio
    }
    
    void resume() {
        // Mock resume audio
    }
};

// ============================================================
// Register audio natives
// ============================================================
void registerNativeAudio(const std::shared_ptr<Environment>& globals) {

    static AudioEngine audioEngine;
    
    // Audio object with methods
    auto audio = std::make_shared<ClawHashMap>();
    
    // Initialize audio
    audio->set("init", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            bool success = audioEngine.initialize();
            return boolValue(success);
        },
        "audio.init"
    )));
    
    // Play sound
    audio->set("playSound", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string soundId = valueToString(args[0]);
            audioEngine.playSound(soundId);
            return boolValue(true);
        },
        "audio.playSound"
    )));
    
    // Play music
    audio->set("playMusic", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string musicId = valueToString(args[0]);
            bool loop = args.size() > 1 ? isBool(args[1]) ? asBool(args[1]) : false : false;
            audioEngine.playMusic(musicId, loop);
            return boolValue(true);
        },
        "audio.playMusic"
    )));
    
    // Set volume
    audio->set("setVolume", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            double volume = asNumber(args[0]);
            audioEngine.setVolume(volume);
            return boolValue(true);
        },
        "audio.setVolume"
    )));
    
    // Stop audio
    audio->set("stop", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            audioEngine.stop();
            return boolValue(true);
        },
        "audio.stop"
    )));
    
    // Pause audio
    audio->set("pause", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            audioEngine.pause();
            return boolValue(true);
        },
        "audio.pause"
    )));
    
    // Resume audio
    audio->set("resume", callableValue(std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>& args) -> Value {
            audioEngine.resume();
            return boolValue(true);
        },
        "audio.resume"
    )));
    
    globals->define("audio", hashMapValue(audio));
    
    // Convenience functions
    globals->define("playSound", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            std::string soundId = valueToString(args[0]);
            audioEngine.playSound(soundId);
            return boolValue(true);
        },
        "playSound"
    )));
    
    globals->define("playMusic", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            std::string musicId = valueToString(args[0]);
            bool loop = args.size() > 1 ? isBool(args[1]) ? asBool(args[1]) : false : false;
            audioEngine.playMusic(musicId, loop);
            return boolValue(true);
        },
        "playMusic"
    )));
}

} // namespace claw
