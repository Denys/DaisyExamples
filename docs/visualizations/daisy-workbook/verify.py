"""Structural, source integrity, regeneration and rendered acceptance for the workbook."""
from __future__ import annotations
import argparse, copy, hashlib, importlib.util, json, tempfile
from collections import Counter
from pathlib import Path
import build as builder

HERE=Path(__file__).resolve().parent

def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()

def static_check(artifact):
    spec=importlib.util.spec_from_file_location('validator',builder.SKILL/'scripts/validate_hyper_doc.py')
    v=importlib.util.module_from_spec(spec); spec.loader.exec_module(v)
    original=v.parse_page
    class FastIds(list):
        def __init__(self, values):
            super().__init__(values); self.counts=Counter(values)
        def count(self,x): return self.counts[x]
    def parse(p):
        parser=original(p); parser.ids=FastIds(parser.ids)
        if p.parent.name=='code':
            line_links=[l for l in parser.links if l.startswith('#L')]
            assert all(l[1:] in parser.ids.counts for l in line_links), 'Broken line anchors in '+str(p)
            parser.links=[l for l in parser.links if not l.startswith('#L')]
        return parser
    v.parse_page=parse
    result=v.validate(artifact)
    d=json.loads(next((artifact/'content').glob('*.json')).read_text(encoding='utf-8'))
    failures=result['failures']
    files=[s for s in d['sources'] if 'sha256' in s]
    for s in files:
        if digest(artifact/'site/originals'/s['path'])!=s['sha256']: failures.append('Source snapshot hash mismatch: '+s['path'])
    coverage=d['coverage']
    if coverage['retained_files']!=len(files) or coverage['uncatalogued_files']: failures.append('Coverage does not reconcile')
    owned={sid for e in d['entries'] for sid in e['source_ids']}
    if owned!={s['id'] for s in files}: failures.append('Source ownership coverage mismatch')
    if len(d['entries'])!=sum(len(c['sections']) for c in d['chapters'] if c['id']!='guida'): failures.append('Entry/chapter count mismatch')
    result['checks'].update({'entries':len(d['entries']),'original_byte_hash_checks':len(files),'coverage_unowned_files':len(coverage['uncatalogued_files'])})
    result['status']='FAIL' if failures else 'PASS'
    builder.write(artifact/'quality-report.md','# Workbook structural QA\n\n'+json.dumps(result,ensure_ascii=False,indent=2)+'\n')
    return result

def regeneration(artifact):
    # Fresh bounded fixture roots inside this task. Never touch an existing user output.
    d=json.loads((HERE/'src/corpus.json').read_text(encoding='utf-8'))
    selected=d['chapters'][1]
    keep=selected['sections'][:2]
    selected['sections']=keep
    d['chapters']=[selected]
    d['entries']=[s['entry'] for s in keep]
    ids={sid for e in d['entries'] for sid in e['source_ids']}
    d['sources']=[s for s in d['sources'] if s['id'] in ids or s['id']=='inventory-freeze']
    # The map uses the inventory source; the fixture intentionally tests deletion only.
    d['coverage']['retained_files']=len(d['sources'])-1
    d['coverage']['snapshot_files']=len(d['sources'])-1
    with tempfile.TemporaryDirectory(prefix='daisy-workbook-fixture-',dir=HERE) as tmp:
        tmp=Path(tmp); content=tmp/'fixture.json'; output=tmp/'artifact'
        builder.write(content,json.dumps(d,ensure_ascii=False,indent=2)+'\n')
        first=builder.build(content,HERE/'src/selection.json',output)
        hashes1={r['path']:r['sha256'] for r in first['files']}
        second=builder.build(content,HERE/'src/selection.json',output)
        hashes2={r['path']:r['sha256'] for r in second['files']}
        assert hashes1==hashes2,'Non-deterministic fixture rebuild'
        # Add a chapter, update one description, remove a section/source and its reader.
        removed=d['entries'].pop()
        removed_code=removed['code_ids'][0] if removed['code_ids'] else removed['source_ids'][0]
        d['chapters'][0]['sections'].pop()
        d['entries'][0]['description']='UPDATED_FIXTURE_UNIQUE_MARKER'
        d['chapters'][0]['sections'][0]['entry']=d['entries'][0]
        added=copy.deepcopy(d['chapters'][0]); added['id']='fixture-add'; added['title']='Added fixture'
        added['sections']=[{'id':'fixture-added-section','title':'Added','blocks':[{'type':'paragraph','text':'ADD_FIXTURE_UNIQUE_MARKER'}]}]
        d['chapters'].append(added)
        needed={sid for e in d['entries'] for sid in e['source_ids']}
        d['sources']=[s for s in d['sources'] if s['id'] in needed or s['id']=='inventory-freeze']
        builder.write(content,json.dumps(d,ensure_ascii=False,indent=2)+'\n')
        third=builder.build(content,HERE/'src/selection.json',output)
        idx=(output/'site/search-index.json').read_text(encoding='utf-8')
        assert 'UPDATED_FIXTURE_UNIQUE_MARKER' in idx
        assert 'fixture-add.html' in idx and (output/'site/chapters/fixture-add.html').exists()
        assert removed['id'] not in idx
        if removed_code not in needed: assert not (output/'site/code'/f'{removed_code}.html').exists()
        assert third['document']['entry_count']==1
        assert third['document']['source_count']==len(d['sources'])
        assert 'UPDATED_FIXTURE_UNIQUE_MARKER' in (output/'print/report.html').read_text(encoding='utf-8')
        # Remove the added chapter, preserving a user-owned marker outside generated trees.
        builder.write(output/'user-note.txt','keep me')
        d['chapters'].pop(); builder.write(content,json.dumps(d,ensure_ascii=False,indent=2)+'\n')
        builder.build(content,HERE/'src/selection.json',output)
        assert not (output/'site/chapters/fixture-add.html').exists()
        assert 'fixture-add.html' not in (output/'site/search-index.json').read_text(encoding='utf-8')
        assert (output/'user-note.txt').read_text()=='keep me'
    return {'status':'PASS','byte_identical_fixture':True,'add_update_remove':True,'removed_code_and_chapter_no_orphans':True,'user_file_preserved':True}


