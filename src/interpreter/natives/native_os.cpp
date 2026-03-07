#include "native_os.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "features/array.h"
#include "features/hashmap.h"
#include "features/string_pool.h"
#include "interpreter/value.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>

#ifdef _WIN32
  #include <windows.h>
  #include <process.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
#endif

namespace claw {

static std::string requireString(const Value& v, const char* fn, int idx) {
    if (!isString(v))
        throw std::runtime_error(std::string(fn) + "(): arg " + std::to_string(idx) + " must be a string");
    const char* p = asStringPtr(v);
    return p ? std::string(p) : std::string();
}

static Value makeStr(const std::string& s) {
    auto sv = StringPool::intern(s);
    return stringValue(sv.data());
}

void registerNativeOS(const std::shared_ptr<Environment>& globals) {

    // osGetEnv(name) -> string | nil
    globals->define("osGetEnv", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto name = requireString(args[0], "osGetEnv", 1);
            const char* val = std::getenv(name.c_str());
            if (!val) return nilValue();
            return makeStr(std::string(val));
        },
        "osGetEnv"
    ));

    // osSetEnv(name, value) -> nil  (best-effort; no-op on some platforms)
    globals->define("osSetEnv", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto name = requireString(args[0], "osSetEnv", 1);
            auto val  = requireString(args[1], "osSetEnv", 2);
#ifdef _WIN32
            _putenv_s(name.c_str(), val.c_str());
#else
            setenv(name.c_str(), val.c_str(), 1);
#endif
            return nilValue();
        },
        "osSetEnv"
    ));

    // osExit(code) -> (never returns)
    globals->define("osExit", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            int code = isNumber(args[0]) ? (int)asNumber(args[0]) : 0;
            std::exit(code);
            return nilValue();
        },
        "osExit"
    ));

    // osExec(cmd) -> number  (exit code)
    globals->define("osExec", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto cmd = requireString(args[0], "osExec", 1);
            int rc = std::system(cmd.c_str());
            return numberToValue((double)rc);
        },
        "osExec"
    ));

    // osCwd() -> string
    globals->define("osCwd", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            return makeStr(std::filesystem::current_path().string());
        },
        "osCwd"
    ));

    // osChdir(path) -> bool
    globals->define("osChdir", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osChdir", 1);
            std::error_code ec;
            std::filesystem::current_path(path, ec);
            return boolValue(!ec);
        },
        "osChdir"
    ));

    // osMkdir(path) -> bool
    globals->define("osMkdir", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osMkdir", 1);
            std::error_code ec;
            std::filesystem::create_directories(path, ec);
            return boolValue(!ec);
        },
        "osMkdir"
    ));

    // osRemove(path) -> bool
    globals->define("osRemove", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osRemove", 1);
            std::error_code ec;
            std::filesystem::remove_all(path, ec);
            return boolValue(!ec);
        },
        "osRemove"
    ));

    // osRename(src, dst) -> bool
    globals->define("osRename", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto src = requireString(args[0], "osRename", 1);
            auto dst = requireString(args[1], "osRename", 2);
            std::error_code ec;
            std::filesystem::rename(src, dst, ec);
            return boolValue(!ec);
        },
        "osRename"
    ));

    // osCopy(src, dst) -> bool
    globals->define("osCopy", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto src = requireString(args[0], "osCopy", 1);
            auto dst = requireString(args[1], "osCopy", 2);
            std::error_code ec;
            std::filesystem::copy(src, dst,
                std::filesystem::copy_options::recursive |
                std::filesystem::copy_options::overwrite_existing, ec);
            return boolValue(!ec);
        },
        "osCopy"
    ));

    // osExists(path) -> bool
    globals->define("osExists", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osExists", 1);
            return boolValue(std::filesystem::exists(path));
        },
        "osExists"
    ));

    // osIsFile(path) -> bool
    globals->define("osIsFile", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osIsFile", 1);
            return boolValue(std::filesystem::is_regular_file(path));
        },
        "osIsFile"
    ));

    // osIsDir(path) -> bool
    globals->define("osIsDir", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osIsDir", 1);
            return boolValue(std::filesystem::is_directory(path));
        },
        "osIsDir"
    ));

    // osListDir(path) -> array<string>
    globals->define("osListDir", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osListDir", 1);
            auto arr = gcAcquireArrayFromPool();
            std::error_code ec;
            for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
                auto sv = StringPool::intern(entry.path().filename().string());
                arr->push(stringValue(sv.data()));
            }
            return arrayValue(arr);
        },
        "osListDir"
    ));

    // osFileSize(path) -> number
    globals->define("osFileSize", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osFileSize", 1);
            std::error_code ec;
            auto sz = std::filesystem::file_size(path, ec);
            if (ec) return numberToValue(-1.0);
            return numberToValue((double)sz);
        },
        "osFileSize"
    ));

    // osAbsPath(path) -> string
    globals->define("osAbsPath", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osAbsPath", 1);
            std::error_code ec;
            auto abs = std::filesystem::absolute(path, ec);
            if (ec) return makeStr(path);
            return makeStr(abs.string());
        },
        "osAbsPath"
    ));

    // osBasename(path) -> string
    globals->define("osBasename", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osBasename", 1);
            return makeStr(std::filesystem::path(path).filename().string());
        },
        "osBasename"
    ));

    // osDirname(path) -> string
    globals->define("osDirname", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osDirname", 1);
            return makeStr(std::filesystem::path(path).parent_path().string());
        },
        "osDirname"
    ));

    // osJoinPath(a, b) -> string
    globals->define("osJoinPath", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto a = requireString(args[0], "osJoinPath", 1);
            auto b = requireString(args[1], "osJoinPath", 2);
            return makeStr((std::filesystem::path(a) / b).string());
        },
        "osJoinPath"
    ));

    // osSleep(ms) -> nil  (sleep for ms milliseconds)
    globals->define("osSleep", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0]))
                throw std::runtime_error("osSleep(): arg 1 must be a number");
            auto ms = (long long)asNumber(args[0]);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            return nilValue();
        },
        "osSleep"
    ));

    // osTime() -> number  (Unix timestamp in seconds)
    globals->define("osTime", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            auto now = std::chrono::system_clock::now().time_since_epoch();
            return numberToValue((double)std::chrono::duration_cast<std::chrono::seconds>(now).count());
        },
        "osTime"
    ));

    // osTimeMs() -> number  (Unix timestamp in milliseconds)
    globals->define("osTimeMs", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
            auto now = std::chrono::system_clock::now().time_since_epoch();
            return numberToValue((double)std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
        },
        "osTimeMs"
    ));

    // osPid() -> number
    globals->define("osPid", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
#ifdef _WIN32
            return numberToValue((double)GetCurrentProcessId());
#else
            return numberToValue((double)getpid());
#endif
        },
        "osPid"
    ));

    // osPlatform() -> string  ("windows" | "linux" | "macos" | "unknown")
    globals->define("osPlatform", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value {
#ifdef _WIN32
            return makeStr("windows");
#elif defined(__APPLE__)
            return makeStr("macos");
#elif defined(__linux__)
            return makeStr("linux");
#else
            return makeStr("unknown");
#endif
        },
        "osPlatform"
    ));

    // osReadFile(path) -> string | nil
    globals->define("osReadFile", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto path = requireString(args[0], "osReadFile", 1);
            std::ifstream f(path);
            if (!f) return nilValue();
            std::ostringstream ss;
            ss << f.rdbuf();
            return makeStr(ss.str());
        },
        "osReadFile"
    ));

    // osWriteFile(path, content) -> bool
    globals->define("osWriteFile", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto path    = requireString(args[0], "osWriteFile", 1);
            auto content = requireString(args[1], "osWriteFile", 2);
            std::ofstream f(path);
            if (!f) return boolValue(false);
            f << content;
            return boolValue(true);
        },
        "osWriteFile"
    ));

    // osAppendFile(path, content) -> bool
    globals->define("osAppendFile", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto path    = requireString(args[0], "osAppendFile", 1);
            auto content = requireString(args[1], "osAppendFile", 2);
            std::ofstream f(path, std::ios::app);
            if (!f) return boolValue(false);
            f << content;
            return boolValue(true);
        },
        "osAppendFile"
    ));
}

} // namespace claw
