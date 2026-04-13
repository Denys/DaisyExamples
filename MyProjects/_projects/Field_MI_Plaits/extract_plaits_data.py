#!/usr/bin/env python3
"""
Extract Plaits data tables from resources.cc for SD card loading
Run: python extract_plaits_data.py

This creates binary files that can be loaded from SD card on Daisy Field
to enable extra Plaits engines (Wavetable, Waveshaping, Chord)
"""

import re
import struct
import sys
import os

# Configuration
INPUT_FILE = 'resources.cc'  # Copy PlaitsPatchInit/eurorack/plaits/resources.cc here
OUTPUT_DIR = 'plaits'


def extract_array_as_int16(content, array_name):
    """Extract int16_t array and convert to binary"""
    pattern = rf'const int16_t {array_name}\[\] = \{{(.*?)\}};'
    match = re.search(pattern, content, re.DOTALL)
    if not match:
        print(f"  ERROR: {array_name} not found!")
        return None
    
    # Parse the array values
    values_str = match.group(1)
    values = [int(x.strip()) for x in values_str.split(',') if x.strip()]
    
    # Convert to little-endian 16-bit binary
    data = struct.pack(f'<{len(values)}h', *values)
    
    print(f"  Found {len(values)} values ({len(data)} bytes)")
    return data


def extract_array_as_float_to_int16(content, array_name):
    """Extract float array, convert to Q15 fixed point"""
    pattern = rf'const float {array_name}\[\] = \{{(.*?)\}};'
    match = re.search(pattern, content, re.DOTALL)
    if not match:
        print(f"  ERROR: {array_name} not found!")
        return None
    
    # Parse as floats
    values_str = match.group(1)
    values = [float(x.strip()) for x in values_str.split(',') if x.strip()]
    
    # Convert float to Q15 (fixed point 1.0 = 32767)
    data_int16 = []
    for v in values:
        q15 = int(v * 32767)
        q15 = max(-32768, min(32767, q15))  # clip
        data_int16.append(q15)
    
    data = struct.pack(f'<{len(data_int16)}h', *data_int16)
    
    print(f"  Found {len(values)} values, saved as {len(data)} bytes (Q15)")
    return data


def main():
    print("=" * 60)
    print("Plaits Data Extractor for SD Card")
    print("=" * 60)
    
    # Check for input file
    if not os.path.exists(INPUT_FILE):
        print(f"\nERROR: {INPUT_FILE} not found!")
        print("\nPlease copy 'PlaitsPatchInit/eurorack/plaits/resources.cc'")
        print("to this directory first.")
        sys.exit(1)
    
    # Read source
    print(f"\nReading {INPUT_FILE}...")
    with open(INPUT_FILE, 'r') as f:
        content = f.read()
    
    # Create output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # Extract wav_integrated_waves (49,920 bytes)
    print("\n[1] Extracting wav_integrated_waves...")
    data = extract_array_as_int16(content, 'wav_integrated_waves')
    if data:
        output_path = os.path.join(OUTPUT_DIR, 'wavetable.bin')
        with open(output_path, 'wb') as f:
            f.write(data)
        print(f"  Saved to {output_path}")
    
    # Extract lut_fold (waveshaping LUT 1)
    print("\n[2] Extracting lut_fold...")
    data = extract_array_as_float_to_int16(content, 'lut_fold')
    if data:
        output_path = os.path.join(OUTPUT_DIR, 'table_fold.bin')
        with open(output_path, 'wb') as f:
            f.write(data)
        print(f"  Saved to {output_path}")
    
    # Extract lut_fold_2 (waveshaping LUT 2)
    print("\n[3] Extracting lut_fold_2...")
    data = extract_array_as_float_to_int16(content, 'lut_fold_2')
    if data:
        output_path = os.path.join(OUTPUT_DIR, 'table_fold2.bin')
        with open(output_path, 'wb') as f:
            f.write(data)
        print(f"  Saved to {output_path}")
    
    # Summary
    print("\n" + "=" * 60)
    print("Extraction complete!")
    print("=" * 60)
    
    if os.path.exists(OUTPUT_DIR):
        print(f"\nFiles created in {OUTPUT_DIR}/:")
        for f in os.listdir(OUTPUT_DIR):
            fpath = os.path.join(OUTPUT_DIR, f)
            size = os.path.getsize(fpath)
            print(f"  {f}: {size} bytes")


if __name__ == '__main__':
    main()