#include "native_collections.h"
#include "interpreter/environment.h"
#include "features/callable.h"
#include "features/hashmap.h"
#include "features/array.h"
#include "features/string_pool.h"
#include "interpreter/value.h"
#include "interpreter/gc_alloc.h"
#include <stdexcept>
#include <deque>
#include <unordered_set>
#include <queue>
#include <stack>
#include <sstream>
#include <algorithm>

namespace claw {

static Value mkS(const std::string& s) {
    return stringValue(StringPool::intern(s).data());
}

// ============================================================
// Set  — unique-value collection backed by unordered_set<string>
//        (keys are valueToString representations for hashing)
// ============================================================

static Value makeSet(const std::vector<Value>& initial = {}) {
    // Store items in a ClawArray for ordered iteration + a hashmap for O(1) lookup
    auto items  = std::make_shared<std::vector<Value>>();
    auto lookup = std::make_shared<std::unordered_map<std::string, bool>>();

    for (auto& v : initial) {
        std::string key = valueToString(v);
        if (!lookup->count(key)) {
            lookup->emplace(key, true);
            items->push_back(v);
        }
    }

    auto map = gcNewHashMap();
    map->set("_type", mkS("Set"));

    // size (property-style via callable)
    map->set("size", callableValue(std::make_shared<NativeFunction>(0,
        [items](const std::vector<Value>&) -> Value {
            return numberToValue(static_cast<double>(items->size()));
        }, "size")));

    map->set("add", callableValue(std::make_shared<NativeFunction>(1,
        [items, lookup](const std::vector<Value>& a) -> Value {
            std::string key = valueToString(a[0]);
            if (!lookup->count(key)) {
                lookup->emplace(key, true);
                items->push_back(a[0]);
            }
            return nilValue();
        }, "add")));

    map->set("has", callableValue(std::make_shared<NativeFunction>(1,
        [lookup](const std::vector<Value>& a) -> Value {
            return boolValue(lookup->count(valueToString(a[0])) > 0);
        }, "has")));

    map->set("delete", callableValue(std::make_shared<NativeFunction>(1,
        [items, lookup](const std::vector<Value>& a) -> Value {
            std::string key = valueToString(a[0]);
            if (!lookup->count(key)) return boolValue(false);
            lookup->erase(key);
            items->erase(std::remove_if(items->begin(), items->end(),
                [&key](const Value& v){ return valueToString(v) == key; }), items->end());
            return boolValue(true);
        }, "delete")));

    map->set("clear", callableValue(std::make_shared<NativeFunction>(0,
        [items, lookup](const std::vector<Value>&) -> Value {
            items->clear(); lookup->clear(); return nilValue();
        }, "clear")));

    map->set("toArray", callableValue(std::make_shared<NativeFunction>(0,
        [items](const std::vector<Value>&) -> Value {
            auto arr = gcNewArray();
            for (auto& v : *items) arr->push(v);
            return arrayValue(arr);
        }, "toArray")));

    // union: merge two sets (both converted to arrays first)
    map->set("union", callableValue(std::make_shared<NativeFunction>(1,
        [items, lookup](const std::vector<Value>& a) -> Value {
            std::vector<Value> combined(*items);
            // Accept another Set (hashmap with toArray) or a plain array
            if (isArray(a[0])) {
                auto arr = asArray(a[0]);
                for (size_t i = 0; i < arr->size(); i++) combined.push_back(arr->get(i));
            } else if (isHashMap(a[0])) {
                // Assume it's a Set — get its internal items via toArray
                // We can't call the callable here, so we iterate the hashmap's data
                // and collect non-method values. Instead, accept an array argument.
                // For now, treat as unsupported and return copy.
            }
            return makeSet(combined);
        }, "union")));

    // intersection: items in both sets
    map->set("intersection", callableValue(std::make_shared<NativeFunction>(1,
        [items, lookup](const std::vector<Value>& a) -> Value {
            std::vector<Value> result;
            if (isArray(a[0])) {
                // Build lookup for other array
                std::unordered_set<std::string> otherKeys;
                auto arr = asArray(a[0]);
                for (size_t i = 0; i < arr->size(); i++) otherKeys.insert(valueToString(arr->get(i)));
                for (auto& v : *items) {
                    if (otherKeys.count(valueToString(v))) result.push_back(v);
                }
            }
            return makeSet(result);
        }, "intersection")));

    // difference: items in this set but not in other
    map->set("difference", callableValue(std::make_shared<NativeFunction>(1,
        [items](const std::vector<Value>& a) -> Value {
            std::vector<Value> result;
            if (isArray(a[0])) {
                std::unordered_set<std::string> otherKeys;
                auto arr = asArray(a[0]);
                for (size_t i = 0; i < arr->size(); i++) otherKeys.insert(valueToString(arr->get(i)));
                for (auto& v : *items) {
                    if (!otherKeys.count(valueToString(v))) result.push_back(v);
                }
            }
            return makeSet(result);
        }, "difference")));


    map->set("toString", callableValue(std::make_shared<NativeFunction>(0,
        [items](const std::vector<Value>&) -> Value {
            std::string s = "Set{";
            for (size_t i = 0; i < items->size(); i++) {
                if (i) s += ", ";
                s += valueToString((*items)[i]);
            }
            s += "}";
            return mkS(s);
        }, "toString")));

    return hashMapValue(map);
}

// ============================================================
// Queue  — FIFO backed by std::deque
// ============================================================

static Value makeQueue(const std::vector<Value>& initial = {}) {
    auto data = std::make_shared<std::deque<Value>>(initial.begin(), initial.end());
    auto map = gcNewHashMap();
    map->set("_type", mkS("Queue"));

    map->set("size", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return numberToValue(static_cast<double>(data->size()));
        }, "size")));

    map->set("isEmpty", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return boolValue(data->empty());
        }, "isEmpty")));

    map->set("enqueue", callableValue(std::make_shared<NativeFunction>(1,
        [data](const std::vector<Value>& a) -> Value {
            data->push_back(a[0]); return nilValue();
        }, "enqueue")));

    map->set("dequeue", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            if (data->empty()) throw std::runtime_error("Queue.dequeue: queue is empty");
            Value v = data->front(); data->pop_front(); return v;
        }, "dequeue")));

    map->set("peek", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            if (data->empty()) return nilValue();
            return data->front();
        }, "peek")));

    map->set("clear", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            data->clear(); return nilValue();
        }, "clear")));

    map->set("toArray", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            auto arr = gcNewArray();
            for (auto& v : *data) arr->push(v);
            return arrayValue(arr);
        }, "toArray")));

    map->set("toString", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            std::string s = "Queue[";
            bool first = true;
            for (auto& v : *data) { if (!first) s += ", "; s += valueToString(v); first = false; }
            s += "]";
            return mkS(s);
        }, "toString")));

    return hashMapValue(map);
}

