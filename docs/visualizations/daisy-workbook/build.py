from __future__ import annotations
import argparse, base64, hashlib, html, importlib.util, json, os, re, shutil, sys
from collections import Counter
from pathlib import Path
from urllib.parse import quote

HERE = Path(__file__).resolve().parent
SKILL = Path(os.environ.get('HTML_HYPER_DOC_SKILL', str(Path.home()/'.codex/skills/html-hyper-doc')))
spec = importlib.util.spec_from_file_location('hyperdoc', SKILL/'scripts/build_hyper_doc.py')
base = importlib.util.module_from_spec(spec); spec.loader.exec_module(base)
E = lambda x: html.escape(str(x), quote=True)
write = base.write_text

LABELS = {'seed':'Seed','pod':'Pod','field':'Field','patch':'Patch','patch_sm':'Patch SM','pedal':'Pedal','petal':'Petal','versio':'Versio','legio':'Legio', 'custom':'Progetti custom','experiments':'Esperimenti','concepts':'Concetti','foundation':'Fondamenti e helper','host':'DaisyHost','dafx':'DaisyDAFX','daisysp':'DaisySP','libdaisy':'libDaisy','archive':'DAFX archivio','other-libs':'Altre librerie custom','backup':'Backup wavetable','qae':'Qualità e strumenti','ci':'CI e helper','cube':'STM32Cube','dvpe':'DVPE e guide'}
BOUNDARY = 'Ispezione statica del sorgente e dei documenti. Build, host test, audio e hardware non eseguiti in questa sessione.'


