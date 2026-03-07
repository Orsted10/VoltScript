import subprocess, tempfile, os

exe = r'e:\ClawScript\build\bin\Release\claw.exe'

# Read the actual test file and bisect it
with open(r'e:\ClawScript\examples\stdlib\regex_test.claw', encoding='utf-8') as f:
    all_lines = f.readlines()

print(f"Total lines: {len(all_lines)}")

for i in range(1, len(all_lines)+1):
    script = ''.join(all_lines[:i])
    fd, fname = tempfile.mkstemp(suffix='.claw')
    try:
        with os.fdopen(fd, 'w', encoding='utf-8') as f:
            f.write(script)
        r = subprocess.run([exe, fname], capture_output=True, text=True, 
                          cwd=r'e:\ClawScript', timeout=10)
        if r.returncode != 0:
            print(f'CRASH at line {i}: {all_lines[i-1].rstrip()}')
            print(f'  RC: {r.returncode}')
            for l in r.stderr.splitlines()[:5]:
                print(f'  {l}')
            break
        else:
            print(f'OK  line {i:3d}: {all_lines[i-1].rstrip()[:60]}')
    finally:
        try: os.unlink(fname)
        except: pass
