/**
 * native_regex.cpp — Regex native module for ClawScript
 *
 * Exposes C++11 <regex> to ClawScript scripts.
 *
 * Functions registered:
 *   reMatch(pattern, str)          -> bool
 *   reSearch(pattern, str)         -> string | nil   (first match)
 *   reFindAll(pattern, str)        -> array<string>
 *   reReplace(pattern, str, repl)  -> string
 *   reSplit(pattern, str)          -> array<string>
 *   reGroups(pattern, str)         -> array<string>  (capture groups of first match)
 *   reTest(pattern, str)           -> bool           (alias for reMatch)
 *   reEscape(str)                  -> string         (escape special regex chars)
 */

#include "native_regex.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "features/array.h"
#include "features/string_pool.h"
#include "interpreter/value.h"
#include <regex>
#include <string>
#include <stdexcept>

namespace claw {

// ── helpers ──────────────────────────────────────────────────────────────────

static std::string requireString(const Value& v, const char* fn, int argIdx) {
    if (!isString(v)) {
        throw std::runtime_error(std::string(fn) + "(): argument " +
                                 std::to_string(argIdx) + " must be a string");
    }
    const char* p = asStringPtr(v);
    return p ? std::string(p) : std::string();
}

static Value makeStr(const std::string& s) {
    auto sv = StringPool::intern(s);
    return stringValue(sv.data());
}

static Value makeArray(const std::vector<std::string>& items) {
    auto arr = gcAcquireArrayFromPool();
    if (!arr) arr = std::make_shared<ClawArray>();
    for (const auto& s : items) {
        arr->push(makeStr(s));
    }
    return arrayValue(arr);
}

// ── registration ─────────────────────────────────────────────────────────────

void registerNativeRegex(const std::shared_ptr<Environment>& globals) {

    // reMatch(pattern, str) -> bool
    // Returns true if the entire string matches the pattern (std::regex_match).
    globals->define("reMatch", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto pat = requireString(args[0], "reMatch", 1);
            auto str = requireString(args[1], "reMatch", 2);
            try {
                std::regex re(pat);
                return boolValue(std::regex_match(str, re));
            } catch (const std::regex_error& e) {
                throw std::runtime_error(std::string("reMatch(): invalid regex: ") + e.what());
            }
        },
        "reMatch"
    ));

    // reTest(pattern, str) -> bool  (alias: search anywhere in string)
    globals->define("reTest", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto pat = requireString(args[0], "reTest", 1);
            auto str = requireString(args[1], "reTest", 2);
            try {
                std::regex re(pat);
                return boolValue(std::regex_search(str, re));
            } catch (const std::regex_error& e) {
                throw std::runtime_error(std::string("reTest(): invalid regex: ") + e.what());
            }
        },
        "reTest"
    ));

    // reSearch(pattern, str) -> string | nil
    // Returns the first match substring, or nil if not found.
    globals->define("reSearch", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto pat = requireString(args[0], "reSearch", 1);
            auto str = requireString(args[1], "reSearch", 2);
            try {
                std::regex re(pat);
                std::smatch m;
                if (std::regex_search(str, m, re)) {
                    return makeStr(m[0].str());
                }
                return nilValue();
            } catch (const std::regex_error& e) {
                throw std::runtime_error(std::string("reSearch(): invalid regex: ") + e.what());
            }
        },
        "reSearch"
    ));

    // reFindAll(pattern, str) -> array<string>
    // Returns all non-overlapping matches.
    globals->define("reFindAll", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto pat = requireString(args[0], "reFindAll", 1);
            auto str = requireString(args[1], "reFindAll", 2);
            try {
                std::regex re(pat);
                std::vector<std::string> results;
                auto begin = std::sregex_iterator(str.begin(), str.end(), re);
                auto end   = std::sregex_iterator();
                for (auto it = begin; it != end; ++it) {
                    results.push_back((*it)[0].str());
                }
                return makeArray(results);
            } catch (const std::regex_error& e) {
                throw std::runtime_error(std::string("reFindAll(): invalid regex: ") + e.what());
            }
        },
        "reFindAll"
    ));

    // reReplace(pattern, str, replacement) -> string
    // Replaces all matches with replacement string.
    globals->define("reReplace", std::make_shared<NativeFunction>(
        3,
        [](const std::vector<Value>& args) -> Value {
            auto pat  = requireString(args[0], "reReplace", 1);
            auto str  = requireString(args[1], "reReplace", 2);
            auto repl = requireString(args[2], "reReplace", 3);
            try {
                std::regex re(pat);
                return makeStr(std::regex_replace(str, re, repl));
            } catch (const std::regex_error& e) {
                throw std::runtime_error(std::string("reReplace(): invalid regex: ") + e.what());
            }
        },
        "reReplace"
    ));

    // reSplit(pattern, str) -> array<string>
    // Splits str by the regex pattern.
    globals->define("reSplit", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto pat = requireString(args[0], "reSplit", 1);
            auto str = requireString(args[1], "reSplit", 2);
            try {
                std::regex re(pat);
                std::vector<std::string> parts;
                std::sregex_token_iterator it(str.begin(), str.end(), re, -1);
                std::sregex_token_iterator end;
                for (; it != end; ++it) {
                    parts.push_back(it->str());
                }
                return makeArray(parts);
            } catch (const std::regex_error& e) {
                throw std::runtime_error(std::string("reSplit(): invalid regex: ") + e.what());
            }
        },
        "reSplit"
    ));

    // reGroups(pattern, str) -> array<string>
    // Returns capture groups of the first match (index 0 = full match, 1+ = groups).
    globals->define("reGroups", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto pat = requireString(args[0], "reGroups", 1);
            auto str = requireString(args[1], "reGroups", 2);
            try {
                std::regex re(pat);
                std::smatch m;
                std::vector<std::string> groups;
                if (std::regex_search(str, m, re)) {
                    for (size_t i = 0; i < m.size(); i++) {
                        groups.push_back(m[i].str());
                    }
                }
                return makeArray(groups);
            } catch (const std::regex_error& e) {
                throw std::runtime_error(std::string("reGroups(): invalid regex: ") + e.what());
            }
        },
        "reGroups"
    ));

    // reEscape(str) -> string
    // Escapes all special regex metacharacters in str.
    globals->define("reEscape", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            auto str = requireString(args[0], "reEscape", 1);
            static const std::string meta = R"(\.^$|?*+()[]{})" ;
            std::string out;
            out.reserve(str.size() * 2);
            for (char c : str) {
                if (meta.find(c) != std::string::npos) out += '\\';
                out += c;
            }
            return makeStr(out);
        },
        "reEscape"
    ));
}

} // namespace claw
