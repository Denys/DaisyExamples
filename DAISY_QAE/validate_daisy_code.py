#!/usr/bin/env python3
"""validate_daisy_code.py — Lint Daisy Field C++ files against QAE standards.

Checks for common anti-patterns and missing initialization patterns
documented in DAISY_DEVELOPMENT_STANDARDS.md and DAISY_BUGS.md.

Rules:
    CTRL-IN-CALLBACK    ProcessAllControls() inside AudioCallback
    ALLOC-IN-CALLBACK   malloc/new/printf/PrintLine inside AudioCallback
    NO-INIT             DSP module declared but never Init'd
    NO-START-ADC        StartAudio() without StartAdc()
    ADC-AFTER-AUDIO     StartAdc() called after StartAudio()
    NO-LOOP-DELAY       Main while(1) missing System::Delay()
    MISSING-INCLUDE     DaisyField used without #include
    STL-USAGE           std::vector/map/string/function detected
    MAKEFILE-NO-LIBDAISY/NO-DAISYSP  Makefile missing directories
    LGPL-FLAG-MISSING   LGPL module used without USE_DAISYSP_LGPL = 1
    HALLUCINATED-API    Known fabricated DaisySP method name detected
    FIELD-KEY-ROW-MAP   Legacy Field A/B key scan mapping detected
    FIELD-LED-ROW-MAP   Legacy Field A/B LED helper mapping detected
    FLOAT-PRINTF-NANO   Float printf formatting used without _printf_float
    FLOAT-SCANF-NANO    Float scanf formatting used without _scanf_float
    FLOAT-PRINTF-FLAG   Makefile deliberately links float printf support
    FLOAT-SCANF-FLAG    Makefile deliberately links float scanf support
    DYNAMIC-ALLOC       Heap allocation detected in firmware source
    IOSTREAM-USAGE      Desktop-style C++ stream I/O detected

Usage:
    python validate_daisy_code.py <file.cpp>
    python validate_daisy_code.py MyProjects/_projects/*/  # check all projects

Exit codes:
    0  All checks passed
    1  Warnings or errors found
"""

import re
import sys
import os
from pathlib import Path
from dataclasses import dataclass
from typing import List


@dataclass
class Finding:
    level: str      # "ERROR" or "WARN"
    line: int       # 1-based line number, 0 if file-level
    rule: str       # Short rule ID
    message: str    # Human-readable description


def extract_audio_callback(lines: List[str]) -> tuple:
    """Find AudioCallback function boundaries. Returns (start, end) line indices."""
    start = None
    brace_depth = 0
    for i, line in enumerate(lines):
        if start is None:
            if re.search(r'\bAudioCallback\b', line) and ('{' in line or
                    (i + 1 < len(lines) and '{' in lines[i + 1])):
                start = i
                brace_depth = line.count('{') - line.count('}')
                if brace_depth == 0 and i + 1 < len(lines):
                    continue
        elif start is not None:
            brace_depth += line.count('{') - line.count('}')
            if brace_depth <= 0:
                return (start, i)
    if start is not None:
        return (start, len(lines) - 1)
    return (None, None)


def _pattern_line(content: str, pattern: str) -> int:
    match = re.search(pattern, content, re.DOTALL)
    if not match:
        return 0
    return content[:match.start()].count('\n') + 1


