"""Repackage the verified local candidate; no network or source-tree copying."""
from pathlib import Path
import hashlib,json,subprocess,sys,zipfile
root=Path(__file__).resolve().parent
subprocess.run([sys.executable,'-B',str(root/'verify_dependencies.py')],check=True)
names=['README.md','CONTRACT.md','Pod_MultiDelay.dvpe','Pod_MultiDelay.cpp','Demo.h','Controls.h','DigiMono.h','Makefile','test_digi.cpp','test_demo.cpp','test_controls.cpp','verify.ps1','verify_dependencies.py','package.py','dependencies.json','verification.log','qae.log','build/Pod_MultiDelay.bin','build/Pod_MultiDelay.elf','build/Pod_MultiDelay.map','build/Pod_MultiDelay.hex']
names += ['HARDWARE_TEST.md']
names += ['diagnostic/Makefile','diagnostic/Pod_MultiDelay_Test.cpp','diagnostic/Pod_MultiDelay_Test.dvpe']
names += ['diagnostic/build/Pod_MultiDelay_Test.'+ext for ext in ['bin','elf','hex','map']]
names += ['hardware_evidence/20260909/'+name for name in ['TESTPLAN.md','telemetry.bin','telemetry.json','telemetry_liveness.bin','telemetry_liveness.json','timings.bin','parse_telemetry.py','verify_evidence.py','diagnostic_build.log','diagnostic_flash.log','normal_flash.log','normal_boot.log','normal_boot_verified.log','capture.log','capture_liveness.log']]
manifest={'version':'0.1.0','board':'DaisyPod','sample_rate':48000,'block_size':48,'app_type':'BOOT_NONE','hardware_validation':'POD_DIGITAL_SCREEN_PASS','normal_image':'FLASH_READBACK_VERIFIED_CALLBACK_REACHED','physical_audio_controls':'NOT_RUN','files':[]}
for name in names:
    data=(root/name).read_bytes()
    manifest['files'].append({'path':name,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
(root/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
with zipfile.ZipFile(root/'Pod_MultiDelay_v0_1.zip','w',compression=zipfile.ZIP_DEFLATED) as z:
    for name in names+['manifest.json']: z.write(root/name,name)
with zipfile.ZipFile(root/'Pod_MultiDelay_v0_1.zip') as z:
    assert z.testzip() is None
    for row in manifest['files']: assert hashlib.sha256(z.read(row['path'])).hexdigest()==row['sha256']
print(f'PACKAGE: {len(names)} files plus manifest, archive hashes verified')
