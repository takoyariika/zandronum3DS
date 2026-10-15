"""Attach devkitARM GDB to the test Citra stub and capture a stopped stack."""
import pathlib
import queue
import re
import subprocess
import threading
import time
import json
ROOT = pathlib.Path(__file__).resolve().parents[1]
sdk = pathlib.Path(json.loads((ROOT/'build-3ds/build-config.json').read_text())['devkitpro'])
gdb = sdk/'devkitARM/bin/arm-none-eabi-gdb.exe'
p = subprocess.Popen([str(gdb), '--interpreter=mi2', '--quiet', str(ROOT/'build-3ds/dist/zandronum.elf')], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
q = queue.Queue()
lines = []
def reader():
    for line in p.stdout:
        lines.append(line)
        q.put(line)
threading.Thread(target=reader, daemon=True).start()
def command(text):
    p.stdin.write(text+'\n'); p.stdin.flush()
def wait_for(text, timeout=10):
    end = time.time()+timeout
    while time.time()<end:
        try: line=q.get(timeout=.2)
        except queue.Empty: continue
        if text in line: return line
    raise RuntimeError('GDB timed out waiting for '+text)
try:
    command('1-gdb-set mi-async on'); wait_for('1^done')
    command('2-target-select remote 127.0.0.1:24689'); wait_for('2^connected')
    command('3-exec-continue'); wait_for('3^running')
    time.sleep(12)
    command('4-exec-interrupt'); wait_for('*stopped')
    command('5-stack-list-frames'); print(wait_for('5^done'))
    command('6-data-list-register-values x 13 14 15'); registers = wait_for('6^done'); print(registers)
    lr = re.search(r'number="14",value="(0x[0-9a-f]+)"', registers)
    sp = re.search(r'number="13",value="(0x[0-9a-f]+)"', registers)
    if lr:
        command('7-interpreter-exec console "info symbol '+lr.group(1)+'"'); wait_for('7^done')
        command('8-interpreter-exec console "x/12i '+hex(int(lr.group(1), 16)-32)+'"'); wait_for('8^done')
    if sp:
        command('9-data-read-memory-bytes '+sp.group(1)+' 96'); print(wait_for('9^done'))
finally:
    (ROOT/'build-3ds/gdb-citra.log').write_text(''.join(lines))
    command('-gdb-exit')
    try: p.wait(timeout=3)
    except subprocess.TimeoutExpired: p.kill()