def make_corpus(inv):
    entries, sources = inv['entries'], inv['sources']
    chapters = []
    for key, label in LABELS.items():
        es = [e for e in entries if e['group']==key]
        if not es: continue
        sections=[]
        for e in es:
            claim = {'id':'claim-'+e['id'], 'label':e['description_label'], 'text':e['description'], 'source_ids':[e['description_source_id']], 'locator':e['path'], 'boundary':BOUNDARY}
            facts = {'id':'facts-'+e['id'], 'label':'VERIFIED', 'text':f"Inventario: {e['code_files']} file di codice; {e['source_lines']} righe. Include, simboli e controlli sono estratti con espressioni regolari e vanno letti nel contesto del sorgente.", 'source_ids':e['source_ids'][:5], 'locator':e['path'], 'boundary':'Conteggi di file/righe, non misure di prestazioni; simboli estratti non sono analisi completa del flusso.', 'numeric':[{'value':e['code_files'],'unit':'file','condition':'source snapshot'},{'value':e['source_lines'],'unit':'righe','condition':'line count, includes comments'}]}
            sections.append({'id':e['id'], 'title':e['title'], 'entry':e, 'blocks':[{'type':'claims','claims':[claim,facts]}]})
        chapters.append({'id':key.replace('_','-'), 'title':label, 'summary':f"{len(es)} voci nel gruppo {label}. Descrizioni, sorgenti originali, dipendenze e documentazione locale.", 'learning_outcomes':['Trovare il codice e la documentazione di ogni voce.','Distinguere presenza del sorgente, interpretazione e prove eseguite.'], 'sections':sections})
    intro = {'id':'guida', 'title':'Come usare il workbook', 'summary':'Copertura, percorso di lettura e limiti delle evidenze.', 'sections':[
        {'id':'ambito','title':'Ambito e fonti', 'blocks':[{'type':'paragraph','text':'Questo workbook fotografa il checkout locale DaisyExamples. Non è un elenco universale di tutti i repository Daisy su Internet. Include ogni file ammesso nei percorsi dichiarati, compresi esempi, custom, librerie canoniche, esperimenti, tooling e copie archivio. Codice originale e documentazione mantengono la propria lingua.'},{'type':'diagram','visual_id':'repository-map'}]},
        {'id':'leggere','title':'Dalla scheda al codice', 'blocks':[{'type':'steps','items':['Cerca un progetto, un modulo o un simbolo nella barra superiore. Scegli il gruppo e il tipo di risultato.','Apri la scheda per leggere descrizione, scheda target, dipendenze, callback e controlli riconoscibili nel codice.','Apri README, CONTROLS e CHECKPOINT originali per le spiegazioni estese. Le loro prove sono storiche, senza rinnovo automatico.','Apri qualsiasi sorgente nel lettore offline: numeri di riga, copia e download. Il checksum identifica i byte congelati.','Prima di modificare o compilare, confronta il file vivo con lo snapshot. Esegui la build dal progetto e consulta le istruzioni locali.']}]},
        {'id':'evidenze','title':'Cosa significano le etichette', 'blocks':[{'type':'table','columns':['Etichetta','Significato nel workbook'], 'rows':[['VERIFIED','File letto e contenuto congelato; non equivale a firmware funzionante.'],['DERIVED','Sintesi da simboli, include e struttura statica; può richiedere lettura manuale.'],['NOT_RUN','Build, test host, audio, flashing e hardware non eseguiti.'],['UNVERIFIED','Scopo non documentato o affermazione storica non rivalidata.']]},{'type':'paragraph','text':'Le descrizioni che iniziano con Documento citano un estratto del README. I gruppi senza README usano una sintesi dal codice, esplicitamente distinta. Simboli e controlli estratti da regex sono indizi di lettura: commenti, rami condizionali e codice non raggiungibile possono comparire.'}]},
        {'id':'copertura','title':'Copertura e confini', 'blocks':[{'type':'bullets','items':inv['coverage']['exclusions']},{'type':'paragraph','text':'I sorgenti musicali importati che fanno parte dei progetti restano inclusi dove non sono in una directory esclusa. Attribuzioni e licenze sono conservate nei file originali. Il nome di una directory non dimostra origine upstream o maturità del firmware.'}]},
        {'id':'build','title':'Ricette di verifica', 'blocks':[{'type':'paragraph','text':'Ricette dalla guida del repository. Sono istruzioni consultabili, non comandi eseguiti per tutti i programmi censiti. Il workbook mostra la procedura rilevante per ogni voce; l’hardware richiede una verifica separata.'},{'type':'code','text':'# Firmware: eseguire dalla directory del progetto\nmake\n\n# DaisyDAFX (dalla root del repository)\ncmake -S DaisyDAFX -B DaisyDAFX/build -DBUILD_EXAMPLES=OFF\ncmake --build DaisyDAFX/build --config Release --target unit_tests\nctest --test-dir DaisyDAFX/build -C Release --output-on-failure\n\n# DaisyHost (dalla root del repository)\ncmake -S DaisyHost -B DaisyHost/build\ncmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone\nctest --test-dir DaisyHost/build -C Release --output-on-failure'},{'type':'paragraph','text':'ci/build_examples.py copre gli esempi board e non i progetti MyProjects. Le schede delle librerie non sono target firmware autonomi. Flashing hardware non fa parte della generazione del workbook.'}]}
    ]}
    chapters.insert(0,intro)
    sources.append({'id':'inventory-freeze', 'title':'Audit di copertura e freeze', 'type':'inventory', 'path':'coverage.json', 'url':'../coverage.json', 'status':'VERIFIED', 'authority':'Deterministic local scanner', 'note':'Radici dichiarate, conteggi, esclusioni e file senza voce.'})
    # Store contextual metadata without other chats, private logs, or full dirty-worktree path lists.
    doc={'id':'daisy-workbook','title':'Daisy Workbook','subtitle':'L’intero workspace, dal primo esempio al progetto complesso. Codice e documentazione in un riferimento offline.', 'version':'v001', 'build_date':inv['date'], 'source_freeze':inv['repo_head'][:12]+' + snapshot locale con SHA-256 per file', 'status':'SOURCE_SNAPSHOT / build e hardware NOT_RUN'}
    sid=sources[0]['id']
    visual={'id':'repository-map','title':'Dove cercare nel workspace','kind':'mermaid','asset':'repository-map.svg','source':'repository-map.md','alt':'Gerarchia dei nove gruppi hardware, dei progetti custom, delle librerie e degli strumenti inclusi nello snapshot.', 'caption':'Percorsi hardware, applicativi e di libreria restano distinti. La copertura dettagliata è nell’audit JSON.', 'reader_job':'Scegliere la famiglia da consultare prima di aprire un sorgente.', 'label':'VERIFIED','source_ids':['inventory-freeze'],'locator':'coverage.json → roots','boundary':'Mappa di cartelle e contenuti; non descrive connessioni audio o dipendenze runtime.', 'layout':'standard'}
    return {'schema_version':'1.0','document':doc,'chapters':chapters,'entries':entries,'sources':sources,'coverage':inv['coverage'], 'snapshot':{k:inv[k] for k in ['repo_head','branch','repo_remote','dirty_worktree','submodules']}, 'visual_root':'visuals','visuals':[visual], 'dashboard_visuals':['repository-map'], 'metrics':[{'label':'Voci','value':len(entries),'detail':'Programs, modules and support workspaces','label_state':'VERIFIED'},{'label':'Sorgenti e documenti','value':len(sources)-1,'detail':'Frozen local source files','label_state':'VERIFIED'}], 'glossary':[{'id':'glossary-callback','term':'Callback audio','definition':'Funzione richiamata dal motore audio per elaborare i buffer.','engineer_translation':'Leggere dimensione dei blocchi, accesso allo stato e costo per campione nel sorgente.'},{'id':'glossary-target','term':'Target','definition':'Programma o libreria che il sistema di build può compilare.','engineer_translation':'TARGET nei Makefile o add_executable/add_library in CMake; non dedotto dal nome della cartella.'},{'id':'glossary-snapshot','term':'Snapshot','definition':'Copia datata dei contenuti consultati.','engineer_translation':'Il riferimento Git da solo non identifica le modifiche locali: ogni file ha il proprio SHA-256.'},{'id':'glossary-sdram','term':'SDRAM','definition':'Memoria esterna usata da molti programmi per buffer audio ampi.','engineer_translation':'Verificare sezioni di memoria, inizializzazione e comportamento sul target prima di dedurre capacità o tempi.'}], 'limitations':[{'label':'NOT_RUN','text':BOUNDARY},{'label':'DERIVED','text':'Simboli, include, callback e controlli sono estratti staticamente; nessuna promessa di analisi semantica completa.'},{'label':'UNVERIFIED','text':'Prove storiche nei documenti e attribuzione upstream non rivalidate. Il workbook è un inventario locale, non una graduatoria di maturità.'}]}