def check_field_row_mapping_patterns(content: str) -> List[Finding]:
    """Catch the recurring Daisy Field physical A/B row inversion."""
    findings = []

    old_key_a = (
        r'kKeyAIndices\s*\[\s*8\s*\]\s*=\s*\{\s*0\s*,\s*1\s*,\s*2\s*,'
        r'\s*3\s*,\s*4\s*,\s*5\s*,\s*6\s*,\s*7\s*\}'
    )
    old_key_b = (
        r'kKeyBIndices\s*\[\s*8\s*\]\s*=\s*\{\s*8\s*,\s*9\s*,\s*10\s*,'
        r'\s*11\s*,\s*12\s*,\s*13\s*,\s*14\s*,\s*15\s*\}'
    )
    if re.search(old_key_a, content, re.DOTALL) or re.search(old_key_b, content, re.DOTALL):
        line = _pattern_line(content, old_key_a) or _pattern_line(content, old_key_b)
        findings.append(Finding(
            "ERROR", line, "FIELD-KEY-ROW-MAP",
            "Legacy Daisy Field A/B scan map detected; physical/logical A uses "
            "scan 8..15 and physical/logical B uses scan 0..7"
        ))

    old_led_a_numeric = (
        r'kLedKeysA\s*\[\s*8\s*\]\s*=\s*\{\s*15\s*,\s*14\s*,\s*13\s*,'
        r'\s*12\s*,\s*11\s*,\s*10\s*,\s*9\s*,\s*8\s*\}'
    )
    old_led_b_numeric = (
        r'kLedKeysB\s*\[\s*8\s*\]\s*=\s*\{\s*0\s*,\s*1\s*,\s*2\s*,'
        r'\s*3\s*,\s*4\s*,\s*5\s*,\s*6\s*,\s*7\s*\}'
    )
    old_led_a_enum = (
        r'kLedKeysA\s*\[\s*8\s*\]\s*=\s*\{(?=[^;]*LED_KEY_A1)(?=[^;]*LED_KEY_A8)[^;]*\}'
    )
    old_led_b_enum = (
        r'kLedKeysB\s*\[\s*8\s*\]\s*=\s*\{(?=[^;]*LED_KEY_B1)(?=[^;]*LED_KEY_B8)[^;]*\}'
    )
    old_led_patterns = [old_led_a_numeric, old_led_b_numeric, old_led_a_enum, old_led_b_enum]
    if any(re.search(pattern, content, re.DOTALL) for pattern in old_led_patterns):
        line = next((_pattern_line(content, pattern) for pattern in old_led_patterns
                     if _pattern_line(content, pattern)), 0)
        findings.append(Finding(
            "ERROR", line, "FIELD-LED-ROW-MAP",
            "Legacy Daisy Field LED row helper map detected; physical/logical A "
            "uses native LED_KEY_B1..B8 and physical/logical B uses native "
            "LED_KEY_A1..A8"
        ))

    return findings


def has_float_printf_format(line: str) -> bool:
    return bool(re.search(
        r'\b(?:snprintf|sprintf|printf|PrintLine)\s*\([^;\n]*%[-+ #0-9.]*[aAeEfFgG]',
        line
    ))


def has_float_scanf_format(line: str) -> bool:
    return bool(re.search(
        r'\b(?:scanf|sscanf|fscanf)\s*\([^;\n]*%[*0-9.]*l?[aAeEfFgG]',
        line
    ))


def check_float_printf_patterns(lines: List[str], has_float_printf_flag: bool) -> List[Finding]:
    findings = []
    if has_float_printf_flag:
        return findings

    for i, line in enumerate(lines):
        stripped = line.lstrip()
        if stripped.startswith('//'):
            continue
        if has_float_printf_format(line):
            findings.append(Finding(
                "ERROR", i + 1, "FLOAT-PRINTF-NANO",
                "Float printf format used without _printf_float; default Daisy "
                "newlib-nano builds can render %f/%g/%e values blank. Use "
                "integer-only formatting or explicitly budget for -u _printf_float."
            ))
    return findings


def check_float_scanf_patterns(lines: List[str], has_float_scanf_flag: bool) -> List[Finding]:
    findings = []
    if has_float_scanf_flag:
        return findings

    for i, line in enumerate(lines):
        stripped = line.lstrip()
        if stripped.startswith('//'):
            continue
        if has_float_scanf_format(line):
            findings.append(Finding(
                "ERROR", i + 1, "FLOAT-SCANF-NANO",
                "Float scanf format used without _scanf_float; default Daisy "
                "newlib-nano builds do not provide normal float scanning. Use "
                "integer/fixed-point parsing or explicitly budget for -u _scanf_float."
            ))
    return findings


def check_heap_allocation_patterns(lines: List[str]) -> List[Finding]:
    findings = []
    allocation_patterns = [
        (r'\bmalloc\s*\(', "malloc()"),
        (r'\bcalloc\s*\(', "calloc()"),
        (r'\brealloc\s*\(', "realloc()"),
        (r'\bfree\s*\(', "free()"),
        (r'\bnew\s+\w', "operator new"),
        (r'\bdelete\s+', "operator delete"),
    ]
    for i, line in enumerate(lines):
        stripped = line.lstrip()
        if stripped.startswith('//'):
            continue
        for pattern, name in allocation_patterns:
            if re.search(pattern, line):
                findings.append(Finding(
                    "WARN", i + 1, "DYNAMIC-ALLOC",
                    f"{name} detected; Daisy newlib-nano heap allocation can "
                    "fragment or fail in long-running audio firmware. Prefer "
                    "static/fixed-capacity storage."
                ))
                break
    return findings


