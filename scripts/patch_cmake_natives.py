"""Add native_regex, native_os, native_events to CMakeLists.txt source lists."""

path = 'e:/ClawScript/CMakeLists.txt'
content = open(path, encoding='utf-8').read()

OLD = '    src/interpreter/natives/native_collections.cpp'
NEW = (
    '    src/interpreter/natives/native_collections.cpp\n'
    '    src/interpreter/natives/native_regex.cpp\n'
    '    src/interpreter/natives/native_os.cpp\n'
    '    src/interpreter/natives/native_events.cpp'
)

count = content.count(OLD)
print(f'Found {count} occurrences of anchor')
content = content.replace(OLD, NEW)
open(path, 'w', encoding='utf-8').write(content)
print('CMakeLists.txt patched OK')
