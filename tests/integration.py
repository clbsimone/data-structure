"""Deterministic CLI regression checks; fixtures live in a temporary directory."""
from pathlib import Path
import random
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def run(exercise, *args, success=True):
    result = subprocess.run([str(ROOT / f'es{exercise}_ASDLab/bin/main_ex{exercise}'),
                             *map(str, args)], capture_output=True, text=True, timeout=10)
    assert (result.returncode == 0) == success, (exercise, args, result.stderr)
    return result.stdout

with tempfile.TemporaryDirectory() as directory:
    tmp = Path(directory)
    source, output = tmp / 'input', tmp / 'output'
    rng = random.Random(42)
    records = [(i, rng.uniform(-100, 100), rng.randrange(-1000, 1000),
                f'word{rng.randrange(10)}'.encode()) for i in range(150)]
    rng.shuffle(records)
    source.write_bytes(b''.join(struct.pack('<Qfq16s', *r) for r in records))
    decoded = list(struct.iter_unpack('<Qfq16s', source.read_bytes()))
    for field in range(1, 5):
        for threshold in [0, 1, 10, 200]:
            run(1, source, output, field, threshold)
            actual = list(struct.iter_unpack('<Qfq16s', output.read_bytes()))
            assert sorted(actual) == sorted(decoded)
            assert [r[field-1] for r in actual] == sorted(r[field-1] for r in decoded)
    run(1, source, output, 'no', 10, success=False)
    run(1, source, output, 1, '-1', success=False)
    source.write_bytes(b'')
    run(1, source, output, 1, 0)
    assert output.read_bytes() == b''
    source.write_bytes(b'bad')
    run(1, source, output, 1, 0, success=False)
    source.write_text('Beta alpha! BETA alpha; longer')
    assert run(2, source, 1).strip() == "'alpha' 2"
    assert run(2, source, 5).strip() == "'alpha' 2"
    run(2, source, 'abc', success=False)
    source.write_text('3,1,2,9\n1,0,3,1\n2,1,1,9\n4,20,1,0\n')
    run(3, source, output)
    assert output.read_text() == '1,0,3\n2,3,4\n3,4,6\n4,20,21\n'
    for text in ['invalid\n', '1,0,2\n', '1,0,-1,2\n', '1,0,2,3,4\n', '1,999999999999999,2,3\n']:
        source.write_text(text)
        run(3, source, output, success=False)
    source.write_text('1,2147483647,2147483647,1\n')
    run(3, source, output)
    assert output.read_text() == '1,2147483647,4294967294\n'
    source.write_text('a,b,1\na,c,2\nb,d,3\ne,f,4\na,a,0\n')
    run(4, source, 'A', output)
    order = output.read_text().splitlines()
    assert order[0] == 'a' and set(order[1:3]) == {'b', 'c'} and order[3:] == ['d']
    run(4, source, 'missing', output, success=False)
print('CLI regression checks passed (sorting, word counts, scheduling, and BFS).')
