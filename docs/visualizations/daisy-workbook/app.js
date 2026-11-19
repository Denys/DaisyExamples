'use strict';
(() => {
  const data = window.DAISY_SEARCH || [];
  const prefix = document.body.dataset.prefix || '';
  const input = document.querySelector('#query');
  const type = document.querySelector('#search-type');
  const panel = document.querySelector('#search-panel');
  const list = document.querySelector('#search-results');
  const status = document.querySelector('#search-status');
  const normalize = value => value.toLocaleLowerCase('it').normalize('NFD').replace(/[\u0300-\u036f]/g,'');
  const rows = data.map(row => ({...row, haystack: normalize(row.title+' '+row.text)}));
  function search() {
    const q = normalize(input.value.trim());
    panel.hidden = !q;
    list.replaceChildren();
    if (!q) return;
    const terms=q.split(/\s+/);
    const found=rows.filter(row => (type.value==='all' || row.type===type.value) && terms.every(term => row.haystack.includes(term)));
    found.sort((a,b) => (normalize(b.title).includes(q)?1:0)-(normalize(a.title).includes(q)?1:0));
    status.textContent=found.length ? `${found.length} risultati${found.length>60?' · primi 60 mostrati, restringi la ricerca':''}` : 'Nessun risultato. Prova un nome più breve o scegli Tutte le fonti.';
    for (const row of found.slice(0,60)) {
      const li=document.createElement('li'),a=document.createElement('a'),title=document.createElement('strong'),desc=document.createElement('small');
      a.href=prefix+row.href;title.textContent=row.title;desc.textContent=({project:'Progetto / modulo',code:'Sorgente',doc:'Documento',chapter:'Capitolo'}[row.type]||row.type)+' · '+row.text.slice(0,170);
      a.append(title,desc);li.append(a);list.append(li);
    }
  }
  let timer;
  input?.addEventListener('input',()=>{clearTimeout(timer);timer=setTimeout(search,80);});
  type?.addEventListener('change',search);
  document.querySelector('[data-search-close]')?.addEventListener('click',()=>{panel.hidden=true;input.focus();});
  const menu=document.querySelector('[data-menu]'),sidebar=document.querySelector('.sidebar');
  function closeMenu(){if(sidebar)sidebar.dataset.open='false';menu?.setAttribute('aria-expanded','false');}
  menu?.addEventListener('click',()=>{const open=sidebar.dataset.open!=='true';sidebar.dataset.open=String(open);menu.setAttribute('aria-expanded',String(open));if(open)sidebar.querySelector('a')?.focus();});
  document.addEventListener('keydown',ev=>{if(ev.key==='Escape'){panel.hidden=true;closeMenu();input?.focus();}if((ev.ctrlKey||ev.metaKey)&&ev.key.toLowerCase()==='k'){ev.preventDefault();input?.focus();}});
  sidebar?.addEventListener('click',ev=>{if(ev.target.closest('a'))closeMenu();});
  const filter=document.querySelector('#group-filter');
  filter?.addEventListener('change',()=>{let visible=0;document.querySelectorAll('.catalog tbody tr').forEach(row=>{row.hidden=filter.value!=='all'&&row.dataset.group!==filter.value;if(!row.hidden)visible++;});document.querySelector('#filter-status').textContent=visible+' voci';document.querySelector('#filter-empty').hidden=visible!==0;});
  document.querySelector('[data-copy]')?.addEventListener('click',async()=>{
    const code=document.querySelector('#source-code');
    const raw=[...code.querySelectorAll('.code-line>span')].map(line=>line.textContent).join('\n');
    const message=document.querySelector('#copy-status');
    try{if(!navigator.clipboard)throw Error('Clipboard unavailable');await navigator.clipboard.writeText(raw);message.textContent='Contenuto copiato.';}catch{const range=document.createRange();range.selectNodeContents(code);const selection=window.getSelection();selection.removeAllRanges();selection.addRange(range);message.textContent='Copia automatica non disponibile: usa Scarica originale per ottenere il file esatto.';}
  });
})();