def emit_visuals(out):
    out.mkdir(parents=True,exist_ok=True)
    write(out/'repository-map.md', '# Repository scope\n\n```mermaid\nflowchart TB\n  R[DaisyExamples snapshot locale] --> B[Esempi hardware: 9 piattaforme]\n  R --> C[MyProjects: custom / esperimenti / fondamenti]\n  R --> L[DaisyHost / DaisyDAFX / DaisySP / libDaisy]\n  R --> T[QAE / CI / Cube / DVPE / archivio]\n```\n\nSource: coverage.json → roots. Boundary: filesystem hierarchy, no runtime topology.\n')
    boxes=[('Hardware','Seed · Pod · Field','Patch · Patch SM','Pedal · Petal · Versio · Legio'),('Progetti','MyProjects/_projects','Esperimenti e fondamenti','Concetti con fonti disponibili'),('Librerie','DaisyHost · DaisyDAFX','DaisySP · libDaisy','Moduli e codice completo'),('Supporto','QAE · CI · helper','Cube · DVPE','Archivio e backup distinti')]
    svg=['<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1000 345" role="img"><title>Gerarchia del workspace Daisy</title><rect width="1000" height="345" fill="#ffffff"/><g font-family="Segoe UI, sans-serif" fill="#292825"><text x="36" y="43" font-size="24" font-weight="600">DaisyExamples / snapshot locale</text><path d="M40 66H960 M145 66V103 M382 66V103 M619 66V103 M856 66V103" stroke="#b3aca1" fill="none"/>']
    for i,(title,a,b,c) in enumerate(boxes):
        x=30+i*240
        svg.append(f'<rect x="{x}" y="103" width="222" height="203" rx="8" fill="#fafaf9" stroke="#e0ddd5"/><text x="{x+16}" y="139" font-size="20" font-weight="600" fill="#a34228">{E(title)}</text><text x="{x+16}" y="181" font-size="13">{E(a)}</text><text x="{x+16}" y="210" font-size="13">{E(b)}</text><text x="{x+16}" y="239" font-size="12">{E(c)}</text>')
    svg.append('</g></svg>'); write(out/'repository-map.svg',''.join(svg))


