"""Patch main.cpp: add --vm flag, runFileVM(), and route execution."""

path = 'e:/ClawScript/src/main.cpp'
content = open(path, encoding='utf-8').read()

# ── 1. Add runFileVM() before runPrompt() ─────────────────────────────────────
RUN_FILE_VM = '''void runFileVM(const std::string& path, bool debugMode) {
    std::unique_ptr<claw::Chunk> chunk;
    if (!compileFileToChunk(path, chunk, debugMode)) {
        exit(65);
    }
    claw::VM vm;
    auto result = vm.interpret(*chunk);
    if (result == claw::InterpretResult::CompileError) exit(65);
    if (result == claw::InterpretResult::RuntimeError)  exit(70);
}

'''

ANCHOR_RUNPROMPT = 'void runPrompt() {'
if ANCHOR_RUNPROMPT in content:
    content = content.replace(ANCHOR_RUNPROMPT, RUN_FILE_VM + ANCHOR_RUNPROMPT, 1)
    print('[1] Inserted runFileVM()')
else:
    print('[1] SKIP: runPrompt anchor not found')

# ── 2. Declare useVM after jitAggressive ──────────────────────────────────────
OLD_DECL = '    bool jitAggressive = false;\n    bool disableCallIC = false;'
NEW_DECL  = '    bool jitAggressive = false;\n    bool useVM = false;\n    bool disableCallIC = false;'
if OLD_DECL in content:
    content = content.replace(OLD_DECL, NEW_DECL, 1)
    print('[2] Declared useVM variable')
else:
    print('[2] SKIP: jitAggressive decl anchor not found')

# ── 3. Add --vm / --interpreter flags before --disable-call-ic ────────────────
OLD_FLAGS = '        } else if (arg == "--disable-call-ic") {'
NEW_FLAGS = (
    '        } else if (arg == "--vm") {\n'
    '            useVM = true;\n'
    '        } else if (arg == "--interpreter") {\n'
    '            useVM = false;\n'
    '        } else if (arg == "--disable-call-ic") {'
)
if OLD_FLAGS in content:
    content = content.replace(OLD_FLAGS, NEW_FLAGS, 1)
    print('[3] Inserted --vm / --interpreter flags')
else:
    print('[3] SKIP: --disable-call-ic anchor not found')

# ── 4. Route execution through VM when --vm flag is set ───────────────────────
OLD_EXEC = (
    '    if (!scriptPath.empty()) {\n'
    '        // Run file\n'
    '        runFile(scriptPath, interpreter, debugMode);\n'
    '    } else {'
)
NEW_EXEC = (
    '    if (!scriptPath.empty()) {\n'
    '        if (useVM) {\n'
    '            runFileVM(scriptPath, debugMode);\n'
    '        } else {\n'
    '            // Tree-walk interpreter (default)\n'
    '            runFile(scriptPath, interpreter, debugMode);\n'
    '        }\n'
    '    } else {'
)
if OLD_EXEC in content:
    content = content.replace(OLD_EXEC, NEW_EXEC, 1)
    print('[4] Routed execution through VM when --vm flag set')
else:
    print('[4] SKIP: execution routing anchor not found')

# ── 5. Add --vm to help text ──────────────────────────────────────────────────
OLD_HELP = '  --disable-call-ic   Disable interpreter call inline cache\n'
NEW_HELP = (
    '  --vm               Execute via bytecode VM (Compiler -> VM path)\n'
    '  --interpreter      Execute via tree-walk interpreter (default)\n'
    '  --disable-call-ic   Disable interpreter call inline cache\n'
)
if OLD_HELP in content:
    content = content.replace(OLD_HELP, NEW_HELP, 1)
    print('[5] Added --vm to help text')
else:
    print('[5] SKIP: help text anchor not found')

open(path, 'w', encoding='utf-8').write(content)
print('main.cpp patched OK')
