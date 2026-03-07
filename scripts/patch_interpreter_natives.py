"""Add Regex, OS, Events native registrations to interpreter.cpp"""

path = 'e:/ClawScript/src/interpreter/interpreter.cpp'
content = open(path, encoding='utf-8').read()

# ── 1. Add includes after native_collections.h ────────────────────────────────
OLD_INC = '#include "interpreter/natives/native_collections.h"'
NEW_INC = (
    '#include "interpreter/natives/native_collections.h"\n'
    '#include "interpreter/natives/native_regex.h"\n'
    '#include "interpreter/natives/native_os.h"\n'
    '#include "interpreter/natives/native_events.h"'
)
if OLD_INC in content:
    content = content.replace(OLD_INC, NEW_INC, 1)
    print('[1] Added includes')
else:
    print('[1] SKIP: native_collections.h include not found')

# ── 2. Add registration calls after registerNativeCollections ─────────────────
OLD_REG = '    registerNativeCollections(globals_);\n    \n    // ==================== TIME AND DATE'
NEW_REG = (
    '    registerNativeCollections(globals_);\n'
    '    registerNativeRegex(globals_);\n'
    '    registerNativeOS(globals_);\n'
    '    registerNativeEvents(globals_);\n'
    '    \n    // ==================== TIME AND DATE'
)
if OLD_REG in content:
    content = content.replace(OLD_REG, NEW_REG, 1)
    print('[2] Added registration calls')
else:
    # Try alternate spacing
    OLD_REG2 = '    registerNativeCollections(globals_);\n\n    // ==================== TIME AND DATE'
    NEW_REG2 = (
        '    registerNativeCollections(globals_);\n'
        '    registerNativeRegex(globals_);\n'
        '    registerNativeOS(globals_);\n'
        '    registerNativeEvents(globals_);\n\n'
        '    // ==================== TIME AND DATE'
    )
    if OLD_REG2 in content:
        content = content.replace(OLD_REG2, NEW_REG2, 1)
        print('[2] Added registration calls (alt spacing)')
    else:
        # Just append after registerNativeCollections(globals_);
        OLD_REG3 = '    registerNativeCollections(globals_);'
        NEW_REG3 = (
            '    registerNativeCollections(globals_);\n'
            '    registerNativeRegex(globals_);\n'
            '    registerNativeOS(globals_);\n'
            '    registerNativeEvents(globals_);'
        )
        if OLD_REG3 in content:
            content = content.replace(OLD_REG3, NEW_REG3, 1)
            print('[2] Added registration calls (simple anchor)')
        else:
            print('[2] SKIP: registerNativeCollections anchor not found')

open(path, 'w', encoding='utf-8').write(content)
print('interpreter.cpp patched OK')
