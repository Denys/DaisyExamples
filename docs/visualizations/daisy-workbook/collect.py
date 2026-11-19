"""Freeze a complete, bounded local Daisy source corpus. No network or mutations to firmware."""
from __future__ import annotations
import argparse
import base64
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
BOARDS = ['seed', 'pod', 'field', 'patch', 'patch_sm', 'pedal', 'petal', 'versio', 'legio']
SKIP = {'.git', '.tmp', '.worktrees', 'build', 'dist', 'node_modules', '__pycache__',
        '.pytest_cache', '.mypy_cache', '.cache', '.vs', '.idea', '.vscode', '.obsidian',
        'noderr', 'noderr-main', '_Vault', 'Debug', 'Release', 'third_party', 'third-party', 'vendor', 'vendors',
        'JUCE', 'googletest', 'GoogleTest', 'CMSIS', 'STM32H7xx_HAL_Driver', 'Middlewares',
        'Drivers', 'resources', 'graphify-out', '.agents', '.codex', 'cmake-build-debug',
        'logs', 'log', 'datasets', 'dataset', 'recordings', 'training_data'}
CODE = {'.cpp', '.cc', '.c', '.cxx', '.h', '.hpp', '.hh', '.inl', '.tpp', '.ino', '.s', '.S', '.lds', '.ioc', '.dvpe'}
DOCNAMES = {'readme.md', 'readme.txt', 'controls.md', 'checkpoint.md', 'changelog.md',
            'license', 'license.md', 'license.txt', 'copying', 'copying.md', 'field_defaults_readme.md',
            'field_defaults_usage.md', 'field_display_project_readme_template.md'}
TOOLS = {'.py', '.ps1', '.sh', '.cmd', '.bat', '.cmake'}

def ident(s):
    stem = re.sub('[^a-z0-9]+', '-', s.lower()).strip('-')[:90]
    return stem + '-' + hashlib.sha256(s.encode()).hexdigest()[:8]

def git(*args):
    p = subprocess.run(['git', *args], cwd=ROOT, capture_output=True, encoding='utf-8', errors='replace')
    return p.stdout.strip()

def walks(base, excluded):
    if not base.exists():
        return []
    out = []
    for parent, dirs, files in os.walk(base):
        rejected = [d for d in dirs if d in SKIP or d.startswith('build-') or d.startswith('.')]
        excluded.extend(str((Path(parent)/d).relative_to(ROOT)).replace('\\','/') for d in rejected)
        dirs[:] = sorted(d for d in dirs if d not in rejected)
        out.extend(Path(parent)/f for f in sorted(files))
    return out

def plain(text):
    text = re.sub(r'```[\s\S]*?```', '', text)
    text = re.sub(r'<[^>]+>', '', text)
    text = re.sub(r'!\[[^]]*\]\([^)]*\)', '', text)
    text = re.sub(r'\[([^]]*)\]\([^)]*\)', r'\1', text)
    text = re.sub(r'[*`#_]', '', text)
    return re.sub(r'\s+', ' ', text).strip()

def first_description(text):
    chunks = re.split(r'\n\s*\n', text)
    for chunk in chunks:
        if re.match(r'\s*(#|\||```|!\[|<|\[!|---|[-*] |[0-9]+\.)', chunk):
            continue
        p = plain(chunk)
        if len(p) >= 32 and not p.startswith(('http', 'Copyright', 'SPDX', '/*', '//')):
            return p[:950]
    return ''

def code_description(text):
    """Read only introductory comments, never mistake executable statements for prose."""
    lines=[]
    for line in text.splitlines()[:45]:
        t=line.strip()
        if t.startswith('//'):
            t=t[2:].strip().lstrip('#').strip()
            if t and not re.match(r'(?:Author|Copyright|SPDX|\w+\.cpp\b)',t,re.I): lines.append(t)
        elif t and not t.startswith(('/*','*','*/')):
            break
        elif t.startswith('*'):
            t=t.lstrip('*').strip()
            if len(t)>20 and not t.startswith(('@','Copyright','SPDX')): lines.append(t)
    return ' '.join(lines)[:950]