def check_file(filepath: str) -> List[Finding]:
    findings = []
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()
    lines = content.splitlines()

    makefile_path = os.path.join(os.path.dirname(filepath), 'Makefile')
    makefile = None
    has_float_printf_flag = False
    has_float_scanf_flag = False
    if os.path.exists(makefile_path):
        with open(makefile_path, 'r', encoding='utf-8', errors='replace') as f:
            makefile = f.read()
        has_float_printf_flag = bool(re.search(r'-u\s*_printf_float|-u_printf_float',
                                               makefile))
        has_float_scanf_flag = bool(re.search(r'-u\s*_scanf_float|-u_scanf_float',
                                              makefile))

    findings.extend(check_field_row_mapping_patterns(content))
    findings.extend(check_float_printf_patterns(lines, has_float_printf_flag))
    findings.extend(check_float_scanf_patterns(lines, has_float_scanf_flag))
    findings.extend(check_heap_allocation_patterns(lines))
    if Path(filepath).suffix.lower() != '.cpp':
        return findings

    cb_start, cb_end = extract_audio_callback(lines)

    # ── Rule 1: ProcessAllControls in AudioCallback ──
    if cb_start is not None:
        for i in range(cb_start, cb_end + 1):
            if re.search(r'ProcessAllControls', lines[i]):
                findings.append(Finding(
                    "ERROR", i + 1, "CTRL-IN-CALLBACK",
                    "ProcessAllControls() inside AudioCallback — move to main loop"
                ))

    # ── Rule 2: malloc/new/printf/PrintLine in AudioCallback ──
    if cb_start is not None:
        forbidden = [
            (r'\bmalloc\s*\(', "malloc()"),
            (r'\bnew\s+\w', "operator new"),
            (r'\bprintf\s*\(', "printf()"),
            (r'\bPrintLine\s*\(', "PrintLine()"),
            (r'\bsprintf\s*\(', "sprintf()"),
        ]
        for i in range(cb_start, cb_end + 1):
            stripped = lines[i].lstrip()
            if stripped.startswith('//'):
                continue
            for pattern, name in forbidden:
                if re.search(pattern, lines[i]):
                    findings.append(Finding(
                        "ERROR", i + 1, "ALLOC-IN-CALLBACK",
                        f"{name} inside AudioCallback — non-deterministic, causes underruns"
                    ))

    # ── Rule 3: DSP modules declared but never Init'd ──
    dsp_modules = [
        'Oscillator', 'VariableShapeOsc', 'VariableSawOsc', 'OscillatorBank',
        'FormantOsc', 'Svf', 'OnePole', 'Biquad', 'MoogLadder',
        'Adsr', 'AdEnv', 'Chorus', 'Flanger', 'Phaser', 'Tremolo',
        'Overdrive', 'Decimator', 'ReverbSc', 'Limiter', 'Compressor',
        'DcBlock', 'ModalVoice', 'StringVoice', 'Pluck', 'Drip',
        'Resonator', 'DelayLine', 'CrossFade',
    ]
    for mod in dsp_modules:
        # Look for declaration: "ModuleName varname;" or "ModuleName varname[N];"
        decl_pattern = rf'\b{mod}\b\s+\w+'
        if re.search(decl_pattern, content):
            # Check if .Init( appears anywhere
            if not re.search(rf'\.Init\s*\(', content):
                findings.append(Finding(
                    "ERROR", 0, "NO-INIT",
                    f"{mod} declared but no .Init() call found — will crash or produce silence"
                ))

    # ── Rule 4: StartAdc before StartAudio ──
    start_adc_pos = None
    start_audio_pos = None
    for i, line in enumerate(lines):
        if re.search(r'StartAdc\s*\(', line) and start_adc_pos is None:
            start_adc_pos = i
        if re.search(r'StartAudio\s*\(', line) and start_audio_pos is None:
            start_audio_pos = i
    if start_audio_pos is not None and start_adc_pos is None:
        findings.append(Finding(
            "WARN", start_audio_pos + 1, "NO-START-ADC",
            "StartAudio() called but no StartAdc() found — knobs/CV won't work"
        ))
    elif (start_audio_pos is not None and start_adc_pos is not None
          and start_adc_pos > start_audio_pos):
        findings.append(Finding(
            "WARN", start_adc_pos + 1, "ADC-AFTER-AUDIO",
            "StartAdc() called after StartAudio() — may miss first reads"
        ))

    # ── Rule 5: Missing main loop delay ──
    in_main = False
    has_while_loop = False
    has_delay = False
    for i, line in enumerate(lines):
        if re.search(r'\bint\s+main\b', line):
            in_main = True
        if in_main and re.search(r'while\s*\(\s*1\s*\)', line):
            has_while_loop = True
        if has_while_loop and re.search(r'System::Delay\s*\(', line):
            has_delay = True
    if has_while_loop and not has_delay:
        findings.append(Finding(
            "WARN", 0, "NO-LOOP-DELAY",
            "Main while(1) loop has no System::Delay() — will spin CPU at 100%"
        ))

    # ── Rule 6: Field include check ──
    if re.search(r'DaisyField', content):
        if not re.search(r'#include\s*"daisy_field\.h"', content):
            findings.append(Finding(
                "ERROR", 0, "MISSING-INCLUDE",
                "Uses DaisyField but missing #include \"daisy_field.h\""
            ))

    # ── Rule 7: std::vector / std::map in audio code ──
    stl_patterns = [
        (r'\bstd::vector\b', "std::vector"),
        (r'\bstd::map\b', "std::map"),
        (r'\bstd::string\b', "std::string"),
        (r'\bstd::function\b', "std::function"),
    ]
    for pattern, name in stl_patterns:
        match = re.search(pattern, content)
        if match:
            line_num = content[:match.start()].count('\n') + 1
            findings.append(Finding(
                "WARN", line_num, "STL-USAGE",
                f"{name} used — avoid heap-allocating STL in embedded audio code"
            ))

    iostream_patterns = [
        (r'#include\s*<iostream>', "<iostream>"),
        (r'\bstd::cout\b', "std::cout"),
        (r'\bstd::cin\b', "std::cin"),
        (r'\bstd::cerr\b', "std::cerr"),
        (r'\bstd::endl\b', "std::endl"),
        (r'\bstd::ostream\b', "std::ostream"),
        (r'\bstd::istream\b', "std::istream"),
    ]
    for pattern, name in iostream_patterns:
        match = re.search(pattern, content)
        if match:
            line_num = content[:match.start()].count('\n') + 1
            findings.append(Finding(
                "WARN", line_num, "IOSTREAM-USAGE",
                f"{name} detected; desktop-style streams can pull large "
                "libstdc++_nano/runtime pieces. Use bounded snprintf or "
                "libDaisy logger instead."
            ))
            break

    # ── Rule 8: Makefile checks ──
    if makefile is not None:
        if not re.search(r'LIBDAISY_DIR', makefile):
            findings.append(Finding(
                "ERROR", 0, "MAKEFILE-NO-LIBDAISY",
                "Makefile missing LIBDAISY_DIR — won't compile"
            ))
        if not re.search(r'DAISYSP_DIR', makefile):
            findings.append(Finding(
                "ERROR", 0, "MAKEFILE-NO-DAISYSP",
                "Makefile missing DAISYSP_DIR — won't compile"
            ))
        # Check if LGPL modules used but flag missing
        lgpl_modules = ['ReverbSc', 'ModalVoice', 'StringVoice', 'MoogLadder']
        uses_lgpl = any(re.search(rf'\b{m}\b', content) for m in lgpl_modules)
        has_lgpl_flag = re.search(r'^\s*USE_DAISYSP_LGPL\s*=\s*1', makefile, re.MULTILINE)
        if uses_lgpl and not has_lgpl_flag:
            findings.append(Finding(
                "ERROR", 0, "LGPL-FLAG-MISSING",
                "Code uses LGPL module but Makefile lacks USE_DAISYSP_LGPL = 1"
            ))
        if has_float_printf_flag:
            findings.append(Finding(
                "WARN", 0, "FLOAT-PRINTF-FLAG",
                "Makefile links _printf_float; verify flash headroom and keep "
                "this as an explicit debug or budgeted release choice."
            ))
        if has_float_scanf_flag:
            findings.append(Finding(
                "WARN", 0, "FLOAT-SCANF-FLAG",
                "Makefile links _scanf_float; verify flash headroom and prefer "
                "integer/fixed-point serial parsers when possible."
            ))

    # ── Rule 10: Hallucinated DaisySP API names ──
    # These method names are invented by LLMs and do not exist in DaisySP.
    # Each entry: wrong_name -> (correct_name, affected_class)
    hallucinated_apis = [
        (r'\bSetCutoff\s*\(',           'SetFreq()',                  'MoogLadder/Svf/Tone'),
        (r'\bSetCutoffFrequency\s*\(',  'SetFreq()',                  'MoogLadder/Svf'),
        (r'\bSetQ\s*\(',                'SetRes()',                   'MoogLadder/Svf/Biquad'),
        (r'\bSetResonance\s*\(',        'SetRes()',                   'MoogLadder/Svf/Biquad'),
        (r'\bSetAttack\s*\(',           'SetTime(ADSR_SEG_ATTACK, t)', 'Adsr'),
        # NOTE: SetDecay is VALID on AnalogBassDrum, AnalogSnareDrum, HiHat.
        # Only flag if called on an Adsr object (variable names: adsr, env, envelope).
        # Using a negative lookbehind to skip drum module calls.
        (r'(?:adsr|env|envelope|Adsr|Env|Envelope)\s*\.\s*SetDecay\s*\(',
                                        'SetTime(ADSR_SEG_DECAY, t)',  'Adsr'),
        (r'\bSetRelease\s*\(',          'SetTime(ADSR_SEG_RELEASE, t)','Adsr'),
        (r'\bSetWaveType\s*\(',         'SetWaveform()',              'Oscillator/BlOsc/Tremolo'),
        (r'\.setFreq\s*\(',             'SetFreq() (PascalCase)',     'All DaisySP classes'),
        (r'\.setAmp\s*\(',              'SetAmp() (PascalCase)',      'Oscillator/WhiteNoise'),
        (r'\.setCutoff\s*\(',           'SetFreq() (PascalCase)',     'MoogLadder/Svf'),
        (r'\bsetCutoffFrequency\s*\(',  'SetFreq()',                  'MoogLadder/Svf'),
    ]
    for i, line in enumerate(lines):
        stripped = line.lstrip()
        if stripped.startswith('//'):
            continue  # Skip comments
        for bad_pattern, correct, context in hallucinated_apis:
            if re.search(bad_pattern, line):
                bad_name = re.search(bad_pattern, line).group(0).rstrip('(')
                findings.append(Finding(
                    "ERROR", i + 1, "HALLUCINATED-API",
                    f"{bad_name.strip()} does not exist in DaisySP "
                    f"({context}) — use {correct} instead"
                ))

    return findings


