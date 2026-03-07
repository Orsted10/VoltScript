import re

path = 'e:/ClawScript/src/interpreter/interpreter.cpp'
content = open(path, encoding='utf-8').read()

old = (
    '        // Handle hash map properties/methods\n'
    '        if (expr->member == "size") {\n'
    '            return numberToValue(static_cast<double>(map->size()));\n'
    '        }'
)

new = (
    '        // Handle hash map properties/methods\n'
    '        // If the map stores a callable under this key (e.g. Set/Queue/Stack methods),\n'
    '        // prefer the stored value so user-defined collections can override built-ins.\n'
    '        if (map->contains(expr->member)) {\n'
    '            Value stored = map->get(expr->member);\n'
    '            if (isCallable(stored)) return stored;\n'
    '        }\n'
    '        if (expr->member == "size") {\n'
    '            return numberToValue(static_cast<double>(map->size()));\n'
    '        }'
)

if old in content:
    content2 = content.replace(old, new, 1)
    open(path, 'w', encoding='utf-8').write(content2)
    print('REPLACED OK')
else:
    idx = content.find('Handle hash map properties/methods')
    print('NOT FOUND, context:')
    print(repr(content[idx:idx+250]))
