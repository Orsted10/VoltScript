"""Add the 24 missing opcodes to vm.cpp before the default: case."""

path = 'e:/ClawScript/src/vm/vm.cpp'
content = open(path, encoding='utf-8').read()

NEW_OPCODES = r"""
            case OpCode::Dup: {
                push(peek(0));
                break;
            }
            case OpCode::JumpIfTrue: {
                uint16_t offset = READ_SHORT();
                if (!isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OpCode::JumpIfNil: {
                uint16_t offset = READ_SHORT();
                if (isNil(peek(0))) frame->ip += offset;
                break;
            }
            case OpCode::JumpIfNotNil: {
                uint16_t offset = READ_SHORT();
                if (!isNil(peek(0))) frame->ip += offset;
                break;
            }
            case OpCode::ToString: {
                Value v = pop();
                auto sv = StringPool::intern(valueToString(v));
                push(stringValue(sv.data()));
                break;
            }
            case OpCode::NewArray: {
                auto arr = gcAcquireArrayFromPool();
                push(arrayValue(arr));
                break;
            }
            case OpCode::ArrayPush: {
                Value val = pop();
                Value arrVal = peek(0);
                if (isArray(arrVal)) {
                    asArray(arrVal)->push(val);
                } else {
                    stackTop_ = stackTop;
                    std::cerr << "ArrayPush: expected array on stack\n";
                    return InterpretResult::RuntimeError;
                }
                break;
            }
            case OpCode::ArraySlice: {
                uint8_t startIdx = READ_BYTE();
                Value arrVal = peek(0);
                if (isArray(arrVal)) {
                    auto src = asArray(arrVal);
                    auto result = gcAcquireArrayFromPool();
                    for (size_t i = startIdx; i < src->size(); i++) {
                        result->push(src->get(i));
                    }
                    pop();
                    push(arrayValue(result));
                } else {
                    stackTop_ = stackTop;
                    std::cerr << "ArraySlice: expected array on stack\n";
                    return InterpretResult::RuntimeError;
                }
                break;
            }
            case OpCode::NewHashMap: {
                auto map = gcAcquireHashMapFromPool();
                push(hashMapValue(map));
                break;
            }
            case OpCode::HashMapSet: {
                Value val = pop();
                Value key = pop();
                Value mapVal = peek(0);
                if (isHashMap(mapVal)) {
                    asHashMap(mapVal)->set(valueToString(key), val);
                } else {
                    stackTop_ = stackTop;
                    std::cerr << "HashMapSet: expected hashmap on stack\n";
                    return InterpretResult::RuntimeError;
                }
                break;
            }
            case OpCode::Spread: {
                Value val = pop();
                if (isArray(val)) {
                    auto arr = asArray(val);
                    for (size_t i = 0; i < arr->size(); i++) {
                        push(arr->get(i));
                    }
                } else if (isString(val)) {
                    const char* s = asStringPtr(val);
                    if (s) {
                        std::string str(s);
                        for (char c : str) {
                            auto sv = StringPool::intern(std::string(1, c));
                            push(stringValue(sv.data()));
                        }
                    }
                }
                break;
            }
            case OpCode::GetIter: {
                Value val = pop();
                auto iterMap = gcAcquireHashMapFromPool();
                if (isArray(val)) {
                    iterMap->set("__iter_array__", val);
                    iterMap->set("__iter_index__", numberToValue(0.0));
                } else if (isString(val)) {
                    iterMap->set("__iter_string__", val);
                    iterMap->set("__iter_index__", numberToValue(0.0));
                } else if (isHashMap(val)) {
                    auto src = asHashMap(val);
                    auto keys = src->getKeys();
                    auto keysArr = gcAcquireArrayFromPool();
                    for (auto& k : keys) {
                        auto sv = StringPool::intern(k);
                        keysArr->push(stringValue(sv.data()));
                    }
                    iterMap->set("__iter_array__", arrayValue(keysArr));
                    iterMap->set("__iter_index__", numberToValue(0.0));
                } else {
                    iterMap->set("__iter_done__", boolValue(true));
                }
                push(hashMapValue(iterMap));
                break;
            }
            case OpCode::IterNext: {
                Value iterVal = peek(0);
                if (!isHashMap(iterVal)) { push(boolValue(false)); break; }
                auto iterMap = asHashMap(iterVal);
                if (iterMap->contains("__iter_done__")) { push(boolValue(false)); break; }
                Value idxVal = iterMap->get("__iter_index__");
                int idx = (int)asNumber(idxVal);
                if (iterMap->contains("__iter_array__")) {
                    Value arrVal = iterMap->get("__iter_array__");
                    auto arr = asArray(arrVal);
                    if (idx >= (int)arr->size()) {
                        push(boolValue(false));
                    } else {
                        iterMap->set("__iter_index__", numberToValue(idx + 1.0));
                        push(arr->get(idx));
                    }
                } else if (iterMap->contains("__iter_string__")) {
                    Value strVal = iterMap->get("__iter_string__");
                    const char* s = asStringPtr(strVal);
                    std::string str = s ? std::string(s) : "";
                    if (idx >= (int)str.size()) {
                        push(boolValue(false));
                    } else {
                        iterMap->set("__iter_index__", numberToValue(idx + 1.0));
                        auto sv = StringPool::intern(std::string(1, str[idx]));
                        push(stringValue(sv.data()));
                    }
                } else {
                    push(boolValue(false));
                }
                break;
            }
            case OpCode::Class: {
                const char* namePtr = READ_STRING_PTR();
                // Create a minimal class via interpreter (class system lives there)
                // For now push a hashmap acting as a class namespace
                auto classMap = gcAcquireHashMapFromPool();
                auto sv = StringPool::intern(std::string("__class__") + namePtr);
                classMap->set("__name__", stringValue(sv.data()));
                push(hashMapValue(classMap));
                break;
            }
            case OpCode::Inherit: {
                // Copy methods from super (both represented as hashmaps in VM path)
                Value superVal = peek(1);
                Value subVal = peek(0);
                if (isHashMap(superVal) && isHashMap(subVal)) {
                    auto superMap = asHashMap(superVal);
                    auto subMap = asHashMap(subVal);
                    for (const auto& key : superMap->getKeys()) {
                        if (!subMap->contains(key)) {
                            subMap->set(key, superMap->get(key));
                        }
                    }
                }
                pop(); // subclass
                break;
            }
            case OpCode::Method: {
                const char* namePtr = READ_STRING_PTR();
                Value method = pop();
                Value classVal = peek(0);
                if (isHashMap(classVal)) {
                    asHashMap(classVal)->set(std::string(namePtr), method);
                }
                break;
            }
            case OpCode::Invoke: {
                const char* namePtr = READ_STRING_PTR();
                uint8_t argCount = READ_BYTE();
                Value receiver = peek(argCount);
                Value method = nilValue();
                if (isInstance(receiver)) {
                    auto instance = asInstance(receiver);
                    Token nameTok(TokenType::Identifier, namePtr, 0);
                    if (instance->has(nameTok)) {
                        method = instance->get(nameTok);
                    } else {
                        auto cls = instance->getClass();
                        auto clsMethod = cls->findMethod(std::string(namePtr));
                        if (clsMethod) method = callableValue(clsMethod);
                    }
                } else if (isHashMap(receiver)) {
                    auto map = asHashMap(receiver);
                    if (map->contains(std::string(namePtr))) {
                        method = map->get(std::string(namePtr));
                    }
                }
                if (isNil(method)) {
                    stackTop_ = stackTop;
                    std::cerr << "Undefined method '" << namePtr << "'.\n";
                    return InterpretResult::RuntimeError;
                }
                stackTop[-argCount - 1] = method;
                stackTop_ = stackTop;
                if (!callValue(method, argCount)) return InterpretResult::RuntimeError;
                stackTop = stackTop_;
                frame = &frames_[frameCount_ - 1];
                break;
            }
            case OpCode::SuperInvoke: {
                const char* namePtr = READ_STRING_PTR();
                uint8_t argCount = READ_BYTE();
                Value superVal = pop();
                Value method = nilValue();
                if (isInstance(superVal)) {
                    auto cls = asInstance(superVal)->getClass()->getSuperclass();
                    if (cls) {
                        auto m = cls->findMethod(std::string(namePtr));
                        if (m) method = callableValue(m);
                    }
                }
                if (isNil(method)) {
                    stackTop_ = stackTop;
                    std::cerr << "Undefined super method '" << namePtr << "'.\n";
                    return InterpretResult::RuntimeError;
                }
                stackTop_ = stackTop;
                if (!callValue(method, argCount)) return InterpretResult::RuntimeError;
                stackTop = stackTop_;
                frame = &frames_[frameCount_ - 1];
                break;
            }
            case OpCode::Construct: {
                uint8_t argCount = READ_BYTE();
                Value classVal = peek(argCount);
                if (!isClass(classVal)) {
                    stackTop_ = stackTop;
                    std::cerr << "Can only construct classes.\n";
                    return InterpretResult::RuntimeError;
                }
                auto cls = asClass(classVal);
                auto instance = std::make_shared<ClawInstance>(cls);
                stackTop[-argCount - 1] = instanceValue(instance);
                auto initMethod = cls->findMethod("__init__");
                if (initMethod) {
                    stackTop_ = stackTop;
                    if (!callValue(callableValue(initMethod), argCount))
                        return InterpretResult::RuntimeError;
                    stackTop = stackTop_;
                    frame = &frames_[frameCount_ - 1];
                }
                break;
            }
            case OpCode::GetMeta: {
                // Meta not yet stored on instances; push nil
                pop();
                push(nilValue());
                break;
            }
            case OpCode::SetMeta: {
                pop(); // meta value - no-op for now
                break;
            }
            case OpCode::Await: {
                // Sync mode: await is a pass-through no-op
                break;
            }
            case OpCode::Yield: {
                // Non-generator context: no-op
                break;
            }
"""

ANCHOR = '            default:\n                stackTop_ = stackTop;\n                std::cerr << "Unknown opcode "'

if ANCHOR in content:
    content2 = content.replace(ANCHOR, NEW_OPCODES + '\n' + ANCHOR, 1)
    open(path, 'w', encoding='utf-8').write(content2)
    print('PATCHED OK — added 24 opcodes')
else:
    print('ANCHOR NOT FOUND')
    # Try to find it
    idx = content.find('default:')
    while idx != -1:
        print(repr(content[idx:idx+80]))
        idx = content.find('default:', idx+1)