def format_findings(filepath: str, findings: List[Finding]) -> str:
    if not findings:
        return f"  PASS  {filepath}"
    lines = [f"  {filepath}"]
    errors = [f for f in findings if f.level == "ERROR"]
    warns = [f for f in findings if f.level == "WARN"]
    for f in errors + warns:
        loc = f":{f.line}" if f.line > 0 else ""
        lines.append(f"    {f.level:5s}  [{f.rule}] line{loc}  {f.message}")
    return '\n'.join(lines)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    targets = []
    source_suffixes = {'.cpp', '.h', '.hpp'}
    for arg in sys.argv[1:]:
        p = Path(arg)
        if p.is_file() and p.suffix.lower() in source_suffixes:
            targets.append(str(p))
        elif p.is_dir():
            for suffix in sorted(source_suffixes):
                targets.extend(str(f) for f in p.rglob(f'*{suffix}'))
        else:
            print(f"Skipping: {arg} (not a C++ source/header file or directory)")

    if not targets:
        print("No C++ source/header files found.")
        sys.exit(1)

    total_errors = 0
    total_warns = 0

    print(f"\nDaisy QAE Linter — checking {len(targets)} file(s)\n")
    print("-" * 60)

    for filepath in sorted(targets):
        findings = check_file(filepath)
        print(format_findings(filepath, findings))
        total_errors += sum(1 for f in findings if f.level == "ERROR")
        total_warns += sum(1 for f in findings if f.level == "WARN")

    print("-" * 60)
    print(f"\n  {total_errors} error(s), {total_warns} warning(s)\n")
    sys.exit(1 if total_errors > 0 else 0)


if __name__ == '__main__':
    main()
