#!/usr/bin/env python3
"""
Extract individual engine data files from Plaits resources.
Each engine gets its own data file for on-demand loading.
Run: python extract_engine_data.py
"""

import re
import struct
import os

INPUT_FILE = 'resources.cc'  # Copy PlaitsPatchInit/eurorack/plaits/resources.cc here
OUTPUT_DIR = 'plaits/wavetables'

def extract_int16_array(content, array_name, count):
    """Extract int16_t array"""
    pattern = rf'const int16_t {array_name}\[\] = \{{(.*?)\}};'
    match = re.search(pattern, content, re.DOTALL)
    if not match:
        return None
    
    values = [int(x.strip()) for x in match.group(1).split(',') if x.strip()][:count]
    return struct.pack(f'<{len(values)}h', *values)

def extract_float_array_as_int16(content, array_name):
    """Extract float array and convert to Q15"""
    pattern = rf'const float {array_name}\[\] = \{{(.*?)\}};'
    match = re.search(pattern, content, re.DOTALL)
    if not match:
        return None
    
    values = [float(x.strip()) for x in match.group(1).split(',') if x.strip()]
    data_int16 = [max(-32768, min(32767, int(x * 32767))) for x in values]
    return struct.pack(f'<{len(data_int16)}h', *data_int16)

def main():
    print("=" * 60)
    print("Plaits Engine Data Extractor (Individual Files)")
    print("=" * 60)
    
    if not os.path.exists(INPUT_FILE):
        print(f"\nERROR: {INPUT_FILE} not found!")
        print("Copy 'PlaitsPatchInit/eurorack/plaits/resources.cc' here first")
        return
    
    with open(INPUT_FILE, 'r') as f:
        content = f.read()
    
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    print("\n[1] Extracting wavetable data...")
    # Full wavetable - needed by Wavetable, Chord engines
    data = extract_int16_array(content, 'wav_integrated_waves', 49920)
    if data:
        with open(f'{OUTPUT_DIR}/wavetable_full.bin', 'wb') as f:
            f.write(data)
        print(f"  wavetable_full.bin: {len(data)} bytes")
    
    print("\n[2] Extracting waveshaping LUTs...")
    # Waveshaping tables - needed by Waveshaping engine
    data = extract_float_array_as_int16(content, 'lut_fold')
    if data:
        with open(f'{OUTPUT_DIR}/waveshape_lut1.bin', 'wb') as f:
            f.write(data)
        print(f"  waveshape_lut1.bin: {len(data)} bytes")
    
    data = extract_float_array_as_int16(content, 'lut_fold_2')
    if data:
        with open(f'{OUTPUT_DIR}/waveshape_lut2.bin', 'wb') as f:
            f.write(data)
        print(f"  waveshape_lut2.bin: {len(data)} bytes")
    
    # Create manifest
    print("\n[3] Creating manifest...")
    manifest = """# Plaits Engine Data Manifest
# On-demand loading for extra engines
# Load specific file when engine is selected

# Engine requirements:
# A2 (Waveshaping): waveshape_lut1.bin + waveshape_lut2.bin
# A6 (Wavetable): wavetable_full.bin
# A7 (Chord): wavetable_full.bin

# File sizes:
# wavetable_full.bin: 99840 bytes (99KB)
# waveshape_lut1.bin: 1032 bytes (1KB)
# waveshape_lut2.bin: 1032 bytes (1KB)

# Usage:
# Load from SD when engine key is pressed
# Free when switching to base engine
"""
    with open(f'{OUTPUT_DIR}/manifest.txt', 'w') as f:
        f.write(manifest)
    print(f"  manifest.txt created")
    
    print("\n" + "=" * 60)
    print("Extraction complete!")
    print("=" * 60)
    print(f"\nFiles in {OUTPUT_DIR}/:")
    for f in sorted(os.listdir(OUTPUT_DIR)):
        size = os.path.getsize(f'{OUTPUT_DIR}/{f}')
        print(f"  {f}: {size} bytes")

if __name__ == '__main__':
    main()