def root_nav(d,current,p):
    links=[('index.html','Catalogo','dashboard'),('chapters/guida.html','Guida e copertura','guida')]
    links += [('chapters/'+c['id']+'.html',c['title'],c['id']) for c in d['chapters'] if c['id']!='guida']
    links += [('sources.html','Fonti e glossario','sources')]
    return ''.join(f'<a href="{p+url}"'+(' aria-current="page"' if current==ident else '')+f'>{E(name)}</a>' for url,name,ident in links)


def shell(d,title,current,body,depth=0,toc=''):
    p='../' if depth else ''
    return f'''<!doctype html><html lang="it"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta name="color-scheme" content="light"><title>{E(title)} · Daisy Workbook</title><link rel="stylesheet" href="{p}assets/style.css"></head><body data-page="{E(current)}" data-prefix="{p}"><a class="skip" href="#main">Salta al contenuto</a><header class="topbar"><a class="brand" href="{p}index.html">Daisy <span>Workbook</span></a><button class="menu-toggle" data-menu aria-expanded="false" aria-controls="sidebar">Indice</button><div class="search-tools"><label class="sr-only" for="query">Cerca nel workbook</label><input id="query" type="search" placeholder="Progetto, modulo, simbolo…" autocomplete="off"><label class="sr-only" for="search-type">Tipo di risultato</label><select id="search-type"><option value="project">Progetti e moduli</option><option value="all" selected>Tutte le fonti</option><option value="code">Codice</option><option value="doc">Documenti</option></select></div><a class="print-link" href="{p}../print/report.html">Stampa</a></header><section id="search-panel" hidden aria-label="Risultati ricerca"><div class="search-results-inner"><p id="search-status" role="status"></p><button data-search-close>Chiudi ricerca</button><ul id="search-results"></ul></div></section><div class="layout"><aside class="sidebar" id="sidebar" data-open="false"><nav aria-label="Capitoli">{root_nav(d,current,p)}</nav><p class="snapshot">Snapshot {E(d['document']['build_date'])}<br>{E(d['document']['version'])} · Fonti locali<br>Build / hardware: NOT_RUN</p></aside><main id="main" class="content">{body}<footer>Fonti congelate il {E(d['document']['build_date'])} · {E(d['document']['version'])} · <a href="{p}sources.html">Provenienza</a></footer></main><aside class="toc" aria-label="In questa pagina">{toc}</aside></div><script src="{p}assets/search-data.js"></script><script src="{p}assets/app.js"></script></body></html>'''


def stats(d):
    return f'<p class="inventory-count" data-chapter-count="{len(d["chapters"])} chapters"><strong>{len(d["entries"])} voci</strong> · {len(d["sources"])-1:,} sorgenti e documenti · {len(d["chapters"])} capitoli</p>'


def table_rows(es, prefix=''):
    return ''.join(f'<tr data-group="{E(e["group"])}" data-name="{E((e["title"]+" "+e["board"]+" "+e["description"]+" "+" ".join(e["features"])).lower())}"><th scope="row"><a href="{prefix}chapters/{e["group"].replace("_","-")}.html#{e["id"]}">{E(e["title"])}</a><small>{E(e["path"])}</small></th><td>{E(e["board"])}</td><td>{E(e["description"][:230])}</td><td class="number">{e["code_files"]}</td></tr>' for e in es)


def dashboard(d,sm,vm):
    counts=Counter(e['group'] for e in d['entries']); maxv=max(counts.values())
    bars=''.join(f'<a class="distribution-row" href="chapters/{k.replace("_","-")}.html"><span>{E(LABELS[k])}</span><span class="bar-track"><span class="bar" style="width:{v/maxv*100:.3f}%"></span></span><strong>{v}</strong></a>' for k,v in counts.items())
    options=''.join(f'<option value="{k}">{E(LABELS[k])} ({v})</option>' for k,v in counts.items())
    visual=base.render_visual(vm['repository-map'],sm,'','assets/visuals/')
    body=f'''<h1>Daisy Workbook</h1><p class="lede">Esempi, progetti e codice.<br>Un riferimento completo per il workspace locale.</p>{stats(d)}<p class="lead-boundary">Descrizioni collegate ai sorgenti originali. Il codice è consultabile offline; build e hardware restano <strong>NOT_RUN</strong>.</p><div class="intro-links"><a href="chapters/guida.html">Leggi la guida e i confini della copertura</a><a href="../pdf/daisy-workbook.pdf">Scarica il PDF</a><a href="../coverage.json">Audit JSON</a></div><section id="distribuzione"><h2>La mappa del catalogo</h2><p>Numero di voci per famiglia. Una voce può essere un programma, un modulo di libreria o un workspace di supporto; le barre non misurano maturità o qualità.</p><div class="distribution">{bars}</div></section><section id="visual-orientation">{visual}</section><section id="catalogo"><div class="section-head"><h2>Tutte le voci</h2><label>Famiglia <select id="group-filter"><option value="all">Tutte ({len(d['entries'])})</option>{options}</select></label></div><p id="filter-status" role="status">{len(d['entries'])} voci</p><div class="table-wrap"><table class="catalog"><thead><tr><th>Progetto / modulo</th><th>Target</th><th>Descrizione dalle fonti</th><th>Codice</th></tr></thead><tbody>{table_rows(d['entries'])}</tbody></table></div><p id="filter-empty" hidden>Nessuna voce per questa selezione. Cambia famiglia o cerca nella barra superiore.</p></section>'''
    return shell(d,'Catalogo','dashboard',body,toc='<a href="#distribuzione">Distribuzione</a><a href="#visual-orientation">Percorsi</a><a href="#catalogo">Catalogo completo</a>')


