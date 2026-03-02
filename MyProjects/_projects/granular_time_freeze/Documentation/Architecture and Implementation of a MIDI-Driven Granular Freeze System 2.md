# Architecture and Implementation of a MIDI-Driven Granular Freeze System

### 1. Executive Summary: The "Texture Freeze" Paradigm

In modern embedded sound design, the "Texture Freeze" architecture represents a strategic evolution beyond the constraints of linear sampling. On high-performance platforms like the Daisy Field, this paradigm shifts the system from a playback-oriented device to a generative microsound engine. By decoupling the Write Pointer from multiple independent Read Pointers, we transition from a standard circular buffer to a malleable audio "cloud" [Source 546, 564].

Unlike DAFX "spectral freezing," which utilizes FFT-based phase vocoding to sustain timbres, the Texture Freeze system operates in the time domain. It captures a discrete window of audio and subjects it to granular multi-tap synthesis—treating audio as a collection of asynchronous grains (microsound) rather than a continuous stream [Source 301, 540]. This decoupling allows for the suspension of recording (the "Freeze") while read heads scan the buffer with stochastic variations, creating organic, evolving textures. This fundamental shift requires a specialized circular buffer engine optimized for the STM32’s ARM Cortex-M7 architecture.

### 2. DSP Architecture: The Circular Buffer Engine

The foundation of granular synthesis is the circular buffer. In a real-time embedded system, pointer logic efficiency is the primary determinant of audio stability.

#### Buffer Management and Hardware Constraints

On the Daisy Field, the internal SRAM is insufficient for high-density granular buffers. Therefore, large audio buffers must be allocated to the 64MB SDRAM using the `DSY_SDRAM_BSS` attribute to prevent memory overflow.

While many high-level implementations use the modulo operator (`%`) for index wrapping, a senior architect must account for two issues: the `%` operator is computationally expensive on the Cortex-M7, and in C++, it can return negative values for negative inputs—a frequent cause of memory corruption when calculating `readIndex = (writeIndex - delay) % size`. To optimize, we utilize **power-of-two buffer sizes** and **bitwise masking**.

```cpp
// Buffer allocation in SDRAM
#define BUFFER_SIZE 262144 // Power of two (2^18)
#define BIT_MASK (BUFFER_SIZE - 1)
float DSY_SDRAM_BSS buffer[BUFFER_SIZE];

uint32_t writeIndex = 0;
bool isFrozen = false;

// Within Audio Callback
void Process(float input) {
    if (!isFrozen) {
        buffer[writeIndex & BIT_MASK] = input;
        writeIndex++;
    }
    // Read logic persists independently
}
```

#### Granular Multi-Tap Synthesis and Stochastic Jitter

The engine employs a multi-tap strategy where variable read heads function as individual grains [Source 27, 297]. To transform a repetitive loop into a "Texture," we apply stochastic jitter to the read head positions. By utilizing a Gaussian random number generator (`randn`), we introduce grain start-time offsets. To ensure these offsets are musically relevant, the Gaussian distribution must be scaled using linear transformation math: y = m \cdot x + \mu, where m is the jitter depth and \mu is the base grain position [Source 8.6, 7.4.1].

**Signal Flow Diagram**

```text
[ Input ] ---> [ Write Gate (isFrozen) ] ---> [ SDRAM Buffer ]
                                                     |
                                    -----------------------------------
                                    |                |                |
                             [ Read Head 1 ]  [ Read Head 2 ]  [ Read Head N ]
                             [ Windowing   ]  [ Windowing   ]  [ Windowing   ]
                                    |                |                |
                                    -----------------------------------
                                                     |
                                             [ Granular Output ]
```

### 3. Control Architecture: The MIDI-Synced Parameter Sequencer

Generative textures gain musical utility when timbral parameters—Grain Size, Density, and Position—are synchronized to a temporal grid [Source 252].

#### MIDI Synchronization and Jitter Mitigation

Synchronizing with a DAW requires a robust **MIDI Clock Listener** handling `0xF8` Real-Time Messages [Source 4]. Given the MIDI standard of 24 pulses per quarter note (PPQN), the sequencer employs a `ticksPerStep` variable to handle subdivisions (e.g., 6 pulses for 16th notes). To account for MIDI clock jitter, the system uses an internal timer to smooth the interval between `0xF8` pulses.

#### Thread Safety in Embedded DSP

A critical architectural challenge is the separation of the high-priority Audio Callback and the lower-priority MIDI/Control thread. To prevent race conditions during parameter updates, all shared variables between the `StepSequence` and the DSP engine must use atomic types or a lock-free message queue.

```cpp
class StepSequence {
    std::atomic<float> grainPosSteps[16];
    uint32_t pulseCount = 0;
    uint32_t currentStep = 0;
    uint32_t ticksPerStep = 6; // 16th note subdivision

    void OnMidiClock() { // Called on 0xF8 message
        pulseCount++;
        if (pulseCount >= ticksPerStep) {
            currentStep = (currentStep + 1) & 0xF; // Bitwise wrap for 16 steps
            pulseCount = 0;
            // Atomic update to the DSP engine
            engine.SetPosition(grainPosSteps[currentStep].load());
        }
    }
};
```

### 4. Interface & Mapping: Performance Logic on Daisy Field

Effective UI turns a complex granular engine into a playable instrument. We use a "Chaos" macro to consolidate high-dimensional control.

#### Hardware Mapping and Scaling

The "Chaos" macro uses linear scale transformations to map a single knob (0.0 to 1.0) to multiple DSP parameters [Source 8.6]. As the Chaos value x increases, we scale Grain Density (m_{dens}) and Jitter (m_{jit}) while inversely scaling Grain Size.

|   |   |   |
|---|---|---|
|Control|Parameter|Scaling Logic (y = m \cdot x + \mu)|
|**Knob 1**|Grain Size|m = -100ms, \mu = 150ms|
|**Knob 2**|Chaos Macro|Maps to Density and Jitter Depth [Source 859]|
|**Knob 3**|Scan Position|m = \text{BufferLength}, \mu = 0|
|**Knob 4**|Feedback/Mix|Linear crossfade between Dry and Cloud|

#### Visual Grammar and OLED Feedback

Following DaisySP Visual Grammar [Source 856], the OLED must distinguish between "Live" and "Frozen" states. In the "Live" state, the display shows the rolling write pointer. Upon freezing, the display renders a static representation of the buffer with a "Playhead" that jumps per grain rather than moving linearly, providing the user with immediate visual confirmation of the granular scanning process.

### 5. Implementation Challenges and Technical Solutions

The transition from a theoretical blueprint to a professional-grade instrument requires addressing low-level artifacts.

#### Artifact Suppression (Concatenation of Fades)

Discontinuities at grain loop points cause audible clicks. To suppress these, we apply a windowing function to every grain. Implementing a Hanning or Triangle window is most efficient via a Look-Up Table (LUT) to save clock cycles. Conceptually, each grain follows an ADSR-like "concatenation of fades": a rapid linear attack, a sustained segment, and a linear release [Source 115, 685, 17.2.1]. For Triangle windows, an additive synthesis approach (summing odd harmonics) can be used to generate the window shape if memory for a LUT is restricted [Source 96].

#### Buffer Optimization and Latency

To ensure a seamless transition from live audio to a frozen state, the read heads must be pre-allocated and running in the background. By ensuring the read pointers are always "live" with a zero-offset, the system avoids the latency of head initialization when the `isFrozen` flag is toggled. Combined with the power-of-two bitwise masking, this ensures a robust, low-latency generative engine suitable for rigorous performance environments.