// ============================================================
// Stack  — LIFO backed by std::vector
// ============================================================

static Value makeStack(const std::vector<Value>& initial = {}) {
    auto data = std::make_shared<std::vector<Value>>(initial);
    auto map = gcNewHashMap();
    map->set("_type", mkS("Stack"));

    map->set("size", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return numberToValue(static_cast<double>(data->size()));
        }, "size")));

    map->set("isEmpty", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return boolValue(data->empty());
        }, "isEmpty")));

    map->set("push", callableValue(std::make_shared<NativeFunction>(1,
        [data](const std::vector<Value>& a) -> Value {
            data->push_back(a[0]); return nilValue();
        }, "push")));

    map->set("pop", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            if (data->empty()) throw std::runtime_error("Stack.pop: stack is empty");
            Value v = data->back(); data->pop_back(); return v;
        }, "pop")));

    map->set("peek", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            if (data->empty()) return nilValue();
            return data->back();
        }, "peek")));

    map->set("clear", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            data->clear(); return nilValue();
        }, "clear")));

    map->set("toArray", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            auto arr = gcNewArray();
            for (auto& v : *data) arr->push(v);
            return arrayValue(arr);
        }, "toArray")));

    map->set("toString", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            std::string s = "Stack[";
            for (size_t i = 0; i < data->size(); i++) {
                if (i) s += ", ";
                s += valueToString((*data)[i]);
            }
            s += "]";
            return mkS(s);
        }, "toString")));

    return hashMapValue(map);
}

// ============================================================
// Deque  — double-ended queue
// ============================================================