def filelist(ids,sm):
    if not ids: return '<p class="muted">Nessun file in questa categoria.</p>'
    return '<ul class="files">'+''.join(f'<li><a href="../code/{sid}.html">{E(sm[sid]["path"])}</a> <small>{sm[sid]["lines"]} righe</small></li>' for sid in ids)+'</ul>'


def recipe(e):
    if e['group']=='host': return 'cmake -S DaisyHost -B DaisyHost/build\ncmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone\nctest --test-dir DaisyHost/build -C Release --output-on-failure'
    if e['group']=='dafx': return 'cmake -S DaisyDAFX -B DaisyDAFX/build -DBUILD_EXAMPLES=OFF\ncmake --build DaisyDAFX/build --config Release --target unit_tests\nctest --test-dir DaisyDAFX/build -C Release --output-on-failure'
    if e['build_ids'] and any(s.endswith('Makefile') for s in e.get('build_paths',[])): return f"# Dalla directory {e['path']}\nmake"
    return ''


def entry_html(e,sm,printing=False):
    docs=[sm[s] for s in e['doc_ids']]
    issues=''.join('<li>'+E(x)+'</li>' for x in e['issues'])
    facts=[('Target nel sorgente',e['board']),('Codice',str(e['code_files'])+' file / '+str(e['source_lines'])+' righe'),('Moduli riconoscibili',', '.join(e['features']) or 'Nessun modulo noto estratto.'),('Callback / controllo',', '.join(e['callbacks']) or 'Nessuna callback nota estratta.'),('Controlli riconoscibili',', '.join(e['controls']) or 'Mapping non estratto; consultare CONTROLS e sorgenti.'),('Classi / strutture',', '.join(e['classes']) or 'Nessuna dichiarazione estratta.')]
    rows=''.join(f'<tr><th scope="row">{E(k)}</th><td>{E(v)}</td></tr>' for k,v in facts)
    refs=f'<a href="../sources.html#{e["description_source_id"]}">Fonte della descrizione</a>'
    docslinks=filelist(e['doc_ids'],sm)
    e['build_paths']=[sm[s]['path'] for s in e['build_ids']]
    command=recipe(e); e.pop('build_paths',None)
    build='<h3>Build da verificare</h3><p>Ricetta dalle istruzioni locali, non eseguita per questo inventario.</p><pre><code>'+E(command)+'</code></pre>' if command else '<p class="muted">Nessuna ricetta autonoma dedotta. Leggere manifest e istruzioni del workspace.</p>'
    flags=''.join(f'<tr><th>{E(k)}</th><td><code>{E(v)}</code></td></tr>' for k,v in e['build_flags'])
    if flags: build+='<div class="table-wrap"><table><tbody>'+flags+'</tbody></table></div>'
    extra=''
    if not printing:
        incl=''.join('<li><code>'+E(x)+'</code></li>' for x in e['includes'])
        extra=f'<details><summary>Dipendenze nel codice ({len(e["includes"])} include)</summary><ul class="include-list">{incl}</ul></details><details><summary>Sorgenti completi ({len(e["code_ids"])} file)</summary>{filelist(e["code_ids"],sm)}</details><details><summary>Manifest e altri file ({len(e["source_ids"])-len(e["code_ids"])-len(e["doc_ids"])} file)</summary>{filelist([s for s in e["source_ids"] if s not in e["code_ids"]+e["doc_ids"]],sm)}</details>'
        lead=next((sm[s] for s in e['code_ids'] if Path(sm[s]['path']).suffix in {'.cpp','.cc','.c'}),None)
        if lead and lead['lines']:
            lines=lead['text'].splitlines(); match=next((i for i,l in enumerate(lines) if re.search(r'void.*(?:AudioCallback|Callback)\s*\(',l)),0)
            excerpt='\n'.join(lines[match:match+38])
            extra+=f'<details><summary>Estratto di lettura: {E(Path(lead["path"]).name)} · righe {match+1}–{min(match+38,len(lines))}</summary><p>Estratto verbatim, non dimostrazione eseguibile autonoma. <a href="../code/{lead["id"]}.html#L{match+1}">Apri il contesto completo</a></p><pre><code>{E(excerpt)}</code></pre></details>'
    sourceattrs=f'id="{e["id"]}"' if not printing else f'id="print-{e["id"]}"'
    return f'''<section class="project-section" {sourceattrs}><h2>{E(e['title'])}</h2><p class="path"><code>{E(e['path'])}</code></p><p><span class="label label-{e['description_label']}">{e['description_label']}</span> {E(e['description'])}</p><p class="source-ref">{refs} · {E(BOUNDARY)}</p><span id="claim-{e['id']}"></span><span id="facts-{e['id']}"></span><div class="table-wrap"><table class="facts"><tbody>{rows}</tbody></table></div><h3>Documentazione originale</h3>{docslinks}{build}{extra}<h3>Punti da controllare</h3><ul>{issues or '<li>Nessun problema documentale rilevato dagli indicatori statici.</li>'}</ul><p class="evidence-line">Sorgenti: VERIFIED · Build: NOT_RUN · Host test: NOT_RUN · Hardware: NOT_RUN</p></section>'''


