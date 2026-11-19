# Daisy Workbook

Workbook HTML offline del checkout locale DaisyExamples del **4 ottobre 2026**:
**502 voci**, **2.829 file originali**, **24 capitoli** e un PDF di **423 pagine**.

## Aprire

- `artifact/site/index.html`: catalogo, ricerca globale e filtri per famiglia.
- `artifact/pdf/daisy-workbook.pdf`: indice cliccabile e segnalibri.
- `daisy-workbook.zip`: estrarre e aprire `site/index.html`; mantenere l’intera struttura.

Ogni voce espone descrizione, target riconoscibile nel sorgente, moduli, callback,
controlli, include, manifest, documenti originali ed eventuali lacune. Il lettore
sorgenti conserva i file completi, numeri di riga e download degli originali.

Le spiegazioni editoriali sono italiane. Estratti di README, commenti e codice
mantengono la lingua originale e rimandano alla fonte congelata. La ricerca
indicizza tutti gli identificatori e simboli qualificati del file, oltre a
percorsi, nomi e descrizioni; non interpreta automaticamente il flusso DSP.

## Copertura

Seed, Pod, Field, Patch, Patch SM, Pedal, Petal, Versio e Legio; tutti i file
ammessi dei progetti custom e degli esperimenti; fondamenti; DaisyHost,
DaisyDAFX, DaisySP (compresa LGPL), libDaisy (compresi core, startup, esempi e
test); supporto QAE/CI/Cube/DVPE; librerie custom e copie archivio distinte.
`artifact/coverage.json` registra radici, esclusioni e directory senza sorgenti
ammessi. L’inventario è del repository locale, non dell’intero ecosistema web.

Sono esclusi binari/build, media, dataset/log privati, cache e stato degli agenti,
corpora non-Daisy (DaisyTheory, Teensy), vendor nelle directory escluse. Le
licenze e attribuzioni disponibili sono conservate negli originali; il workbook
non assegna una nuova licenza ai contenuti importati.

## Evidenza

- Snapshot Git base: `659677e61c7f6268c0d11d7c7c775806afe6be68`, worktree con modifiche.
- Ogni originale ha SHA-256 dei byte: il solo commit non identifica il contenuto locale.
- `VERIFIED` indica lettura/congelamento, `DERIVED` sintesi statica da simboli/documenti.
- Build firmware, test host, audio, flashing e hardware: **NOT_RUN**.
- Le prove riportate nei README/CHECKPOINT sono storiche; non vengono rivalidate da questo inventario.

## Rigenerare

Richiede Python e la skill installata `html-hyper-doc` (configurabile con
`HTML_HYPER_DOC_SKILL`). Il builder estende a runtime il generatore della skill,
senza modificarlo o copiarne la libreria di template. Template base: `docs-page`;
donatori `dashboard` (distribuzione) e `data-report` (tabelle). HTML-SAK non usato.

```powershell
# Rigenerare lo stesso snapshot; il corpus .json.gz viene espanso automaticamente.
py -3 -X utf8 docs/visualizations/daisy-workbook/build.py

# Nuovo snapshot: censisce nuovamente i percorsi locali dichiarati.
py -3 -X utf8 docs/visualizations/daisy-workbook/build.py --refresh --date YYYY-MM-DD

# Verifiche: Playwright Chromium, pypdf e PyMuPDF devono essere disponibili.
py -3 -X utf8 docs/visualizations/daisy-workbook/verify.py --render --regeneration
```

Non modificare a mano le pagine generate. Cambiare il corpus/generatore e
rigenerare. `--refresh` sostituisce lo snapshot e richiede nuove verifiche.

## QA eseguita

Registro fonti, copertura, tutti gli hash e collegamenti interni; rigenerazione
identica e fixture add/update/remove; Chromium offline a 1440×1000 e 390×844;
ricerca (compresi simboli oltre l’inizio del file), stato vuoto, filtri, tastiera,
menu mobile, scorrimento tabelle e lettore del codice; console/rete/overflow;
PDF con indice, segnalibri e pagine rasterizzate.

Evidenze: `verification.json`, `determinism-report.json`,
`artifact/quality-report.md`, `artifact/render-report.json`, screenshot e PDF.
La verifica visiva è rappresentativa; gli hash e i link coprono l’intero corpus.