def features(text):
    names = [('DelayLine', 'linee di ritardo'), ('ReverbSc', 'riverbero ReverbSc'),
             ('PitchShifter', 'trasposizione di altezza'), ('Oscillator', 'oscillatori'),
             ('WhiteNoise', 'rumore bianco'), ('Svf', 'filtro a variabili di stato'),
             ('AdEnv', 'inviluppi AD'), ('Adsr', 'inviluppi ADSR'), ('Compressor', 'compressore'),
             ('Overdrive', 'saturazione'), ('Chorus', 'chorus'), ('Flanger', 'flanger'),
             ('Decimator', 'decimazione'), ('Looper', 'looper'), ('Metro', 'clock Metro'),
             ('MidiHandler', 'gestione MIDI'), ('SdmmcHandler', 'accesso SD/MMC'),
             ('UsbHandle', 'interfaccia USB'), ('STFT', 'elaborazione STFT'),
             ('FFT', 'trasformata FFT'), ('Sequencer', 'sequenziamento')]
    return [desc for name, desc in names if re.search(r'\b'+name+r'\b', text)]

def collect(date):
    excluded, groups, allfiles = [], {}, {}
    def group(key, label, files):
        groups[key] = {'title': label, 'paths': sorted(set(files), key=lambda p:p.as_posix())}
        for p in groups[key]['paths']:
            allfiles[p] = key
    for b in BOARDS:
        group(b, {'patch_sm':'Patch SM'}.get(b,b.title()), walks(ROOT/b, excluded))
    for key, path, title in [
        ('custom', 'MyProjects/_projects', 'Progetti custom'),
        ('experiments', 'MyProjects/_experiments', 'Esperimenti'),
        ('concepts', 'MyProjects/_concepts', 'Concetti e bozze'),
        ('foundation', 'MyProjects/foundation_examples', 'Fondamenti e helper'),
        ('host', 'DaisyHost', 'DaisyHost'), ('dafx', 'DaisyDAFX', 'DaisyDAFX'),
        ('daisysp', 'DaisySP', 'DaisySP'), ('libdaisy', 'libDaisy', 'libDaisy'),
        ('archive', 'MyProjects/DAFX_2_Daisy_lib', 'DAFX archivio'),
        ('other-libs', 'MyProjects/Other_LIB_dev', 'Altre librerie custom'),
        ('backup', 'MyProjects/field_wavetable_morph_synth_backup', 'Backup wavetable'),
        ('qae', 'DAISY_QAE', 'Qualità e strumenti'), ('ci', 'ci', 'CI e helper'),
        ('cube', 'cube', 'STM32Cube'), ('dvpe', 'DVPE', 'DVPE e guide')]:
        group(key, title, walks(ROOT/path, excluded))
    # Entire library source roots include LGPL, core/startup, examples and tests;
    # generated/vendor directories remain excluded by the same bounded rules.
    standards = ROOT/'DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md'
    if standards.exists(): groups['qae']['paths'].append(standards)
    groups['ci']['paths'] += [ROOT/'helper.py', ROOT/'rebuild_all.sh', ROOT/'README.md', ROOT/'LICENSE']
    sources = {}
    def add(p):
        rel = p.relative_to(ROOT).as_posix()
        if rel in sources:
            return sources[rel]
        raw = p.read_bytes()
        try:
            text = raw.decode('utf-8-sig')
            encoding = 'utf-8'
        except UnicodeDecodeError:
            text = raw.decode('cp1252', errors='replace')
            encoding = 'cp1252'
        # Secrets are not expected in allowlisted code/docs; stop if literal credentials appear.
        if re.search(r'(?:sk-proj-|ghp_|github_pat_)[A-Za-z0-9_\-]{20,}', text):
            raise ValueError('Credential-like literal in allowlisted source: '+rel)
        sid = 'src-'+ident(rel)
        record = {'id':sid, 'title':rel, 'type':'local-source', 'path':rel, 'status':'VERIFIED',
            'authority':'Local source snapshot', 'sha256':hashlib.sha256(raw).hexdigest(),
            'bytes':len(raw), 'lines':len(text.splitlines()), 'encoding':encoding, 'text':text,
            'url':'code/'+sid+'.html', 'retrieved_date':date,
            'note':'Contenuto locale congelato; la presenza del codice non prova compilazione o comportamento hardware.'}
        if text.encode('utf-8') != raw:
            record['raw_base64'] = base64.b64encode(raw).decode('ascii')
        sources[rel] = record
        return record
    entries, retained, uncatalogued = [], [], []
    for key, g in groups.items():
        files = sorted(set(p for p in g['paths'] if p.is_file() and (p.suffix in CODE or p.name.lower() in DOCNAMES or p.name in {'Makefile','CMakeLists.txt'} or p==standards or (key in {'ci','qae','dafx','host','other-libs','libdaisy','daisysp'} and p.suffix in TOOLS) or (key=='dvpe' and p.suffix=='.md'))))
        if not files:
            continue
        retained.extend(p.relative_to(ROOT).as_posix() for p in files)
        manifests = sorted({p.parent for p in files if p.name in {'Makefile','CMakeLists.txt'}})
        owners = {}
        for p in files:
            rel = p.relative_to(ROOT)
            if key in {'custom','experiments','concepts'}:
                owner = Path(*rel.parts[:3]) if len(rel.parts)>3 else p.parent.relative_to(ROOT)
            elif key in {'host','dafx','archive','qae','ci','foundation','other-libs','backup','dvpe'}:
                owner = Path(key) # synthetic workspace identity replaced below
            elif key in {'daisysp','libdaisy'}:
                # Paired .cpp/.h module records; root manifests/docs form a support entry.
                owner = Path(*rel.parts[:-1]) / p.stem if p.suffix in CODE else Path(rel.parts[0])/'support'
            else:
                candidates = [m for m in manifests if m==p.parent or m in p.parents]
                owner = max(candidates, key=lambda x:len(x.parts)).relative_to(ROOT) if candidates else p.parent.relative_to(ROOT)
            owners.setdefault(owner.as_posix(), []).append(p)
        for owner, owned in sorted(owners.items()):
            records = [add(p) for p in owned]
            codes = sorted([s for s in records if Path(s['path']).suffix in CODE], key=lambda s:(len(Path(s['path']).parts),Path(s['path']).suffix not in {'.cpp','.cc','.c'},s['path']))
            docs = [s for s in records if Path(s['path']).name.lower() in DOCNAMES or (key=='dvpe' and s['path'].endswith('.md'))]
            builds = [s for s in records if Path(s['path']).name in {'Makefile','CMakeLists.txt'}]
            if not codes and not docs and not builds and key not in {'ci','qae'}:
                uncatalogued.extend(s['path'] for s in records)
                continue
            title = Path(owner).name
            if key in {'host','dafx','archive','qae','ci','foundation','other-libs','backup','dvpe'}:
                title = g['title']
                owner = str(Path(os.path.commonpath([p.parent for p in owned])).relative_to(ROOT)).replace('\\','/')
            entry_id = 'project-'+ident(key+'/'+owner)
            readme = next(iter(sorted((s for s in docs if Path(s['path']).name.lower().startswith('readme')),key=lambda s:len(Path(s['path']).parts))),None)
            primary = next((s for s in codes if Path(s['path']).suffix in {'.cpp','.cc','.c'}),codes[0] if codes else None)
            lead = readme or (primary or (docs[0] if docs else records[0]))
            desc = first_description(lead['text']) if readme or lead in docs else code_description(lead['text'])
            active_codes=[s for s in codes if '/archive/' not in s['path'].lower() and '/backup/' not in s['path'].lower()]
            joined = '\n'.join(s['text'] for s in active_codes)
            feats = features(joined)
            classes = sorted(set(re.findall(r'\b(?:class|struct)\s+([A-Za-z_]\w*)', joined)))[:45]
            includes = sorted(set(re.findall(r'#\s*include\s*[<"]([^>"\n]+)', joined)))
            callbacks = sorted(set(re.findall(r'\b(\w*(?:AudioCallback|Callback|ProcessControls|UpdateControls|UpdateOled|UpdateDisplay|ProcessMidi)\w*)\s*\(', joined)))[:30]
            controls = sorted(set(re.findall(r'\b(?:hw|hardware|daisy|pod|field|patch|petal)\.(knob\w*|encoder|button\w*|key\w*|gate\w*|cv\w*|led\w*|sw\w*)', joined)))
            board_matches = sorted(set(re.findall(r'\bDaisy(Seed|Pod|Field|PatchSM|PatchSm|Patch|Petal|Versio|Legio|Pedal)\b', joined)))
            board = ', '.join(board_matches) or (g['title'] if key in BOARDS else 'Non dichiarata / host')
            if key in {'libdaisy','daisysp','dafx','archive','other-libs'}:
                board = 'Libreria'
            if key == 'host': board = 'Host desktop'
            if not desc:
                if feats:
                    desc = 'Il sorgente contiene '+', '.join(feats)+'. Consultare callback, classi e include elencati per seguire l’implementazione.'
                elif classes:
                    desc = 'Il modulo dichiara '+', '.join(classes[:8])+'. La scheda espone sorgenti e dipendenze; funzione musicale non descritta nella documentazione disponibile.'
                else:
                    desc = 'Voce censita da sorgenti e documenti locali. Scopo non descritto in un README leggibile: consultare il codice e i manifest allegati.'
            summary = 'Documento: '+desc if readme else 'Dal codice / documenti: '+desc
            build_text = '\n'.join(s['text'] for s in builds)
            flags = sorted(set(re.findall(r'^\s*(TARGET|LIBDAISY_DIR|DAISYSP_DIR|USE_DAISYSP_LGPL|USE_FATFS|USE_USBHOST|USE_CMSIS_DSP|OPT|CPP_STANDARD)\s*[:?+]?=\s*([^\n#]+)', build_text, re.M)))
            issues = []
            if not readme: issues.append('README non presente in questa voce.')
            if not any(Path(s['path']).name.lower()=='checkpoint.md' for s in docs): issues.append('CHECKPOINT locale non presente; milestone non dedotta.')
            if any(re.search(r'^<{7}|^={7}$|^>{7}', s['text'], re.M) for s in docs): issues.append('Marcatori di conflitto presenti nei documenti; interpretazione da verificare.')
            if key in {'archive','backup'}: issues.append('Copia archivio/backup: preferire il workspace canonico per nuovi sviluppi.')
            if not codes: issues.append('Nessun sorgente firmware C/C++ in questa voce; documentazione/concept/tooling.')
            entries.append({'id':entry_id, 'title':title, 'path':owner, 'group':key, 'board':board,
                'description':summary, 'description_source_id':lead['id'], 'description_label':'VERIFIED' if readme and first_description(readme['text']) else 'DERIVED',
                'primary_source_id':primary['id'] if primary else None,
                'source_ids':[s['id'] for s in records], 'code_ids':[s['id'] for s in codes],
                'doc_ids':[s['id'] for s in docs], 'build_ids':[s['id'] for s in builds],
                'features':feats, 'includes':includes, 'classes':classes, 'callbacks':callbacks,
                'controls':controls, 'build_flags':flags, 'issues':issues,
                'validation':{'source':'VERIFIED', 'build':'NOT_RUN','host_tests':'NOT_RUN','hardware':'NOT_RUN'},
                'code_files':len(codes), 'source_lines':sum(s['lines'] for s in codes)})
    records = sorted(sources.values(), key=lambda s:s['path'])
    coverage = {'retained_files':len(set(retained)), 'snapshot_files':len(records),
        'uncatalogued_files':uncatalogued, 'excluded_directories':sorted(set(excluded)),
        'roots':{k:{'title':v['title'], 'discovered_files':len(v['paths']), 'entries':sum(e['group']==k for e in entries)} for k,v in groups.items()},
        'scope':'Every allowlisted source/manifests/doc file in the declared Daisy roots. Not every byte in the mixed workspace.',
        'exclusions':['Build outputs, binaries, resources/media, logs/datasets, credentials and agent state.',
            'Vendor code in nested third_party/Drivers/CMSIS/JUCE and root stmlib/third_party.',
            'DaisyTheory textbook/Teensy/STK corpora and MyProjects/TeensyAudio: supporting non-Daisy repositories, not Daisy firmware.',
            'Full API semantics, hardware qualification and historic test re-execution.']}
    coverage['directories_without_eligible_project_sources'] = [p.relative_to(ROOT).as_posix() for root in ['MyProjects/_projects','MyProjects/_experiments','MyProjects/_concepts'] if (ROOT/root).exists() for p in sorted((ROOT/root).iterdir()) if p.is_dir() and not any(s['path'].startswith(p.relative_to(ROOT).as_posix()+'/') for s in records)]
    result = {'date':date, 'repo_head':git('rev-parse','HEAD'), 'branch':git('branch','--show-current'),
        'repo_remote':git('remote','get-url','origin'), 'dirty_worktree':bool(git('status','--porcelain','--untracked-files=normal')),
        'submodules':git('submodule','status'), 'entries':entries, 'sources':records, 'coverage':coverage}
    return result

if __name__=='__main__':
    ap=argparse.ArgumentParser(); ap.add_argument('--date', default='2026-10-04'); ap.add_argument('--output', type=Path, default=HERE/'src'/'inventory.json'); args=ap.parse_args()
    data=collect(args.date); args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(data, ensure_ascii=False, indent=2)+'\n', encoding='utf-8', newline='\n')
    print(json.dumps({'entries':len(data['entries']), 'sources':len(data['sources']), 'bytes':args.output.stat().st_size, 'groups':dict(Counter(e['group'] for e in data['entries'])), 'coverage':{k:v for k,v in data['coverage'].items() if k not in {'roots','excluded_directories'}}}, ensure_ascii=False))