def render(artifact):
    from playwright.sync_api import sync_playwright
    d=json.loads(next((artifact/'content').glob('*.json')).read_text(encoding='utf-8'))
    report={'schema_version':'1.0','status':'PASS','failures':[], 'pages':[], 'interactions':{},'checks':{},'viewports':{'desktop':{'width':1440,'height':1000},'mobile':{'width':390,'height':844}}}
    shot=artifact/'screenshots'; shot.mkdir(exist_ok=True); (artifact/'pdf').mkdir(exist_ok=True)
    def url(rel): return (artifact/rel).resolve().as_uri()
    with sync_playwright() as p:
        browser=p.chromium.launch()
        report['browser']=browser.version
        for device,view in report['viewports'].items():
            context=browser.new_context(viewport=view,offline=True,device_scale_factor=1)
            page=context.new_page(); errors=[]; external=[]
            page.on('pageerror',lambda e:errors.append(str(e)))
            page.on('console',lambda m:errors.append(m.text) if m.type=='error' else None)
            page.on('request',lambda r:external.append(r.url) if r.url.startswith(('http:','https:')) else None)
            for rel,name in [('site/index.html','catalogo'),('site/chapters/custom.html','custom'),('site/chapters/host.html','host'),('site/chapters/daisysp.html','daisysp'),('site/sources.html','fonti')]:
                page.goto(url(rel),wait_until='load'); page.wait_for_timeout(100)
                overflow=page.evaluate('document.documentElement.scrollWidth > innerWidth + 1')
                images=page.locator('img').evaluate_all('(xs)=>xs.every(x=>x.complete&&x.naturalWidth>0)')
                if overflow: report['failures'].append(device+' overflow '+name)
                if not images: report['failures'].append(device+' broken image '+name)
                page.screenshot(path=str(shot/f'{name}-{device}.png'),full_page=False)
                report['pages'].append({'page':rel,'viewport':device,'overflow':overflow,'images_loaded':images})
            page.goto(url('site/index.html'))
            page.locator('#query').fill('Field_AdditiveSynth'); page.wait_for_timeout(200)
            found=page.locator('#search-results li').count(); report['interactions'][device+'_search_project']=found>0
            assert found>0
            page.locator('#query').fill('unlikely_no_match_398499'); page.wait_for_timeout(180)
            report['interactions'][device+'_search_empty']='Nessun risultato' in page.locator('#search-status').inner_text()
            page.keyboard.press('Escape'); assert page.locator('#search-panel').is_hidden()
            page.locator('#query').fill('RecalcPartials'); page.locator('#search-type').select_option('code'); page.wait_for_timeout(180)
            report['interactions'][device+'_search_code']=page.locator('#search-results li').count()>0
            page.keyboard.press('Escape'); page.locator('#group-filter').select_option('custom')
            visible=page.locator('.catalog tbody tr:visible').count()
            expected=sum(e['group']=='custom' for e in d['entries'])
            report['interactions'][device+'_filter_count']={'observed':visible,'expected':expected}
            assert visible==expected
            if device=='mobile':
                page.locator('[data-menu]').click(); assert page.locator('.sidebar').get_attribute('data-open')=='true'
                page.screenshot(path=str(shot/'indice-mobile.png'))
                page.keyboard.press('Escape'); assert page.locator('.sidebar').get_attribute('data-open')=='false'
                report['interactions']['mobile_menu_open_close']=True
                page.goto(url('site/index.html')); table=page.locator('.table-wrap').last
                report['interactions']['mobile_table_pan']=table.evaluate('(x)=>{x.scrollLeft=100;return x.scrollLeft>0}')
            else:
                page.keyboard.press('Control+k'); assert page.locator('#query').evaluate('(x)=>x===document.activeElement')
                report['interactions']['keyboard_search_focus']=True
            # Inspect a real callback and original-source reader, including line anchors.
            target=next(e for e in d['entries'] if e['title']=='Field_AdditiveSynth')
            page.goto(url('site/chapters/custom.html')+'#'+target['id']); page.locator('#'+target['id']).scroll_into_view_if_needed()
            page.screenshot(path=str(shot/f'progetto-{device}.png'))
            sid=target['primary_source_id']
            page.goto(url('site/code/'+sid+'.html')+'#L1')
            assert page.locator('#L1').count()==1
            assert page.locator('.code-line').count()==next(s['lines'] for s in d['sources'] if s['id']==sid)
            overflow=page.evaluate('document.documentElement.scrollWidth > innerWidth + 1')
            if overflow:report['failures'].append(device+' source overflow')
            page.screenshot(path=str(shot/f'codice-{device}.png'))
            report['interactions'][device+'_source_reader']=True
            if errors: report['failures'].extend(errors)
            if external: report['failures'].extend('Unexpected network: '+u for u in external)
            report['checks'][device+'_console_errors']=errors
            report['checks'][device+'_external_requests']=external
            context.close()
        context=browser.new_context(viewport=report['viewports']['desktop'],offline=True)
        page=context.new_page(); page.goto(url('print/report.html'),wait_until='load'); page.emulate_media(media='print')
        page.pdf(path=str(artifact/'pdf/daisy-workbook.pdf'),format='A4',print_background=True,prefer_css_page_size=True,outline=True,tagged=True)
        page.screenshot(path=str(shot/'stampa-desktop.png'))
        report['checks']['print_chapters']=page.locator('[data-chapter-id]').count()
        report['checks']['print_project_sections']=page.locator('.project-section').count()
        assert report['checks']['print_project_sections']==len(d['entries'])
        assert report['checks']['print_chapters']==len(d['chapters'])
        context.close();browser.close()
    from pypdf import PdfReader
    pdf=PdfReader(artifact/'pdf/daisy-workbook.pdf'); report['checks']['pdf_pages']=len(pdf.pages)
    import fitz
    with fitz.open(artifact/'pdf/daisy-workbook.pdf') as rendered_pdf:
        for number in [0,1,2,min(25,len(rendered_pdf)-1),len(rendered_pdf)-1]:
            rendered_pdf[number].get_pixmap(matrix=fitz.Matrix(1.5,1.5)).save(shot/f'pdf-page-{number+1}.png')
        report['checks']['pdf_outline_entries']=len(rendered_pdf.get_toc())
    report['checks']['pdf_first_page_text']=pdf.pages[0].extract_text()[:250]
    assert 'Daisy Workbook' in pdf.pages[0].extract_text()
    for k,v in report['interactions'].items():
        if v is False:report['failures'].append('Interaction failed: '+k)
    report['status']='FAIL' if report['failures'] else 'PASS'
    builder.write(artifact/'render-report.json',json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    m=json.loads((artifact/'manifest.json').read_text(encoding='utf-8'))
    m['render_files']=[{'path':f.relative_to(artifact).as_posix(),'sha256':digest(f),'bytes':f.stat().st_size} for f in sorted(list(shot.glob('*.png'))+list((artifact/'pdf').glob('*.pdf'))+[artifact/'render-report.json'])]
    builder.write(artifact/'manifest.json',json.dumps(m,ensure_ascii=False,indent=2)+'\n')
    return report

if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--artifact',type=Path,default=HERE/'artifact');ap.add_argument('--render',action='store_true');ap.add_argument('--regeneration',action='store_true');args=ap.parse_args()
    result={}
    if args.regeneration: result['regeneration']=regeneration(args.artifact)
    if args.render: result['render']=render(args.artifact.resolve()); print('Render:',result['render']['status'])
    result['structural']=static_check(args.artifact.resolve())
    builder.write(HERE/'verification.json',json.dumps(result,ensure_ascii=False,indent=2)+'\n')
    print(json.dumps({k:{'status':v['status'],'failures':v.get('failures',[]),'checks':v.get('checks',{})} for k,v in result.items()},ensure_ascii=False))
    raise SystemExit(0 if all(r['status']=='PASS' for r in result.values()) else 1)