static Value makeDeque(const std::vector<Value>& initial = {}) {
    auto data = std::make_shared<std::deque<Value>>(initial.begin(), initial.end());
    auto map = gcNewHashMap();
    map->set("_type", mkS("Deque"));

    map->set("size", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return numberToValue(static_cast<double>(data->size()));
        }, "size")));

    map->set("isEmpty", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return boolValue(data->empty());
        }, "isEmpty")));

    map->set("pushFront", callableValue(std::make_shared<NativeFunction>(1,
        [data](const std::vector<Value>& a) -> Value {
            data->push_front(a[0]); return nilValue();
        }, "pushFront")));

    map->set("pushBack", callableValue(std::make_shared<NativeFunction>(1,
        [data](const std::vector<Value>& a) -> Value {
            data->push_back(a[0]); return nilValue();
        }, "pushBack")));

    map->set("popFront", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            if (data->empty()) throw std::runtime_error("Deque.popFront: deque is empty");
            Value v = data->front(); data->pop_front(); return v;
        }, "popFront")));

    map->set("popBack", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            if (data->empty()) throw std::runtime_error("Deque.popBack: deque is empty");
            Value v = data->back(); data->pop_back(); return v;
        }, "popBack")));

    map->set("peekFront", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return data->empty() ? nilValue() : data->front();
        }, "peekFront")));

    map->set("peekBack", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return data->empty() ? nilValue() : data->back();
        }, "peekBack")));

    map->set("get", callableValue(std::make_shared<NativeFunction>(1,
        [data](const std::vector<Value>& a) -> Value {
            int idx = static_cast<int>(asNumber(a[0]));
            if (idx < 0) idx = static_cast<int>(data->size()) + idx;
            if (idx < 0 || idx >= static_cast<int>(data->size())) return nilValue();
            return (*data)[idx];
        }, "get")));

    map->set("clear", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            data->clear(); return nilValue();
        }, "clear")));

    map->set("toArray", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            auto arr = gcNewArray();
            for (auto& v : *data) arr->push(v);
            return arrayValue(arr);
        }, "toArray")));

    map->set("toString", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            std::string s = "Deque[";
            bool first = true;
            for (auto& v : *data) { if (!first) s += ", "; s += valueToString(v); first = false; }
            s += "]";
            return mkS(s);
        }, "toString")));

    return hashMapValue(map);
}

// ============================================================
// PriorityQueue  — min-heap backed by std::vector + comparator
// ============================================================

static Value makePriorityQueue() {
    // Stores {priority: number, value: any} pairs, min-heap by priority
    using PQItem = std::pair<double, Value>;
    auto data = std::make_shared<std::vector<PQItem>>();
    auto map = gcNewHashMap();
    map->set("_type", mkS("PriorityQueue"));

    auto heapCmp = [](const PQItem& a, const PQItem& b){ return a.first > b.first; }; // min-heap

    map->set("size", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return numberToValue(static_cast<double>(data->size()));
        }, "size")));

    map->set("isEmpty", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return boolValue(data->empty());
        }, "isEmpty")));

    map->set("enqueue", callableValue(std::make_shared<NativeFunction>(2,
        [data, heapCmp](const std::vector<Value>& a) -> Value {
            double priority = asNumber(a[1]);
            data->push_back({priority, a[0]});
            std::push_heap(data->begin(), data->end(),
                [](const PQItem& x, const PQItem& y){ return x.first > y.first; });
            return nilValue();
        }, "enqueue")));

    map->set("dequeue", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            if (data->empty()) throw std::runtime_error("PriorityQueue.dequeue: empty");
            std::pop_heap(data->begin(), data->end(),
                [](const PQItem& x, const PQItem& y){ return x.first > y.first; });
            Value v = data->back().second;
            data->pop_back();
            return v;
        }, "dequeue")));

    map->set("peek", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return data->empty() ? nilValue() : data->front().second;
        }, "peek")));

    map->set("clear", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            data->clear(); return nilValue();
        }, "clear")));

    map->set("toString", callableValue(std::make_shared<NativeFunction>(0,
        [data](const std::vector<Value>&) -> Value {
            return mkS("PriorityQueue(size=" + std::to_string(data->size()) + ")");
        }, "toString")));

    return hashMapValue(map);
}

// ============================================================
// Register all collections as globals
// ============================================================

void registerNativeCollections(const std::shared_ptr<Environment>& globals) {
    // Set(items...) constructor
    globals->define("Set", std::make_shared<NativeFunction>(-1,
        [](const std::vector<Value>& args) -> Value {
            return makeSet(args);
        }, "Set"));

    // Queue(items...) constructor
    globals->define("Queue", std::make_shared<NativeFunction>(-1,
        [](const std::vector<Value>& args) -> Value {
            return makeQueue(args);
        }, "Queue"));

    // Stack(items...) constructor
    globals->define("Stack", std::make_shared<NativeFunction>(-1,
        [](const std::vector<Value>& args) -> Value {
            return makeStack(args);
        }, "Stack"));

    // Deque(items...) constructor
    globals->define("Deque", std::make_shared<NativeFunction>(-1,
        [](const std::vector<Value>& args) -> Value {
            return makeDeque(args);
        }, "Deque"));

    // PriorityQueue() constructor
    globals->define("PriorityQueue", std::make_shared<NativeFunction>(0,
        [](const std::vector<Value>&) -> Value {
            return makePriorityQueue();
        }, "PriorityQueue"));
}

} // namespace claw
