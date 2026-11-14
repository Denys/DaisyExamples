"""Verify the frozen external build inputs; never refresh pins silently."""
from pathlib import Path
import hashlib,json,sys
root=Path(__file__).resolve().parent
items=json.loads((root/'dependencies.json').read_text())['files']
errors=[]
for row in items:
    p=Path(row['path'])
    if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=row['sha256']:
        errors.append(row['path'])
if errors:
    print('DEPENDENCY_DRIFT_OR_UNAVAILABLE:\n'+'\n'.join(errors));sys.exit(1)
print(f'DEPENDENCIES: {len(items)} exact local input hashes match')
