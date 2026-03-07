import subprocess, sys

exe = r'e:\ClawScript\build\bin\Release\claw.exe'
tests = [
    ('basic',       r'e:\ClawScript\examples\basic\hello_test.claw'),
    ('collections', r'e:\ClawScript\examples\basic\collections_test.claw'),
    ('gamemath',    r'e:\ClawScript\examples\math\gamemath_test.claw'),
    ('os',          r'e:\ClawScript\examples\stdlib\os_test.claw'),
    ('regex',       r'e:\ClawScript\examples\stdlib\regex_test.claw'),
    ('events',      r'e:\ClawScript\examples\stdlib\events_test.claw'),
    ('lang_feat',   r'e:\ClawScript\examples\stdlib\language_features_test.claw'),
    ('ecs',         r'e:\ClawScript\examples\stdlib\ecs_test.claw'),
    ('class_mini',  r'e:\ClawScript\test_class_mini.claw'),
    ('metamethods', r'e:\ClawScript\examples\stdlib\metamethods_strings_test.claw'),
    ('coroutines',  r'e:\ClawScript\examples\stdlib\coroutine_test.claw'),
    ('for_of_coro', r'e:\ClawScript\examples\stdlib\for_of_coro_test.claw'),
]

passed = 0
failed = []
for name, path in tests:
    r = subprocess.run([exe, path], capture_output=True, timeout=15)
    ok = r.returncode == 0
    if ok:
        passed += 1
        print(f'  [PASS] {name}')
    else:
        failed.append(name)
        out = r.stdout.decode('utf-8', errors='replace').strip()
        err = r.stderr.decode('utf-8', errors='replace').strip()
        print(f'  [FAIL] {name}')
        if out: print(f'         stdout: {out[:200]}')
        if err: print(f'         stderr: {err[:200]}')

print(f'\nResult: {passed}/{len(tests)} passed')
if failed:
    print('Failed:', ', '.join(failed))
    sys.exit(1)