def chapter_page(d,c,index,sm,vm):
    if not all('entry' in s for s in c['sections']):
        sections=''.join(f'<section id="{s["id"]}"><h2>{E(s["title"])}</h2>'+''.join(base.render_block(b,sm,'../',vm,'../assets/visuals/') for b in s['blocks'])+'</section>' for s in c['sections'])
    else:
        jump=''.join(f'<a href="#{s["id"]}">{E(s["title"])}</a>' for s in c['sections'])
        sections=f'<details class="chapter-index" open><summary>Voci in questo capitolo ({len(c["sections"])})</summary><div class="jump-list">{jump}</div></details>'+''.join(entry_html(s['entry'],sm) for s in c['sections'])
    previous=d['chapters'][index-1] if index else None; following=d['chapters'][index+1] if index+1<len(d['chapters']) else None
    pager=f'<nav class="pager" aria-label="Capitoli precedente e successivo"><a href="{previous["id"]+".html" if previous else "../index.html"}">Precedente: {E(previous["title"] if previous else "Catalogo")}</a><a href="{following["id"]+".html" if following else "../sources.html"}">Successivo: {E(following["title"] if following else "Fonti")}</a></nav>'
    toc=''.join(f'<a href="#{s["id"]}">{E(s["title"])}</a>' for s in c['sections'])
    return shell(d,c['title'],c['id'],f'<article data-chapter-id="{c["id"]}"><h1>{E(c["title"])}</h1><p class="lede">{E(c["summary"])}</p>{sections}{pager}</article>',depth=1,toc=toc)


