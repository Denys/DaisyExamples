# NIM_Granular.md

## Purpose
Advanced granular synthesis engine for the Nimbus project, enabling complex textural sounds through manipulation of short audio grains extracted from sample buffers.

## Current Implementation Status
✅ **IMPLEMENTED** - Component exists in field/Nimbus/

## Implementation Details
- **Location**: field/Nimbus/dsp/granular_processor.cpp
- **Current interfaces**:
  - `GranularProcessor` class
  - Methods: Init, SetSpeed, SetGrainSize, SetGrainDensity, SetPosition
  - Process method for audio callback
- **Dependencies**: DaisySP, AudioBuffer, sample management
- **Dependents**: Nimbus application

## Core Logic & Functionality
1. **Grain Extraction**: Extract short samples (1-100ms) from buffer
2. **Grain Positioning**: Random or sequential position control
3. **Grain Envelope**: Apply envelope to each grain to prevent clicks
4. **Pitch/Time Control**: Independent pitch and time manipulation
5. **Density Control**: Number of grains per second
6. **Windowing**: Multiple window function options (Hann, Hamming, etc.)

## Current Quality Assessment
- **Completeness**: High - comprehensive granular features
- **Code Quality**: Good - well-structured with clear comments
- **Test Coverage**: N/A - tested via audio output
- **Documentation**: Good - inline comments, README

## Technical Debt & Improvement Areas
- Could add more grain modes
- Performance optimization for dense grains

## Interface Definition
```cpp
class GranularProcessor {
public:
    void Init(float sample_rate);
    void SetBuffer(AudioBuffer* buffer);
    void SetSpeed(float speed);        // -2.0 to 2.0
    void SetGrainSize(float size_ms);  // 1-200ms
    void SetGrainDensity(float density); // grains per second
    void SetPosition(float position);   // 0.0-1.0
    
    float Process();  // Returns grain output
};
```

## ARC Verification Criteria

### Functional Criteria
- [x] Grains extracted correctly from buffer
- [x] Envelope prevents audio clicks
- [x] Pitch control works (-2x to 2x)
- [x] Time stretching works independently

### Input Validation Criteria  
- [x] Parameters clamped to valid ranges
- [x] Null buffer handled gracefully

### Error Handling Criteria
- [x] Handles empty buffer
- [x] No NaN outputs

### Quality Criteria
- [x] Performance: Real-time at 48kHz
- [x] Quality: Smooth grain transitions

## Future Enhancement Opportunities
- Add formantic preservation
- Add grain cloud mode
- Add pitch correction
