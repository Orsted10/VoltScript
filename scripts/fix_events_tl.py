"""
Fix tl_current_interpreter linker error:
1. Define thread_local in interpreter.cpp
2. Move extern decl to file scope in native_events.cpp
"""

# ── Fix 1: define in interpreter.cpp ─────────────────────────────────────────
ipath = 'e:/ClawScript/src/interpreter/interpreter.cpp'
ic = open(ipath, encoding='utf-8').read()

# Add definition after the includes block (find namespace claw {)
OLD_NS = 'namespace claw {'
NEW_NS = 'namespace claw {\n\nthread_local Interpreter* tl_current_interpreter = nullptr;\n'
if OLD_NS in ic and 'tl_current_interpreter' not in ic:
    ic = ic.replace(OLD_NS, NEW_NS, 1)
    print('[1] Defined tl_current_interpreter in interpreter.cpp')
else:
    print('[1] SKIP (already present or anchor not found)')

# Set it at the start of execute() — find the execute method
# We look for the Interpreter::execute signature
OLD_EXEC = 'void Interpreter::execute(const std::vector<StmtPtr>& statements) {'
NEW_EXEC = 'void Interpreter::execute(const std::vector<StmtPtr>& statements) {\n    tl_current_interpreter = this;'
if OLD_EXEC in ic and 'tl_current_interpreter = this' not in ic:
    ic = ic.replace(OLD_EXEC, NEW_EXEC, 1)
    print('[1b] Set tl_current_interpreter in execute()')
else:
    print('[1b] SKIP execute() anchor')

open(ipath, 'w', encoding='utf-8').write(ic)

# ── Fix 2: move extern to file scope in native_events.cpp ────────────────────
epath = 'e:/ClawScript/src/interpreter/natives/native_events.cpp'
ec = open(epath, encoding='utf-8').read()

# Remove the inline extern declarations inside lambdas
ec = ec.replace(
    '                    extern thread_local Interpreter* tl_current_interpreter;\n'
    '                    if (tl_current_interpreter) {\n'
    '                        fn->call(*tl_current_interpreter, payload);\n'
    '                    }',
    '                    if (tl_current_interpreter) {\n'
    '                        fn->call(*tl_current_interpreter, payload);\n'
    '                    }'
)
ec = ec.replace(
    '                    extern thread_local Interpreter* tl_current_interpreter;\n'
    '                        if (tl_current_interpreter) {\n'
    '                            return asCallable(innerHandler)->call(*tl_current_interpreter, callArgs);\n'
    '                        }',
    '                        if (tl_current_interpreter) {\n'
    '                            return asCallable(innerHandler)->call(*tl_current_interpreter, callArgs);\n'
    '                        }'
)

# Add file-scope extern declaration after the includes
OLD_NS2 = 'namespace claw {'
NEW_NS2 = 'namespace claw {\n\nextern thread_local Interpreter* tl_current_interpreter;\n'
if OLD_NS2 in ec and 'extern thread_local Interpreter* tl_current_interpreter' not in ec:
    ec = ec.replace(OLD_NS2, NEW_NS2, 1)
    print('[2] Added file-scope extern in native_events.cpp')
else:
    print('[2] SKIP (already present or anchor not found)')

# Also need to include interpreter.h for the Interpreter type
OLD_INC = '#include "interpreter/value.h"'
NEW_INC = '#include "interpreter/value.h"\n#include "interpreter/interpreter.h"'
if OLD_INC in ec and '"interpreter/interpreter.h"' not in ec:
    ec = ec.replace(OLD_INC, NEW_INC, 1)
    print('[2b] Added interpreter.h include in native_events.cpp')

open(epath, 'w', encoding='utf-8').write(ec)
print('Done')