def sources_page(d):
    gs=''.join(f'<section id="{g["id"]}"><h3>{E(g["term"])}</h3><p>{E(g["definition"])}</p><p>{E(g["engineer_translation"])}</p></section>' for g in d['glossary'])
    rows=[]
    for s in d['sources']:
        extra=f'{s["bytes"]:,} byte · {s["lines"]:,} righe<br><code>{s["sha256"]}</code>' if 'sha256' in s else 'Audit di copertura'
        rows.append(f'<tr id="{s["id"]}" data-source-id="{s["id"]}"><th scope="row"><a href="{E(s["url"])}">{E(s["title"])}</a></th><td>VERIFIED</td><td>{extra}</td></tr>')
    body=f'<h1>Fonti e glossario</h1><p class="lede">Ogni descrizione rimanda ai contenuti congelati. I checksum identificano i byte originali; la loro presenza non certifica il firmware.</p><section id="snapshot"><h2>Identità dello snapshot</h2><dl><dt>Repository</dt><dd>{E(d["snapshot"]["repo_remote"])}</dd><dt>Commit base</dt><dd><code>{E(d["snapshot"]["repo_head"])}</code></dd><dt>Branch di partenza</dt><dd>{E(d["snapshot"]["branch"])}</dd><dt>Worktree locale</dt><dd>{"Con modifiche locali: gli hash per file sono l’identità autorevole." if d["snapshot"]["dirty_worktree"] else "Pulito al censimento."}</dd></dl><p><a href="../coverage.json">Copertura e directory escluse (JSON)</a></p></section><section id="glossario"><h2>Glossario</h2>{gs}</section><section id="registro"><h2>Registro delle fonti ({len(d["sources"])-1} file + audit)</h2><div class="table-wrap"><table class="sources-table"><thead><tr><th>Fonte</th><th>Evidenza</th><th>Dimensione e SHA-256</th></tr></thead><tbody>{"".join(rows)}</tbody></table></div></section>'
    return shell(d,'Fonti e glossario','sources',body,toc='<a href="#snapshot">Identità</a><a href="#glossario">Glossario</a><a href="#registro">Registro completo</a>')


def print_report(d,sm,vm):
    chapters=[]
    for c in d['chapters']:
        if not all('entry' in s for s in c['sections']):
            ss=''.join(f'<section id="print-{s["id"]}"><h2>{E(s["title"])}</h2>'+''.join(base.render_block(b,sm,'../site/',vm,'../site/assets/visuals/') for b in s['blocks'])+'</section>' for s in c['sections'])
        else: ss=''.join(entry_html(s['entry'],sm,True).replace('../sources.html','../site/sources.html').replace('../code/','../site/code/') for s in c['sections'])
        chapters.append(f'<article class="chapter-print" id="print-chapter-{c["id"]}" data-chapter-id="{c["id"]}"><h1>{E(c["title"])}</h1><p>{E(c["summary"])}</p>{ss}</article>')
    toc='<section class="print-index"><h2>Indice</h2><ol>'+''.join(f'<li><a href="#print-chapter-{c["id"]}">{E(c["title"])} — {E(c["summary"])}</a></li>' for c in d['chapters'])+'</ol></section>'
    sources=''.join(f'<p data-source-id="{s["id"]}"><strong>{E(s["path"])}</strong> · '+(f'<code>{s["sha256"]}</code>' if 'sha256' in s else 'coverage audit')+'</p>' for s in d['sources'])
    return f'<!doctype html><html lang="it"><head><meta charset="utf-8"><title>Daisy Workbook · stampa</title><link rel="stylesheet" href="../site/assets/style.css"></head><body class="print-document"><main><h1>Daisy Workbook</h1>{stats(d)}<p>Snapshot {d["document"]["build_date"]} · {E(d["document"]["source_freeze"])}</p><p>Edizione di consultazione stampabile. I sorgenti completi sono nell’edizione HTML offline; i documenti originali conservano le descrizioni estese e le loro prove storiche.</p>{toc}{"".join(chapters)}<article class="chapter-print source-appendix"><h1>Registro delle fonti</h1>{sources}</article></main></body></html>'


def search_index(d):
    rows=[{'type':'chapter','title':c['title'],'text':c['summary'],'href':'chapters/'+c['id']+'.html'} for c in d['chapters']]
    for e in d['entries']:
        rows.append({'type':'project','title':e['title'],'text':' '.join([e['path'],e['board'],e['description'],*e['features'],*e['callbacks'],*e['classes']]),'group':e['group'],'href':'chapters/'+e['group'].replace('_','-')+'.html#'+e['id']})
    for s in d['sources']:
        if 'text' not in s: continue
        symbols=' '.join(sorted(set(re.findall(r'\b[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*',s['text']))))
        code=Path(s['path']).suffix in {'.cpp','.cc','.c','.h','.hpp','.ino','.py','.ps1','.sh','.cmake','.ioc','.dvpe'}
        rows.append({'type':'code' if code else 'doc','title':s['path'],'text':symbols+' '+first_snippet(s['text']),'href':s['url']})
    return rows


