# Architecture and Implementation of a MIDI-Driven Granular Freeze System

## 1. System Architecture and "Texture Freeze" Conceptual Framework

The "Texture Freeze" system is a real-time DSP architecture designed to bridge the gap between linear audio streams and the stochastic manipulation of microsound. In modern experimental synthesis, "freezing" is not a simple pause in execution but a strategic state-change within the audio signal path. Specifically, it involves the decoupling of the **Write Pointer** from the **Read Pointers** within a circular buffer. While a standard delay or pass-through system advances both pointers synchronously to maintain a constant temporal relationship, the Freeze state suspends `Write()` operations, effectively isolating a specific temporal segment of the live audio stream for continuous interrogation.

This system differs fundamentally from traditional sampling. Per DAFX conventions, "Freezing" is the process of memorizing the spectrum or signal fragments of a _live_ circulating buffer, rather than triggering a static, pre-recorded file. This implementation draws heavily on Curtis Roads’ "Microsound" theory, which shifts the sonic perspective from macroscopic linear streams to the millisecond time scale (typically 1 to 100ms). By arresting the buffer, we transition from macro-musical structures to the creation of "granular clouds"—fixed spectral fragments that remain manipulatable via read-head modulation and stochastic reorganization.

## 2. DSP Architecture: The Circular Buffer Engine

Implementing a granular engine on an embedded platform like the Daisy Field requires a robust, non-blocking circular buffer to handle the high ISR (Interrupt Service Routine) overhead of the ARM Cortex-M7 core. To maintain a constant memory footprint, the buffer is pre-allocated in SDRAM.

The implementation relies on conditional logic within the audio callback. When the "Freeze" state is inactive, the Write pointer advances and overwrites the oldest data. When activated, the Write pointer is locked, but the Read pointers continue to scan the frozen data. To ensure zero memory violations in C++, we utilize 0-based indexing and modulo arithmetic for wrapping. For a buffer of size `BUFFER_SIZE`, the relationship is defined as:

`uint32_t read_ptr = (write_ptr - delay_samples + BUFFER_SIZE) % BUFFER_SIZE;`

The "Granular Read Head" is implemented as a multi-tap delay engine. Each "grain" is essentially a variable delay tap. To prevent digital clipping at the summed output, a "sum-and-scale" strategy is applied: the sum of all active grain outputs is multiplied by the reciprocal of the total number of taps (e.g., `sum * 0.25f` for a 4-tap engine).

To transform robotic, periodic repetitions into organic "textures," we apply stochastic jitter to each Read Head. Jitter introduces random deviations from the nominal delay tap position, which is critical for breaking up periodic phase cancellation and creating the "cloud" density described in Source 302.

### Signal Flow: Live Input to Multi-Tap Granular Output

```text
[ LIVE AUDIO INPUT ] 
         |
         v
[ WRITE GATE (Freeze Switch) ] ----> [ CIRCULAR BUFFER (SDRAM) ]
         |                                  |
         |         +------------------------+-----------------------+
         |         |                        |                       |
         v         v                        v                       v
    [ WRITE PTR ] [ TAP 1 ]              [ TAP 2 ]               [ TAP n ]
    (Suspended    (Jittered)             (Jittered)              (Jittered)
     on Freeze)    |                        |                       |
                   +-----------+------------+-----------+-----------+
                               |
                               v
                      [ SUM-AND-SCALE ] ----> [ FINAL OUTPUT ]
```

## 3. Control Architecture: The Parameter Step Sequencer

The control architecture evolves the step sequencer from a pitch-triggering tool into a timbral-modulation engine. By modulating non-traditional parameters—specifically **Grain Size**, **Density**, and **Buffer Position**—the system generates rhythmic complexity derived from the timbre itself.

Synchronization is achieved via a MIDI Clock Listener that monitors 0xF8 Real-Time Messages. To achieve musical 16th-note resolution, the sequencer logic must account for the 24 PPQN (Pulses Per Quarter Note) standard. The sequencer advances one step every 6 pulses.

### Technical C++ Implementation: `StepSequence` Class

```cpp
class StepSequence {
public:
    static constexpr uint8_t kStepCount = 16;
    float parameters[kStepCount];
    uint8_t current_step = 0;

    // Advances the sequencer based on 0xF8 MIDI clock pulses
    // 24 PPQN / 4 = 6 pulses for 16th note resolution
    void UpdateStep(uint32_t clockPulse) {
        if (clockPulse % 6 == 0) {
            current_step = (current_step + 1) % kStepCount;
        }
    }

    float GetCurrentParam() {
        return parameters[current_step];
    }
};
```

This temporal quantization ensures that shifts in the granular texture are phase-locked with the master DAW tempo, providing the precision required for professional live performance.

## 4. Interface & Mapping: The Daisy Field Implementation

High-stakes performance requires an interface that maps complex DSP interactions to intuitive physical controls. We utilize a "Super Knob" mapping strategy where the physical hardware controls multiple non-linear software variables simultaneously.

The **"Chaos" Macro** is the centerpiece of this implementation. It modulates both Grain Density and Jitter using a non-linear scaling curve; as density increases, the jitter amount expands exponentially to mask the onset of metallic resonance typical of high-density granular synthesis.

Visual feedback is handled via the OLED using the _DaisySP Visual Grammar_. The display differentiates states by switching between a "Waveform Overview" (Live) and a "Stochastic Density Visualization" (Frozen), providing the performer with a clear indication of grain distribution within the buffer.

### Hardware Control Mapping

|   |   |   |
|---|---|---|
|Control|Software Parameter|Functional Outcome|
|**Knob 1**|Grain Size|Sets grain duration (10ms - 100ms range).|
|**Knob 2**|Grain Density|Adjusts overlap count of the multi-tap heads.|
|**Knob 3**|Chaos Macro|Non-linear modulation of Jitter and Density.|
|**Knob 4**|Buffer Position|Scans the Read Pointers through the frozen buffer.|
|**Switch 1**|Freeze Toggle|Disables the `Write()` function to lock audio.|
|**Switch 2**|Sync Mode|Toggles 24 PPQN MIDI vs. Internal Clock.|

## 5. Implementation Challenges & Mitigation Strategies

Operating in an embedded environment with the AK4430 codec and Cortex-M7 requires careful management of buffer latency and sonic artifacts. A significant challenge in "Freeze" systems is the potential for audible discontinuities when the Write pointer is suspended or when grain boundaries clash.

To mitigate clicks and "pops," we implement windowing functions on a **per-grain basis**. This allows multiple overlapping "clouds" to exist without phase-related impulses at the start or end of each grain's lifecycle. We prioritize the Hanning window for its superior spectral smoothing characteristics.

### C++ Snippet: Hanning Window Application

```cpp
#include <cmath>

// Applies a Hanning window to an individual grain buffer
// Formula: 0.5 * (1 - cos(2*PI*n / (N-1)))
void ApplyHanning(float* grainBuffer, uint32_t grainSize) {
    for (uint32_t n = 0; n < grainSize; n++) {
        float window = 0.5f * (1.0f - cosf(2.0f * M_PI * n / (grainSize - 1)));
        grainBuffer[n] *= window;
    }
}
```

By combining 0-based pointer arithmetic for buffer stability with MIDI-synced granular modulation, the system achieves a balance of precision and stochastic organicism. This architecture ensures the Daisy Field platform functions as a professional-grade microsound instrument capable of complex real-time spectral transformation.