def first_snippet(t):
    return re.sub(r'\s+',' ',t[:1600])[:650]


def build(corpus_path,selection_path,out):
    d=json.loads(corpus_path.read_text(encoding='utf-8')); base.validate_corpus(d)
    base.STYLE=(HERE/'theme.css').read_text(encoding='utf-8'); base.APP=(HERE/'app.js').read_text(encoding='utf-8')
    base.dashboard=dashboard; base.chapter_page=chapter_page; base.sources_page=sources_page; base.print_report=print_report; base.search_index=search_index
    emit_visuals(corpus_path.parent/'visuals')
    manifest=base.build(corpus_path,selection_path,out)
    sm={s['id']:s for s in d['sources']}
    index=search_index(d)
    write(out/'site/assets/search-data.js','window.DAISY_SEARCH='+json.dumps(index,ensure_ascii=False).replace('</','<\\/')+';\n')
    write(out/'coverage.json',json.dumps(d['coverage'],ensure_ascii=False,indent=2)+'\n')
    for s in d['sources']:
        if 'text' not in s: continue
        raw=base64.b64decode(s['raw_base64']) if 'raw_base64' in s else s['text'].encode('utf-8')
        if hashlib.sha256(raw).hexdigest()!=s['sha256']: raise ValueError('Snapshot bytes do not match source hash: '+s['path'])
        rawpath=out/'site/originals'/Path(s['path']); rawpath.parent.mkdir(parents=True,exist_ok=True); rawpath.write_bytes(raw)
        lines=''.join(f'<span class="code-line" id="L{i}"><a class="line-number" href="#L{i}" aria-label="Riga {i}">{i}</a><span>{E(line)}</span></span>' for i,line in enumerate(s['text'].splitlines(),1))
        body=f'<article class="source-reader"><h1>{E(Path(s["path"]).name)}</h1><p class="path">{E(s["path"])}</p><p class="source-info">{s["lines"]:,} righe · {s["bytes"]:,} byte · <strong>VERIFIED: snapshot del file</strong></p><p class="checksum"><span>SHA-256</span> <code>{s["sha256"]}</code></p><p>{E(BOUNDARY)}</p><div class="code-actions"><button data-copy>Copia contenuto</button><a download href="../originals/{quote(s["path"])}">Scarica originale</a><a href="../sources.html#{s["id"]}">Provenienza</a><span id="copy-status" role="status"></span></div><pre class="source-code"><code id="source-code">{lines}</code></pre></article>'
        write(out/'site/code'/f'{s["id"]}.html',shell(d,Path(s['path']).name,'code',body,depth=1,toc='<a href="#source-code">Codice completo</a>'))
    manifest['generator']='daisy-workbook/1.0 + html-hyper-doc/1.0'
    manifest['document']['entry_count']=len(d['entries']); manifest['document']['snapshot_file_count']=len(d['sources'])-1
    manifest['files']=[{'path':p.relative_to(out).as_posix(),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size} for p in sorted(out.rglob('*')) if p.is_file() and p.name not in {'manifest.json','quality-report.md','render-report.json'} and p.relative_to(out).parts[0] not in {'pdf','screenshots'}]
    write(out/'manifest.json',json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
    return manifest


if __name__=='__main__':
    ap=argparse.ArgumentParser(); ap.add_argument('--refresh',action='store_true'); ap.add_argument('--date',default='2026-10-04'); ap.add_argument('--output',type=Path,default=HERE/'artifact'); ap.add_argument('--content',type=Path,default=HERE/'src/corpus.json'); args=ap.parse_args()
    compressed=args.content.with_suffix(args.content.suffix+'.gz')
    if not args.refresh and not args.content.exists() and compressed.exists():
        import gzip
        args.content.write_bytes(gzip.decompress(compressed.read_bytes()))
    if args.refresh or not args.content.exists():
        from collect import collect
        invpath=HERE/'src/inventory.json'
        inv=json.loads(invpath.read_text(encoding='utf-8')) if invpath.exists() and not args.refresh else collect(args.date)
        write(args.content,json.dumps(make_corpus(inv),ensure_ascii=False,indent=2)+'\n')
    result=build(args.content,HERE/'src/selection.json',args.output.resolve())
    print(json.dumps(result['document'],ensure_ascii=False))

